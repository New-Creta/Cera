#include "command_list.h"
#include "command_queue.h"

#include "directx_call.h"
#include "directx_device.h"
#include "directx_swapchain.h"
#include "directx_util.h"

#include "resources/byte_address_buffer.h"
#include "resources/constant_buffer.h"
#include "resources/constant_buffer_view.h"
#include "resources/index_buffer.h"
#include "resources/pipeline_state_object.h"
#include "resources/root_signature.h"
#include "resources/shader_resource_view.h"
#include "resources/texture.h"
#include "resources/unordered_access_view.h"
#include "resources/vertex_buffer.h"

#include "descriptors/descriptor_allocator.h"

#include "util/assert.h"
#include "util/log.h"
#include "util/pointer_math.h"

#include "dxgi/dxgi_adapter_manager.h"
#include "dxgi/dxgi_adapter.h"
#include "dxgi/objects/factory.h"

#include "rhi_globals.h"

#include <dxgi1_5.h>
#include <dxgidebug.h>

namespace cera
{
    namespace renderer
    {
        //-------------------------------------------------------------------------
        s32 get_max_none_sampler_descriptor_count(wrl::com_ptr<ID3D12Device> device, D3D12_RESOURCE_BINDING_TIER resource_binding_tier)
        {
            switch (resource_binding_tier)
            {
            case D3D12_RESOURCE_BINDING_TIER_1:
                return D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1;
            case D3D12_RESOURCE_BINDING_TIER_2:
                return D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2;
            case D3D12_RESOURCE_BINDING_TIER_3: {
                // From: https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html#levels-of-hardware-support
                //   For Tier 3, the max # descriptors is listed as 1000000+. The + indicates that the runtime allows applications
                //   to try creating descriptor heaps with more than 1000000 descriptors, leaving the driver to decide whether
                //   it can support the request or fail the call. There is no cap exposed indicating how large of a descriptor
                //   heap the hardware could support � applications can just try what they want and fall back to 1000000 if
                //   larger doesn�t work.
                // RenderDoc seems to give up on subsequent API calls if one of them return E_OUTOFMEMORY, so we don't use this
                // detection method if the RD plug-in is loaded.

#if _DEBUG
                wrl::com_ptr<ID3D12InfoQueue> info_queue;

                if (renderer::g_is_debug_layer_enabled)
                {
                    // Temporarily silence CREATE_DESCRIPTOR_HEAP_LARGE_NUM_DESCRIPTORS since we know we might break on it
                    device->QueryInterface(IID_PPV_ARGS(info_queue.GetAddressOf()));
                    if (info_queue)
                    {
                        D3D12_MESSAGE_ID message_id = D3D12_MESSAGE_ID_CREATE_DESCRIPTOR_HEAP_LARGE_NUM_DESCRIPTORS;

                        D3D12_INFO_QUEUE_FILTER new_filter{};
                        new_filter.DenyList.NumIDs = 1;
                        new_filter.DenyList.pIDList = &message_id;

                        info_queue->PushStorageFilter(&new_filter);
                    }
                }
#endif

                // create an overly large heap and test for failure
                D3D12_DESCRIPTOR_HEAP_DESC temp_heap_desc{};
                temp_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                temp_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
                // For single-adapter operation, set this to zero.
                // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_descriptor_heap_desc
                temp_heap_desc.NodeMask = 0;
                temp_heap_desc.NumDescriptors = 2 * D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2;

                wrl::com_ptr<ID3D12DescriptorHeap> temp_heap;
                HRESULT hr = device->CreateDescriptorHeap(&temp_heap_desc, IID_PPV_ARGS(temp_heap.GetAddressOf()));
                if (DX_SUCCESS(hr))
                {
                    return -1;
                }
                else
                {
                    return D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2;
                }

#if _DEBUG
                if (renderer::g_is_debug_layer_enabled)
                {
                    if (info_queue)
                    {
                        info_queue->PopStorageFilter();
                    }
                }
#endif
            }
            break;
            default:
                return D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2;
            }
        }
        //-------------------------------------------------------------------------
        s32 get_max_sampler_descriptor_count()
        {
            return D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
        }

        //-------------------------------------------------------------------------
        D3D_ROOT_SIGNATURE_VERSION get_root_signature_version(wrl::com_ptr<ID3D12Device> device)
        {
            D3D12_FEATURE_DATA_ROOT_SIGNATURE d3d12_root_signature_caps = {};

            // This is the highest version we currently support.
            // If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
            d3d12_root_signature_caps.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
            if (DX_FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &d3d12_root_signature_caps, sizeof(d3d12_root_signature_caps))))
            {
                d3d12_root_signature_caps.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
            }

            return d3d12_root_signature_caps.HighestVersion;
        }

        namespace internal
        {
            //-------------------------------------------------------------------------
            /**
             * Always enable the debug layer before doing anything DX12 related so all possible errors generated while creating
             * DX12 objects are caught by the debug layer.
             */
            bool enable_debug_layer()
            {
#if CERA_PLATFORM_WINDOWS
                // Always enable the debug layer before doing anything DX12 related
                // so all possible errors generated while creating DX12 objects
                // are caught by the debug layer.
                wrl::com_ptr<ID3D12Debug> debug_interface;
                if (DX_FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface))))
                {
                    log::error("The debug interface requires the D3D12 SDK Layers. Please install the Graphics Tools for Windows. See: "
                               "https://docs.microsoft.com/en-us/windows/uwp/gaming/use-the-directx-runtime-and-visual-studio-graphics-diagnostic-features");
                    log::error("Unable to get D3D12DebugInterface");

                    return false;
                }

                debug_interface->EnableDebugLayer();

                if (g_with_gpu_validation)
                {
                    wrl::com_ptr<ID3D12Debug1> debug_interface1;
                    if (DX_SUCCESS(debug_interface->QueryInterface(IID_PPV_ARGS(debug_interface1.GetAddressOf()))))
                    {
                        debug_interface1->SetEnableGPUBasedValidation(true);
                    }
                }
#endif

                return true;
            }

            //-------------------------------------------------------------------------
            wrl::com_ptr<ID3D12Device2> create_device(const std::shared_ptr<adapter>& adapter, bool is_debug_layer_enabled)
            {
                if (is_debug_layer_enabled)
                {
                    if (!enable_debug_layer())
                    {
                        log::error("Unable to create ID3D12Debug");
                        return false;
                    }
                }

                wrl::com_ptr<ID3D12Device2> d3d12_device;
                if (DX_FAILED((D3D12CreateDevice(const_cast<IDXGIAdapter*>(adapter->com_ptr()), adapter->description().max_supported_feature_level, IID_PPV_ARGS(&d3d12_device)))))
                {
                    log::error("Unable to create D3D12Device");
                    return nullptr;
                }

                // Enable debug messages in debug mode.
#if CERA_PLATFORM_WINDOWS
                if (is_debug_layer_enabled)
                {
                    wrl::com_ptr<ID3D12InfoQueue> info_queue;
                    if (DX_SUCCESS(d3d12_device.As(&info_queue)))
                    {
                        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
                        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

                        // Suppress messages based on their severity level
                        D3D12_MESSAGE_SEVERITY severities[] = {D3D12_MESSAGE_SEVERITY_INFO};

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
                }
#endif

                return d3d12_device;
            }

            //-------------------------------------------------------------------------
            bool check_tearing_support(std::shared_ptr<adapter> adapter)
            {
                bool allow_tearing = false;

                // Get the factory that was used to create the adapter.
                wrl::com_ptr<IDXGIFactory> dxgi_factory;
                wrl::com_ptr<IDXGIFactory5> dxgi_factory5;
                if (DX_SUCCESS(adapter->com_ptr()->GetParent(IID_PPV_ARGS(&dxgi_factory))))
                {
                    // Now get the DXGIFactory5 so I can use the IDXGIFactory5::CheckFeatureSupport method.
                    if (DX_SUCCESS(dxgi_factory.As(&dxgi_factory5)))
                    {
                        BOOL dxgi_allow_tearing = FALSE;
                        if (DX_SUCCESS(dxgi_factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &dxgi_allow_tearing, sizeof(BOOL))))
                        {
                            allow_tearing = dxgi_allow_tearing == TRUE;
                        }
                    }
                }

                return allow_tearing;
            }
        } // namespace internal

        namespace adaptors
        {
            class MakeUnorderedAccessView : public UnorderedAccessView
            {
              public:
                MakeUnorderedAccessView(d3d12_device& device, const std::shared_ptr<resource>& inResource, const std::shared_ptr<resource>& inCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav)
                    : UnorderedAccessView(device, inResource, inCounterResource, uav)
                {
                }

                ~MakeUnorderedAccessView() override = default;
            };

            class MakeShaderResourceView : public ShaderResourceView
            {
              public:
                MakeShaderResourceView(d3d12_device& device, const std::shared_ptr<resource>& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srv) : ShaderResourceView(device, resource, srv)
                {
                }

                ~MakeShaderResourceView() override = default;
            };

            class MakeConstantBufferView : public ConstantBufferView
            {
              public:
                MakeConstantBufferView(d3d12_device& device, const std::shared_ptr<ConstantBuffer>& constantBuffer, size_t offset) : ConstantBufferView(device, constantBuffer, offset)
                {
                }

                ~MakeConstantBufferView() override = default;
            };

            class MakePipelineStateObject : public PipelineStateObject
            {
              public:
                MakePipelineStateObject(d3d12_device& device, const D3D12_PIPELINE_STATE_STREAM_DESC& desc) : PipelineStateObject(device, desc)
                {
                }

                ~MakePipelineStateObject() override = default;
            };

            class MakeRootSignature : public RootSignature
            {
              public:
                MakeRootSignature(d3d12_device& device, const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc) : RootSignature(device, rootSignatureDesc)
                {
                }

                ~MakeRootSignature()
                {
                }
            };

            class MakeTexture : public Texture
            {
              public:
                MakeTexture(d3d12_device& device, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue) : Texture(device, resourceDesc, clearValue)
                {
                }

                MakeTexture(d3d12_device& device, wrl::com_ptr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue) : Texture(device, resource, clearValue)
                {
                }

                ~MakeTexture() override = default;
            };

            class MakeVertexBuffer : public VertexBuffer
            {
              public:
                MakeVertexBuffer(d3d12_device& device, size_t numVertices, size_t vertexStride) : VertexBuffer(device, numVertices, vertexStride)
                {
                }

                MakeVertexBuffer(d3d12_device& device, wrl::com_ptr<ID3D12Resource> resource, size_t numVertices, size_t vertexStride) : VertexBuffer(device, resource, numVertices, vertexStride)
                {
                }

                ~MakeVertexBuffer() override = default;
            };

            class MakeIndexBuffer : public IndexBuffer
            {
              public:
                MakeIndexBuffer(d3d12_device& device, size_t numIndices, DXGI_FORMAT indexFormat) : IndexBuffer(device, numIndices, indexFormat)
                {
                }

                MakeIndexBuffer(d3d12_device& device, wrl::com_ptr<ID3D12Resource> resource, size_t numIndices, DXGI_FORMAT indexFormat) : IndexBuffer(device, resource, numIndices, indexFormat)
                {
                }

                ~MakeIndexBuffer() override = default;
            };

            class MakeConstantBuffer : public ConstantBuffer
            {
              public:
                MakeConstantBuffer(d3d12_device& device, wrl::com_ptr<ID3D12Resource> resource) : ConstantBuffer(device, resource)
                {
                }

                ~MakeConstantBuffer() override = default;
            };

            class MakeByteAddressBuffer : public ByteAddressBuffer
            {
              public:
                MakeByteAddressBuffer(d3d12_device& device, const D3D12_RESOURCE_DESC& desc) : ByteAddressBuffer(device, desc)
                {
                }

                MakeByteAddressBuffer(d3d12_device& device, wrl::com_ptr<ID3D12Resource> resource) : ByteAddressBuffer(device, resource)
                {
                }

                ~MakeByteAddressBuffer() override = default;
            };

            class MakeDescriptorAllocator : public DescriptorAllocator
            {
              public:
                MakeDescriptorAllocator(d3d12_device& device, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 numDescriptorsPerHeap = 256) : DescriptorAllocator(device, type, numDescriptorsPerHeap)
                {
                }

                ~MakeDescriptorAllocator() override = default;
            };

            class MakeCommandQueue : public CommandQueue
            {
              public:
                MakeCommandQueue(d3d12_device& device, D3D12_COMMAND_LIST_TYPE type) : CommandQueue(device, type)
                {
                }

                ~MakeCommandQueue() override = default;
            };

            class make_device : public d3d12_device
            {
              public:
                make_device(const std::shared_ptr<adapter>& adaptor, wrl::com_ptr<ID3D12Device2> device, bool is_debug) 
                    : d3d12_device(adaptor, device, is_debug)
                {
                }

                virtual ~make_device() override = default;
            };
        } // namespace adaptors

        void d3d12_device::report_live_objects()
        {
            IDXGIDebug1* dxgi_debug;
            DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgi_debug));

            dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
            dxgi_debug->Release();
        }

        std::shared_ptr<d3d12_device> d3d12_device::create(const std::shared_ptr<adapter>& in_adapter, bool in_enable_debug_layer)
        {
            if (in_adapter)
            {
                wrl::com_ptr<ID3D12Device2> d3d_device = internal::create_device(in_adapter, in_enable_debug_layer);

                if (d3d_device)
                {
                    return std::make_shared<adaptors::make_device>(in_adapter, d3d_device, in_enable_debug_layer);
                }
                else
                {
                    log::error("Unable to create ID3D12Device2");
                    return nullptr;
                }
            }

            log::error("Unable to create IDXGIAdapter4");
            return nullptr;
        }

        d3d12_device::d3d12_device(std::shared_ptr<adapter> adaptor, wrl::com_ptr<ID3D12Device2> d3dDevice, bool is_debug)
            : wrl::ComObject<ID3D12Device2>(d3dDevice)
            , m_adapter(adaptor)
            , m_direct_command_queue(nullptr)
            , m_compute_command_queue(nullptr)
            , m_copy_command_queue(nullptr)
            , m_tearing_supported(false)
            , m_is_debug(is_debug)
        {
            CERA_ASSERT_X(m_adapter != nullptr, "Invalid Adaptor given");

            m_direct_command_queue = std::make_unique<adaptors::MakeCommandQueue>(*this, D3D12_COMMAND_LIST_TYPE_DIRECT);
            m_compute_command_queue = std::make_unique<adaptors::MakeCommandQueue>(*this, D3D12_COMMAND_LIST_TYPE_COMPUTE);
            m_copy_command_queue = std::make_unique<adaptors::MakeCommandQueue>(*this, D3D12_COMMAND_LIST_TYPE_COPY);

            m_tearing_supported = internal::check_tearing_support(adaptor);

            // Create descriptor allocators
            {
                for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
                {
                    m_descriptor_allocators[i] = std::make_unique<adaptors::MakeDescriptorAllocator>(*this, static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
                }
            }

            // Check features.
            {
                m_max_none_sampler_descriptors = get_max_none_sampler_descriptor_count(d3dDevice, m_adapter->description().resource_binding_tier);
                m_max_sampler_descriptors = get_max_sampler_descriptor_count();

                m_highest_root_signature_version = get_root_signature_version(d3dDevice);
            }
        }

        d3d12_device::~d3d12_device() = default;

        const adapter_description& d3d12_device::adapter_description() const
        {
            return m_adapter->description();
        }

        IDXGIAdapter* d3d12_device::dxgi_adapter()
        {
            return m_adapter->c_ptr();
        }

        const IDXGIAdapter* d3d12_device::dxgi_adapter() const
        {
            return m_adapter->c_ptr();
        }

        ID3D12Device2* d3d12_device::d3d_device()
        {
            return c_ptr();
        }

        const ID3D12Device2* d3d12_device::d3d_device() const
        {
            return c_ptr();
        }

        CommandQueue& d3d12_device::command_queue(D3D12_COMMAND_LIST_TYPE type)
        {
            CommandQueue* command_queue = nullptr;

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

        void d3d12_device::flush()
        {
            m_direct_command_queue->flush();
            m_compute_command_queue->flush();
            m_copy_command_queue->flush();
        }

        DescriptorAllocation d3d12_device::allocate_descriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, u32 numDescriptors)
        {
            return m_descriptor_allocators[type]->allocate(numDescriptors);
        }

        void d3d12_device::release_stale_descriptors()
        {
            for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
            {
                m_descriptor_allocators[i]->release_stale_descriptors();
            }
        }

        std::shared_ptr<ConstantBuffer> d3d12_device::create_constant_buffer(wrl::com_ptr<ID3D12Resource> resource)
        {
            std::shared_ptr<ConstantBuffer> constant_buffer = std::make_shared<adaptors::MakeConstantBuffer>(*this, resource);

            return constant_buffer;
        }

        std::shared_ptr<ByteAddressBuffer> d3d12_device::create_byte_address_buffer(memory_size bufferSize)
        {
            // Align-up to 4-bytes
            auto aligned_buffer_size = align_up(bufferSize.size_in_bytes(), 4);

            std::shared_ptr<ByteAddressBuffer> buffer = std::make_shared<adaptors::MakeByteAddressBuffer>(*this, CD3DX12_RESOURCE_DESC::Buffer(aligned_buffer_size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS));

            return buffer;
        }

        std::shared_ptr<ByteAddressBuffer> d3d12_device::create_byte_address_buffer(wrl::com_ptr<ID3D12Resource> resource)
        {
            std::shared_ptr<ByteAddressBuffer> buffer = std::make_shared<adaptors::MakeByteAddressBuffer>(*this, resource);

            return buffer;
        }

        std::shared_ptr<IndexBuffer> d3d12_device::create_index_buffer(size_t numIndices, DXGI_FORMAT indexFormat)
        {
            std::shared_ptr<IndexBuffer> index_buffer = std::make_shared<adaptors::MakeIndexBuffer>(*this, numIndices, indexFormat);

            return index_buffer;
        }

        std::shared_ptr<IndexBuffer> d3d12_device::create_index_buffer(wrl::com_ptr<ID3D12Resource> resource, size_t numIndices, DXGI_FORMAT indexFormat)
        {
            std::shared_ptr<IndexBuffer> index_buffer = std::make_shared<adaptors::MakeIndexBuffer>(*this, resource, numIndices, indexFormat);

            return index_buffer;
        }

        std::shared_ptr<VertexBuffer> d3d12_device::create_vertex_buffer(size_t numVertices, size_t vertexStride)
        {
            std::shared_ptr<VertexBuffer> vertex_buffer = std::make_shared<adaptors::MakeVertexBuffer>(*this, numVertices, vertexStride);

            return vertex_buffer;
        }

        std::shared_ptr<VertexBuffer> d3d12_device::create_vertex_buffer(wrl::com_ptr<ID3D12Resource> resource, size_t numVertices, size_t vertexStride)
        {
            std::shared_ptr<VertexBuffer> vertex_buffer = std::make_shared<adaptors::MakeVertexBuffer>(*this, resource, numVertices, vertexStride);

            return vertex_buffer;
        }

        std::shared_ptr<Texture> d3d12_device::create_texture(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue)
        {
            std::shared_ptr<Texture> texture = std::make_shared<adaptors::MakeTexture>(*this, resourceDesc, clearValue);

            return texture;
        }

        std::shared_ptr<Texture> d3d12_device::create_texture(wrl::com_ptr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue)
        {
            std::shared_ptr<Texture> texture = std::make_shared<adaptors::MakeTexture>(*this, resource, clearValue);

            return texture;
        }

        std::shared_ptr<RootSignature> d3d12_device::create_root_signature(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc)
        {
            std::shared_ptr<RootSignature> RootSignature = std::make_shared<adaptors::MakeRootSignature>(*this, rootSignatureDesc);

            return RootSignature;
        }

        std::shared_ptr<PipelineStateObject> d3d12_device::do_create_pipeline_state_object(const D3D12_PIPELINE_STATE_STREAM_DESC& pipelineStateStreamDesc)
        {
            std::shared_ptr<PipelineStateObject> pipeline_state_object = std::make_shared<adaptors::MakePipelineStateObject>(*this, pipelineStateStreamDesc);

            return pipeline_state_object;
        }

        std::shared_ptr<ConstantBufferView> d3d12_device::create_constant_buffer_view(const std::shared_ptr<ConstantBuffer>& constant_buffer, size_t offset)
        {
            std::shared_ptr<ConstantBufferView> constant_buffer_view = std::make_shared<adaptors::MakeConstantBufferView>(*this, constant_buffer, offset);

            return constant_buffer_view;
        }

        std::shared_ptr<ShaderResourceView> d3d12_device::create_shader_resource_view(const std::shared_ptr<resource>& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srv)
        {
            std::shared_ptr<ShaderResourceView> ShaderResourceView = std::make_shared<adaptors::MakeShaderResourceView>(*this, resource, srv);

            return ShaderResourceView;
        }

        std::shared_ptr<UnorderedAccessView> d3d12_device::create_unordered_access_view(const std::shared_ptr<resource>& inResource, const std::shared_ptr<resource>& inCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav)
        {
            std::shared_ptr<UnorderedAccessView> unordered_access_view = std::make_shared<adaptors::MakeUnorderedAccessView>(*this, inResource, inCounterResource, uav);

            return unordered_access_view;
        }

        D3D_ROOT_SIGNATURE_VERSION d3d12_device::highest_root_signature_version() const
        {
            return m_highest_root_signature_version;
        }

        DXGI_SAMPLE_DESC d3d12_device::multisample_quality_levels(DXGI_FORMAT format, u32 numSamples, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags) const
        {
            DXGI_SAMPLE_DESC sample_desc = {1, 0};

            D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS quality_levels;
            quality_levels.Format = format;
            quality_levels.SampleCount = 1;
            quality_levels.Flags = flags;
            quality_levels.NumQualityLevels = 0;

            auto device = const_cast<ID3D12Device2*>(d3d_device());

            while (quality_levels.SampleCount <= numSamples && SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &quality_levels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS))) &&
                   quality_levels.NumQualityLevels > 0)
            {
                // That works...
                sample_desc.Count = quality_levels.SampleCount;
                sample_desc.Quality = quality_levels.NumQualityLevels - 1;

                // But can we do better?
                quality_levels.SampleCount *= 2;
            }

            return sample_desc;
        }

        bool d3d12_device::is_tearing_supported() const
        {
            return m_tearing_supported;
        }
    } // namespace renderer
} // namespace cera