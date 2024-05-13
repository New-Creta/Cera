#include "command_list.h"
#include "command_queue.h"

#include "directx_call.h"
#include "directx_util.h"
#include "directx_device.h"
#include "directx_swapchain.h"

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

#include "util/log.h"
#include "util/assert.h"
#include "util/pointer_math.h"

#include "dxgi/objects/adapter.h"
#include "dxgi/objects/factory.h"
#include "dxgi/dxgi_adapter_manager.h"

#include "gpu_description.h"

#include <dxgi1_5.h>
#include <dxgidebug.h>

namespace cera
{
  namespace renderer
  {
    namespace internal
    {
      //-------------------------------------------------------------------------
      size_t highest_scoring_gpu(const std::vector<GpuDescription>& gpus)
      {
        auto it = std::max_element(gpus.cbegin(), gpus.cend(),
                                   [](const GpuDescription& lhs, const GpuDescription& rhs)
                                   {
                                     const size_t lhs_vram = lhs.dedicated_video_memory.size_in_bytes();
                                     const size_t rhs_vram = rhs.dedicated_video_memory.size_in_bytes();

                                     return rhs_vram > lhs_vram;
                                   });

        return it != gpus.cend() ? std::distance(gpus.cbegin(), it) : -1;
      }

      wrl::ComPtr<ID3D12Device2> create_device(const std::shared_ptr<dxgi::Adapter>& adaptor)
      {
        wrl::ComPtr<ID3D12Device2> d3d12_device;
        if(DX_FAILED((D3D12CreateDevice(const_cast<IDXGIAdapter4*>(adaptor->com_ptr()), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&d3d12_device)))))
        {
          log::error("Unable to create D3D12Device");
          return nullptr;
        }

// Enable debug messages in debug mode.
#if defined(CERA_BUILD_DEBUG)
        wrl::ComPtr<ID3D12InfoQueue> info_queue;
        if(DX_SUCCESS(d3d12_device.As(&info_queue)))
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
          new_filter.DenyList.NumSeverities  = _countof(severities);
          new_filter.DenyList.pSeverityList  = severities;
          new_filter.DenyList.NumIDs         = _countof(deny_ids);
          new_filter.DenyList.pIDList        = deny_ids;

          if(DX_FAILED(info_queue->PushStorageFilter(&new_filter)))
          {
            log::error("Unable to push StorageFilter");
            return nullptr;
          }
        }
#endif

        return d3d12_device;
      }

      bool check_tearing_support(std::shared_ptr<dxgi::Adapter> adapter)
      {
        bool allow_tearing = false;

        // Get the factory that was used to create the adapter.
        wrl::ComPtr<IDXGIFactory> dxgi_factory;
        wrl::ComPtr<IDXGIFactory5> dxgi_factory5;
        if(DX_SUCCESS(adapter->com_ptr()->GetParent(IID_PPV_ARGS(&dxgi_factory))))
        {
          // Now get the DXGIFactory5 so I can use the IDXGIFactory5::CheckFeatureSupport method.
            if (DX_SUCCESS(dxgi_factory.As(&dxgi_factory5)))
            {
                BOOL dxgi_allow_tearing = FALSE;
                if(DX_SUCCESS(dxgi_factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &dxgi_allow_tearing, sizeof(BOOL))))
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
        MakeUnorderedAccessView(Device& device, const std::shared_ptr<Resource>& inResource, const std::shared_ptr<Resource>& inCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav)
            : UnorderedAccessView(device, inResource, inCounterResource, uav)
        {
        }

        ~MakeUnorderedAccessView() override = default;
      };

      class MakeShaderResourceView : public ShaderResourceView
      {
      public:
        MakeShaderResourceView(Device& device, const std::shared_ptr<Resource>& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srv)
            : ShaderResourceView(device, resource, srv)
        {
        }

        ~MakeShaderResourceView() override = default;
      };

      class MakeConstantBufferView : public ConstantBufferView
      {
      public:
        MakeConstantBufferView(Device& device, const std::shared_ptr<ConstantBuffer>& constantBuffer, size_t offset)
            : ConstantBufferView(device, constantBuffer, offset)
        {
        }

        ~MakeConstantBufferView() override = default;
      };

      class MakePipelineStateObject : public PipelineStateObject
      {
      public:
        MakePipelineStateObject(Device& device, const D3D12_PIPELINE_STATE_STREAM_DESC& desc)
            : PipelineStateObject(device, desc)
        {
        }

        ~MakePipelineStateObject() override = default;
      };
      class MakeRootSignature : public RootSignature
      {
      public:
        MakeRootSignature(Device& device, const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc)
            : RootSignature(device, rootSignatureDesc)
        {
        }

        ~MakeRootSignature() {}
      };

      class MakeTexture : public Texture
      {
      public:
        MakeTexture(Device& device, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue)
            : Texture(device, resourceDesc, clearValue)
        {
        }

        MakeTexture(Device& device, wrl::ComPtr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue)
            : Texture(device, resource, clearValue)
        {
        }

        ~MakeTexture() override = default;
      };

      class MakeVertexBuffer : public VertexBuffer
      {
      public:
        MakeVertexBuffer(Device& device, size_t numVertices, size_t vertexStride)
            : VertexBuffer(device, numVertices, vertexStride)
        {
        }

        MakeVertexBuffer(Device& device, wrl::ComPtr<ID3D12Resource> resource, size_t numVertices, size_t vertexStride)
            : VertexBuffer(device, resource, numVertices, vertexStride)
        {
        }

        ~MakeVertexBuffer() override = default;
      };

      class MakeIndexBuffer : public IndexBuffer
      {
      public:
        MakeIndexBuffer(Device& device, size_t numIndices, DXGI_FORMAT indexFormat)
            : IndexBuffer(device, numIndices, indexFormat)
        {
        }

        MakeIndexBuffer(Device& device, wrl::ComPtr<ID3D12Resource> resource, size_t numIndices, DXGI_FORMAT indexFormat)
            : IndexBuffer(device, resource, numIndices, indexFormat)
        {
        }

        ~MakeIndexBuffer() override = default;
      };

      class MakeConstantBuffer : public ConstantBuffer
      {
      public:
        MakeConstantBuffer(Device& device, wrl::ComPtr<ID3D12Resource> resource)
            : ConstantBuffer(device, resource)
        {
        }

        ~MakeConstantBuffer() override = default;
      };

      class MakeByteAddressBuffer : public ByteAddressBuffer
      {
      public:
        MakeByteAddressBuffer(Device& device, const D3D12_RESOURCE_DESC& desc)
            : ByteAddressBuffer(device, desc)
        {
        }

        MakeByteAddressBuffer(Device& device, wrl::ComPtr<ID3D12Resource> resource)
            : ByteAddressBuffer(device, resource)
        {
        }

        ~MakeByteAddressBuffer() override = default;
      };

      class MakeDescriptorAllocator : public DescriptorAllocator
      {
      public:
        MakeDescriptorAllocator(Device& device, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 numDescriptorsPerHeap = 256)
            : DescriptorAllocator(device, type, numDescriptorsPerHeap)
        {
        }

        ~MakeDescriptorAllocator() override = default;
      };

      class MakeCommandQueue : public CommandQueue
      {
      public:
        MakeCommandQueue(Device& device, D3D12_COMMAND_LIST_TYPE type)
            : CommandQueue(device, type)
        {
        }

        ~MakeCommandQueue() override = default;
      };

      class MakeDevice : public Device
      {
      public:
        MakeDevice(const std::shared_ptr<dxgi::Adapter>& adaptor, wrl::ComPtr<ID3D12Device2> device)
            : Device(adaptor, device)
        {
        }

        virtual ~MakeDevice() override = default;
      };
    } // namespace adaptors

    bool Device::enable_debug_layer()
    {

      // Always enable the debug layer before doing anything DX12 related
      // so all possible errors generated while creating DX12 objects
      // are caught by the debug layer.
      wrl::ComPtr<ID3D12Debug> debug_interface;
      if(DX_FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface))))
      {
        log::error("Unable to get D3D12DebugInterface");
        return false;
      }

      debug_interface->EnableDebugLayer();

      wrl::ComPtr<ID3D12Debug1> debug_interface1;

      debug_interface.As(&debug_interface1);
      debug_interface1->SetEnableGPUBasedValidation(true);

      return true;
    }

    void Device::report_live_objects()
    {
      IDXGIDebug1* dxgi_debug;
      DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgi_debug));

      dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
      dxgi_debug->Release();
    }

    std::shared_ptr<Device> Device::create()
    {
      if(!enable_debug_layer())
      {
        log::error("Unable to create ID3D12Debug");
        return false;
      }

    u32 create_factory_flags = 0;
#if defined(CERA_BUILD_DEBUG)
      create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
      // Create dxgi factory
      std::unique_ptr<dxgi::Factory> factory = dxgi::Factory::create(create_factory_flags);
      if(!factory)
      {
        log::error("Failed to create DXGI Factory");
        return false;
      }

      // Find highest scoring gpu
      const dxgi::AdapterManager adapter_manager(factory.get(), &internal::highest_scoring_gpu);
      const std::shared_ptr<dxgi::Adapter> selected_gpu = adapter_manager.selected();

      if(selected_gpu)
      {
        wrl::ComPtr<ID3D12Device2> d3d_device = internal::create_device(selected_gpu);

        if(d3d_device)
        {
          return std::make_shared<adaptors::MakeDevice>(selected_gpu, d3d_device);
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

    Device::Device(std::shared_ptr<dxgi::Adapter> adaptor, wrl::ComPtr<ID3D12Device2> d3dDevice)
        : wrl::ComObject<ID3D12Device2>(std::move(d3dDevice))
        , m_adapter(adaptor)
        , m_direct_command_queue(nullptr)
        , m_compute_command_queue(nullptr)
        , m_copy_command_queue(nullptr)
        , m_tearing_supported(false)
    {
      CERA_ASSERT_X(m_adapter != nullptr, "Invalid Adaptor given");

      m_direct_command_queue  = std::make_unique<adaptors::MakeCommandQueue>(*this, D3D12_COMMAND_LIST_TYPE_DIRECT);
      m_compute_command_queue = std::make_unique<adaptors::MakeCommandQueue>(*this, D3D12_COMMAND_LIST_TYPE_COMPUTE);
      m_copy_command_queue    = std::make_unique<adaptors::MakeCommandQueue>(*this, D3D12_COMMAND_LIST_TYPE_COPY);

      m_tearing_supported = internal::check_tearing_support(adaptor);

      // Create descriptor allocators
      {
        for(int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        {
          m_descriptor_allocators[i] = std::make_unique<adaptors::MakeDescriptorAllocator>(*this, static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
        }
      }

      // Check features.
      {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE feature_data;
        feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if(DX_FAILED(d3d_device()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &feature_data, sizeof(D3D12_FEATURE_DATA_ROOT_SIGNATURE))))
        {
          feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        m_highest_root_signature_version = feature_data.HighestVersion;
      }
    }

    Device::~Device() = default;

    const GpuDescription& Device::adapter_description() const
    {
      return m_adapter->description();
    }

    IDXGIAdapter4* Device::dxgi_adapter()
    {
      return m_adapter->c_ptr();
    }

    const IDXGIAdapter4* Device::dxgi_adapter() const
    {
      return m_adapter->c_ptr();
    }

    ID3D12Device2* Device::d3d_device()
    {
      return c_ptr();
    }

    const ID3D12Device2* Device::d3d_device() const
    {
      return c_ptr();
    }

    CommandQueue& Device::command_queue(D3D12_COMMAND_LIST_TYPE type)
    {
      CommandQueue* command_queue = nullptr;

      switch(type)
      {
        case D3D12_COMMAND_LIST_TYPE_DIRECT: command_queue = m_direct_command_queue.get(); break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE: command_queue = m_compute_command_queue.get(); break;
        case D3D12_COMMAND_LIST_TYPE_COPY: command_queue = m_copy_command_queue.get(); break;
        default: assert(false && "Invalid D3D12_COMMAND_LIST_TYPE.");
      }

      return *command_queue;
    }

    void Device::flush()
    {
      m_direct_command_queue->flush();
      m_compute_command_queue->flush();
      m_copy_command_queue->flush();
    }

    DescriptorAllocation Device::allocate_descriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, u32 numDescriptors)
    {
      return m_descriptor_allocators[type]->allocate(numDescriptors);
    }

    void Device::release_stale_descriptors()
    {
      for(int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
      {
        m_descriptor_allocators[i]->release_stale_descriptors();
      }
    }

    std::shared_ptr<ConstantBuffer> Device::create_constant_buffer(wrl::ComPtr<ID3D12Resource> resource)
    {
      std::shared_ptr<ConstantBuffer> constant_buffer = std::make_shared<adaptors::MakeConstantBuffer>(*this, resource);

      return constant_buffer;
    }

    std::shared_ptr<ByteAddressBuffer> Device::create_byte_address_buffer(memory_size bufferSize)
    {
      // Align-up to 4-bytes
      auto aligned_buffer_size = align_up(bufferSize.size_in_bytes(), 4);

      std::shared_ptr<ByteAddressBuffer> buffer = std::make_shared<adaptors::MakeByteAddressBuffer>(*this, CD3DX12_RESOURCE_DESC::Buffer(aligned_buffer_size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS));

      return buffer;
    }

    std::shared_ptr<ByteAddressBuffer> Device::create_byte_address_buffer(wrl::ComPtr<ID3D12Resource> resource)
    {
      std::shared_ptr<ByteAddressBuffer> buffer = std::make_shared<adaptors::MakeByteAddressBuffer>(*this, resource);

      return buffer;
    }

    std::shared_ptr<IndexBuffer> Device::create_index_buffer(size_t numIndices, DXGI_FORMAT indexFormat)
    {
      std::shared_ptr<IndexBuffer> index_buffer = std::make_shared<adaptors::MakeIndexBuffer>(*this, numIndices, indexFormat);

      return index_buffer;
    }

    std::shared_ptr<IndexBuffer> Device::create_index_buffer(wrl::ComPtr<ID3D12Resource> resource, size_t numIndices, DXGI_FORMAT indexFormat)
    {
      std::shared_ptr<IndexBuffer> index_buffer = std::make_shared<adaptors::MakeIndexBuffer>(*this, resource, numIndices, indexFormat);

      return index_buffer;
    }

    std::shared_ptr<VertexBuffer> Device::create_vertex_buffer(size_t numVertices, size_t vertexStride)
    {
      std::shared_ptr<VertexBuffer> vertex_buffer = std::make_shared<adaptors::MakeVertexBuffer>(*this, numVertices, vertexStride);

      return vertex_buffer;
    }

    std::shared_ptr<VertexBuffer> Device::create_vertex_buffer(wrl::ComPtr<ID3D12Resource> resource, size_t numVertices, size_t vertexStride)
    {
      std::shared_ptr<VertexBuffer> vertex_buffer = std::make_shared<adaptors::MakeVertexBuffer>(*this, resource, numVertices, vertexStride);

      return vertex_buffer;
    }

    std::shared_ptr<Texture> Device::create_texture(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue)
    {
      std::shared_ptr<Texture> texture = std::make_shared<adaptors::MakeTexture>(*this, resourceDesc, clearValue);

      return texture;
    }

    std::shared_ptr<Texture> Device::create_texture(wrl::ComPtr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue)
    {
      std::shared_ptr<Texture> texture = std::make_shared<adaptors::MakeTexture>(*this, resource, clearValue);

      return texture;
    }

    std::shared_ptr<RootSignature> Device::create_root_signature(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc)
    {
      std::shared_ptr<RootSignature> RootSignature = std::make_shared<adaptors::MakeRootSignature>(*this, rootSignatureDesc);

      return RootSignature;
    }

    std::shared_ptr<PipelineStateObject> Device::do_create_pipeline_state_object(const D3D12_PIPELINE_STATE_STREAM_DESC& pipelineStateStreamDesc)
    {
      std::shared_ptr<PipelineStateObject> pipeline_state_object = std::make_shared<adaptors::MakePipelineStateObject>(*this, pipelineStateStreamDesc);

      return pipeline_state_object;
    }

    std::shared_ptr<ConstantBufferView> Device::create_constant_buffer_view(const std::shared_ptr<ConstantBuffer>& constant_buffer, size_t offset)
    {
      std::shared_ptr<ConstantBufferView> constant_buffer_view = std::make_shared<adaptors::MakeConstantBufferView>(*this, constant_buffer, offset);

      return constant_buffer_view;
    }

    std::shared_ptr<ShaderResourceView> Device::create_shader_resource_view(const std::shared_ptr<Resource>& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srv)
    {
      std::shared_ptr<ShaderResourceView> ShaderResourceView = std::make_shared<adaptors::MakeShaderResourceView>(*this, resource, srv);

      return ShaderResourceView;
    }

    std::shared_ptr<UnorderedAccessView> Device::create_unordered_access_view(const std::shared_ptr<Resource>& inResource, const std::shared_ptr<Resource>& inCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav)
    {
      std::shared_ptr<UnorderedAccessView> unordered_access_view = std::make_shared<adaptors::MakeUnorderedAccessView>(*this, inResource, inCounterResource, uav);

      return unordered_access_view;
    }

    D3D_ROOT_SIGNATURE_VERSION Device::highest_root_signature_version() const
    {
      return m_highest_root_signature_version;
    }

    DXGI_SAMPLE_DESC Device::multisample_quality_levels(DXGI_FORMAT format, u32 numSamples, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags) const
    {
      DXGI_SAMPLE_DESC sample_desc = {1, 0};

      D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS quality_levels;
      quality_levels.Format           = format;
      quality_levels.SampleCount      = 1;
      quality_levels.Flags            = flags;
      quality_levels.NumQualityLevels = 0;

      auto device = const_cast<ID3D12Device2*>(d3d_device());

      while(quality_levels.SampleCount <= numSamples && SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &quality_levels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS))) &&
            quality_levels.NumQualityLevels > 0)
      {
        // That works...
        sample_desc.Count   = quality_levels.SampleCount;
        sample_desc.Quality = quality_levels.NumQualityLevels - 1;

        // But can we do better?
        quality_levels.SampleCount *= 2;
      }

      return sample_desc;
    }

    bool Device::is_tearing_supported() const
    {
      return m_tearing_supported;
    }
  } // namespace renderer
} // namespace cera