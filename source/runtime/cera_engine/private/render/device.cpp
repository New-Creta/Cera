#include "render/device.h"
#include "render/d3dx12_declarations.h"
#include "render/d3dx12_call.h"
#include "render/command_queue.h"
#include "render/descriptor_allocator.h"
#include "render/vertex_buffer.h"
#include "render/index_buffer.h"
#include "render/byte_address_buffer.h"
#include "render/constant_buffer.h"
#include "render/constant_buffer_view.h"
#include "render/shader_resource_view.h"
#include "render/unordered_access_view.h"
#include "render/root_signature.h"
#include "render/command_list.h"
#include "render/texture.h"
#include "render/pipeline_state_object.h"
#include "render/swapchain.h"

#include "util/memory_helpers.h"
#include "util/log.h"

#include <dxgi1_5.h>
#include <dxgidebug.h>

namespace cera
{
    namespace internal
    {
        wrl::ComPtr<IDXGIAdapter4> get_adapter(bool useWarp)
        {
            u32 create_factory_flags = 0;
            #if defined(_DEBUG)
            create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
            #endif
            wrl::ComPtr<IDXGIFactory4> dxgi_factory;

            if (DX_FAILED((CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(&dxgi_factory)))))
            {
                log::error("Unable to create IDXGIFactory4");
                return nullptr;
            }

            wrl::ComPtr<IDXGIAdapter1> dxgi_adapter1;
            wrl::ComPtr<IDXGIAdapter4> dxgi_adapter4;

            if (useWarp)
            {
                if (DX_FAILED(dxgi_factory->EnumWarpAdapter(IID_PPV_ARGS(&dxgi_adapter1))))
                {
                    log::error("Unable to create to enumerate WarpAdapters");
                    return nullptr;
                }

                if (DX_FAILED((dxgi_adapter1.As(&dxgi_adapter4))))
                {
                    log::error("Unable convert IDXGIAdapter1 to IDXGIAdapter4");
                    return nullptr;
                }
            }
            else
            {
                size_t max_dedicated_video_memory = 0;
                for (u32 i = 0; dxgi_factory->EnumAdapters1(i, &dxgi_adapter1) != DXGI_ERROR_NOT_FOUND; ++i)
                {
                    DXGI_ADAPTER_DESC1 dxgi_adapter_desc1;
                    dxgi_adapter1->GetDesc1(&dxgi_adapter_desc1);

                    // Check to see if the adapter can create a D3D12 device without actually
                    // creating it. The adapter with the largest dedicated video memory
                    // is favored.
                    if ((dxgi_adapter_desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 && SUCCEEDED(D3D12CreateDevice(dxgi_adapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) && dxgi_adapter_desc1.DedicatedVideoMemory > max_dedicated_video_memory)
                    {
                        max_dedicated_video_memory = dxgi_adapter_desc1.DedicatedVideoMemory;

                        if (DX_FAILED((dxgi_adapter1.As(&dxgi_adapter4))))
                        {
                            log::error("Unable convert IDXGIAdapter1 to IDXGIAdapter4");
                            return nullptr;
                        }
                    }
                }
            }

            return dxgi_adapter4;
        }
        
        wrl::ComPtr<ID3D12Device2> create_device(wrl::ComPtr<IDXGIAdapter4> adapter)
        {
            wrl::ComPtr<ID3D12Device2> d3d12_device;
            if (DX_FAILED((D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&d3d12_device)))))
            {
                log::error("Unable to create D3D12Device");
                return nullptr;
            }

            // Enable debug messages in debug mode.
            #if defined(_DEBUG)
            wrl::ComPtr<ID3D12InfoQueue> info_queue;
            if (DX_SUCCESS(d3d12_device.As(&info_queue)))
            {
                info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
                info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

                // Suppress messages based on their severity level
                D3D12_MESSAGE_SEVERITY severities[] =
                {
                    D3D12_MESSAGE_SEVERITY_INFO };

                // Suppress individual messages by their ID
                D3D12_MESSAGE_ID deny_ids[] = {
                    D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE, // I'm really not sure how to avoid this message.
                    D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
                    D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                     // This warning occurs when using capture frame while graphics debugging.
                };

                D3D12_INFO_QUEUE_FILTER new_filter = {};
                new_filter.DenyList.NumSeverities = _countof(severities);
                new_filter.DenyList.pSeverityList = severities;
                new_filter.DenyList.NumIDs = _countof(deny_ids);
                new_filter.DenyList.pIDList = deny_ids;

                if (DX_FAILED(info_queue->PushStorageFilter(&new_filter)))
                {
                    log::error("Unable to push StorageFilter");
                    return nullptr;
                }
            }
            #endif

            return d3d12_device;
        }

        bool check_tearing_support()
        {
            bool allow_tearing = false;

            // Rather than create the DXGI 1.5 factory interface directly, we create the
            // DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
            // graphics debugging tools which will not support the 1.5 factory interface 
            // until a future update.
            wrl::ComPtr<IDXGIFactory4> factory4;

            if (DX_SUCCESS(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
            {
                wrl::ComPtr<IDXGIFactory5> factory5;
                if (DX_SUCCESS(factory4.As(&factory5)))
                {
                    if (DX_FAILED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, sizeof(allow_tearing))))
                    {
                        allow_tearing = false;
                    }
                }
            }

            return allow_tearing == true;
        }
    }

    namespace adaptors
    {
        class make_unordered_access_view : public unordered_access_view
        {
        public:
            make_unordered_access_view(device& device, const std::shared_ptr<resource>& inResource, const std::shared_ptr<resource>& inCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav)
                : unordered_access_view(device, inResource, inCounterResource, uav)
            {}

            ~make_unordered_access_view() override = default;
        };

        class make_shader_resource_view : public shader_resource_view
        {
        public:
            make_shader_resource_view(device& device, const std::shared_ptr<resource>& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srv)
                : shader_resource_view(device, resource, srv)
            {}

            ~make_shader_resource_view() override = default;
        };

        class make_constant_buffer_view : public constant_buffer_view
        {
        public:
            make_constant_buffer_view(device& device, const std::shared_ptr<constant_buffer>& constantBuffer, size_t offset)
                : constant_buffer_view(device, constantBuffer, offset)
            {}

            ~make_constant_buffer_view() override = default;
        };

        class make_pipeline_state_object : public pipeline_state_object
        {
        public:
            make_pipeline_state_object(device& device, const D3D12_PIPELINE_STATE_STREAM_DESC& desc)
                : pipeline_state_object(device, desc)
            {}

            ~make_pipeline_state_object() override = default;
        };
        class make_root_signature : public root_signature
        {
        public:
            make_root_signature(device& device, const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc)
                : root_signature(device, rootSignatureDesc)
            {}

            ~make_root_signature() {}
        };

        class make_texture : public texture
        {
        public:
            make_texture(device& device, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue)
                : texture(device, resourceDesc, clearValue)
            {}

            make_texture(device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue)
                : texture(device, resource, clearValue)
            {}

            ~make_texture() override = default;
        };

        class make_vertex_buffer : public vertex_buffer
        {
        public:
            make_vertex_buffer(device& device, size_t numVertices, size_t vertexStride)
                : vertex_buffer(device, numVertices, vertexStride)
            {}

            make_vertex_buffer(device& device, ComPtr<ID3D12Resource> resource, size_t numVertices, size_t vertexStride)
                : vertex_buffer(device, resource, numVertices, vertexStride)
            {}

            ~make_vertex_buffer() override = default;
        };

        class make_index_buffer : public index_buffer
        {
        public:
            make_index_buffer(device& device, size_t numIndices, DXGI_FORMAT indexFormat)
                : index_buffer(device, numIndices, indexFormat)
            {}

            make_index_buffer(device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numIndices, DXGI_FORMAT indexFormat)
                : index_buffer(device, resource, numIndices, indexFormat)
            {}

            ~make_index_buffer() override = default;
        };

        class make_constant_buffer : public constant_buffer
        {
        public:
            make_constant_buffer(device& device, ComPtr<ID3D12Resource> resource)
                : constant_buffer(device, resource)
            {}

            ~make_constant_buffer() override = default;
        };

        class make_byte_address_buffer : public byte_address_buffer
        {
        public:
            make_byte_address_buffer(device& device, const D3D12_RESOURCE_DESC& desc)
                : byte_address_buffer(device, desc)
            {}

            make_byte_address_buffer(device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource)
                : byte_address_buffer(device, resource)
            {}

            ~make_byte_address_buffer() override = default;
        };

        class make_descriptor_allocator : public descriptor_allocator
        {
        public:
            make_descriptor_allocator(device& device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap = 256)
                : descriptor_allocator(device, type, numDescriptorsPerHeap)
            {}

            ~make_descriptor_allocator() override = default;
        };

        class make_command_queue : public command_queue
        {
        public:
            make_command_queue(device& device, D3D12_COMMAND_LIST_TYPE type)
                : command_queue(device, type)
            {}

            ~make_command_queue() override = default;
        };

        class make_device : public device
        {
        public:
            make_device(wrl::ComPtr<IDXGIAdapter4> adaptor, wrl::ComPtr<ID3D12Device2> device)
                : device(adaptor, device)
            {}

            virtual ~make_device() {}
        };
    }

    bool device::enable_debug_layer()
    {
        #if defined(_DEBUG)
        // Always enable the debug layer before doing anything DX12 related
        // so all possible errors generated while creating DX12 objects
        // are caught by the debug layer.
        wrl::ComPtr<ID3D12Debug> debug_interface;
        if (DX_FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface))))
        {
            log::error("Unable to get D3D12DebugInterface");
            return false;
        }

        debug_interface->EnableDebugLayer();
        #endif

        return true;
    }

    void device::report_live_objects()
    {
        IDXGIDebug1* dxgi_debug;
        DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgi_debug));

        dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
        dxgi_debug->Release();
    }

    std::shared_ptr<device> device::create()
    {
        if (!enable_debug_layer())
        {
            log::error("Unable to create ID3D12Debug");
            return false;
        }

        wrl::ComPtr<IDXGIAdapter4> dxgi_adaptor = internal::get_adapter(false);

        if (dxgi_adaptor)
        {
            wrl::ComPtr<ID3D12Device2> d3d_device = internal::create_device(dxgi_adaptor);

            if (d3d_device)
            {
                return std::make_shared<adaptors::make_device>(dxgi_adaptor, d3d_device);
            }
            else
            {
                log::error("Unable to create ID3D12Device2");
                return nullptr;
            }
        }
        else
        {
            log::error("Unable to create IDXGIAdapter4");
            return false;
        }

        return nullptr;
    }

    device::device(wrl::ComPtr<IDXGIAdapter4> dxgiAdaptor, wrl::ComPtr<ID3D12Device2> d3dDevice)
        :m_dxgi_adapter(dxgiAdaptor)
        ,m_d3d12_device(d3dDevice)
        ,m_direct_command_queue(nullptr)
        ,m_compute_command_queue(nullptr)
        ,m_copy_command_queue(nullptr)
        ,m_tearing_supported(false)
    {
        assert(m_dxgi_adapter != nullptr);
        assert(m_d3d12_device != nullptr);

        m_direct_command_queue = std::make_unique<adaptors::make_command_queue>(*this, D3D12_COMMAND_LIST_TYPE_DIRECT);
        m_compute_command_queue = std::make_unique<adaptors::make_command_queue>(*this, D3D12_COMMAND_LIST_TYPE_COMPUTE);
        m_copy_command_queue = std::make_unique<adaptors::make_command_queue>(*this, D3D12_COMMAND_LIST_TYPE_COPY);

        m_tearing_supported = internal::check_tearing_support();

        // Create descriptor allocators
        {
            for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
            {
                m_descriptor_allocators[i] = std::make_unique<adaptors::make_descriptor_allocator>(*this, static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
            }
        }

        // Check features.
        {
            D3D12_FEATURE_DATA_ROOT_SIGNATURE feature_data;
            feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
            
            if (DX_FAILED(m_d3d12_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &feature_data, sizeof(D3D12_FEATURE_DATA_ROOT_SIGNATURE))))
            {
                feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
            }

            m_highest_root_signature_version = feature_data.HighestVersion;
        }
    }

    device::~device() = default;

    IDXGIAdapter4* device::get_dxgi_adapter() const
    {
        return m_dxgi_adapter.Get();
    }

    ID3D12Device2* device::get_d3d_device() const
    {
        return m_d3d12_device.Get();
    }

    command_queue& device::get_command_queue(D3D12_COMMAND_LIST_TYPE type) const
    {
        command_queue* command_queue = nullptr;

        switch (type)
        {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            command_queue = m_direct_command_queue.get();
            break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            command_queue = m_compute_command_queue.get();
            break;
        case D3D12_COMMAND_LIST_TYPE_COPY:
            command_queue = m_copy_command_queue.get();
            break;
        default:
            assert(false && "Invalid D3D12_COMMAND_LIST_TYPE.");
        }

        return *command_queue;
    }

    void device::flush()
    {
        m_direct_command_queue->flush();
        m_compute_command_queue->flush();
        m_copy_command_queue->flush();
    }

    descriptor_allocation device::allocate_descriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, u32 numDescriptors)
    {
        return m_descriptor_allocators[type]->allocate(numDescriptors);
    }

    u32 device::get_descriptor_handle_increment_size(D3D12_DESCRIPTOR_HEAP_TYPE type) const
    {
        return m_d3d12_device->GetDescriptorHandleIncrementSize(type);
    }

    void device::release_stale_descriptors()
    {
        for ( int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i )
        {
            m_descriptor_allocators[i]->release_stale_descriptors();
        }
    }

    std::shared_ptr<constant_buffer> device::create_constant_buffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource)
    {
        std::shared_ptr<constant_buffer> constant_buffer = std::make_shared<adaptors::make_constant_buffer>(*this, resource);

        return constant_buffer;
    }

    std::shared_ptr<byte_address_buffer> device::create_byte_address_buffer(size_t bufferSize)
    {
        // Align-up to 4-bytes
        size_t aligned_buffer_size = memory::align_up(bufferSize, 4);

        std::shared_ptr<byte_address_buffer> buffer = std::make_shared<adaptors::make_byte_address_buffer>(*this, CD3DX12_RESOURCE_DESC::Buffer(aligned_buffer_size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS));

        return buffer;
    }

    std::shared_ptr<byte_address_buffer> device::create_byte_address_buffer(ComPtr<ID3D12Resource> resource)
    {
        std::shared_ptr<byte_address_buffer> buffer = std::make_shared<adaptors::make_byte_address_buffer>(*this, resource);

        return buffer;
    }

    std::shared_ptr<index_buffer> device::create_index_buffer(size_t numIndices, DXGI_FORMAT indexFormat)
    {
        std::shared_ptr<index_buffer> index_buffer = std::make_shared<adaptors::make_index_buffer>(*this, numIndices, indexFormat);

        return index_buffer;
    }

    std::shared_ptr<index_buffer> device::create_index_buffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numIndices, DXGI_FORMAT indexFormat)
    {
        std::shared_ptr<index_buffer> index_buffer = std::make_shared<adaptors::make_index_buffer>(*this, resource, numIndices, indexFormat);

        return index_buffer;
    }

    std::shared_ptr<vertex_buffer> device::create_vertex_buffer(size_t numVertices, size_t vertexStride)
    {
        std::shared_ptr<vertex_buffer> vertex_buffer = std::make_shared<adaptors::make_vertex_buffer>(*this, numVertices, vertexStride);

        return vertex_buffer;
    }

    std::shared_ptr<vertex_buffer> device::create_vertex_buffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numVertices, size_t vertexStride)
    {
        std::shared_ptr<vertex_buffer> vertex_buffer = std::make_shared<adaptors::make_vertex_buffer>(*this, resource, numVertices, vertexStride);

        return vertex_buffer;
    }

    std::shared_ptr<texture> device::create_texture(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue)
    {
        std::shared_ptr<texture> texture = std::make_shared<adaptors::make_texture>(*this, resourceDesc, clearValue);

        return texture;
    }

    std::shared_ptr<texture> device::create_texture(Microsoft::WRL::ComPtr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue)
    {
        std::shared_ptr<texture> texture = std::make_shared<adaptors::make_texture>(*this, resource, clearValue);

        return texture;
    }

    std::shared_ptr<root_signature> device::create_root_signature(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc)
    {
        std::shared_ptr<root_signature> root_signature = std::make_shared<adaptors::make_root_signature>(*this, rootSignatureDesc);

        return root_signature;
    }

    std::shared_ptr<pipeline_state_object> device::do_create_pipeline_state_object(const D3D12_PIPELINE_STATE_STREAM_DESC& pipelineStateStreamDesc)
    {
        std::shared_ptr<pipeline_state_object> pipeline_state_object = std::make_shared<adaptors::make_pipeline_state_object>(*this, pipelineStateStreamDesc);

        return pipeline_state_object;
    }

    std::shared_ptr<constant_buffer_view> device::create_constant_buffer_view(const std::shared_ptr<constant_buffer>& constant_buffer, size_t offset)
    {
        std::shared_ptr<constant_buffer_view> constant_buffer_view = std::make_shared<adaptors::make_constant_buffer_view>(*this, constant_buffer, offset);

        return constant_buffer_view;
    }

    std::shared_ptr<shader_resource_view> device::create_shader_resource_view(const std::shared_ptr<resource>& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srv)
    {
        std::shared_ptr<shader_resource_view> shader_resource_view = std::make_shared<adaptors::make_shader_resource_view>(*this, resource, srv);

        return shader_resource_view;
    }

    std::shared_ptr<unordered_access_view> device::create_unordered_access_view(const std::shared_ptr<resource>& inResource, const std::shared_ptr<resource>& inCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav)
    {
        std::shared_ptr<unordered_access_view> unordered_access_view = std::make_shared<adaptors::make_unordered_access_view>(*this, inResource, inCounterResource, uav);

        return unordered_access_view;
    }

    D3D_ROOT_SIGNATURE_VERSION device::get_highest_root_signature_version() const
    {
        return m_highest_root_signature_version;
    }

    DXGI_SAMPLE_DESC device::get_multisample_quality_levels(DXGI_FORMAT format, UINT numSamples, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags) const
    {
        DXGI_SAMPLE_DESC sample_desc = { 1, 0 };

        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS quality_levels;
        quality_levels.Format = format;
        quality_levels.SampleCount = 1;
        quality_levels.Flags = flags;
        quality_levels.NumQualityLevels = 0;

        while (quality_levels.SampleCount <= numSamples 
            && SUCCEEDED(m_d3d12_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &quality_levels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS))) 
            && quality_levels.NumQualityLevels > 0)
        {
            // That works...
            sample_desc.Count = quality_levels.SampleCount;
            sample_desc.Quality = quality_levels.NumQualityLevels - 1;

            // But can we do better?
            quality_levels.SampleCount *= 2;
        }

        return sample_desc;
    }

    bool device::is_tearing_supported() const
    {
        return m_tearing_supported;
    }
}