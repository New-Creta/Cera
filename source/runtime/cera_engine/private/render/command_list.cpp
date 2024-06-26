#include "render/command_list.h"

#include "render/resource_state_tracker.h"
#include "render/upload_buffer.h"
#include "render/dynamic_descriptor_heap.h"
#include "render/root_signature.h"
#include "render/device.h"
#include "render/resource.h"
#include "render/vertex_buffer.h"
#include "render/index_buffer.h"
#include "render/byte_address_buffer.h"
#include "render/texture.h"
#include "render/d3dx12_call.h"
#include "render/render_target.h"
#include "render/pipeline_state_object.h"
#include "render/shader_resource_view.h"
#include "render/constant_buffer_view.h"
#include "render/unordered_access_view.h"
#include "render/constant_buffer.h"

#include "util/log.h"

namespace cera
{
    namespace adaptors
    {
        // Adapter for std::make_unique
        class make_upload_buffer : public upload_buffer
        {
        public:
            make_upload_buffer(device& device, size_t pageSize = _2MB)
                : upload_buffer(device, pageSize)
            {}

            virtual ~make_upload_buffer() {}
        };
    }

    namespace conversions
    {
        static std::string to_string(D3D12_COMMAND_LIST_TYPE type)
        {
            switch(type)
            {
            case D3D12_COMMAND_LIST_TYPE_DIRECT: return "D3D12_COMMAND_LIST_TYPE_DIRECT";
            case D3D12_COMMAND_LIST_TYPE_COPY: return "D3D12_COMMAND_LIST_TYPE_COPY";
            case D3D12_COMMAND_LIST_TYPE_COMPUTE: return "D3D12_COMMAND_LIST_TYPE_COMPUTE";
            }

            return "UNKNOWN";
        }
    }

    command_list::command_list(device& device, D3D12_COMMAND_LIST_TYPE type)
        : m_device(device)
        , m_d3d_command_list_type(type)
        , m_root_signature(nullptr)
        , m_pipeline_state(nullptr)
    {
        log::info("Created a new CommandList of type: {0} - Instance nr: {1}", conversions::to_string(type), instance_nr());

        auto d3d_device = m_device.get_d3d_device();

        HRESULT hr = S_OK;

        hr = d3d_device->CreateCommandAllocator(m_d3d_command_list_type, IID_PPV_ARGS(&m_d3d_command_allocator));
        assert(SUCCEEDED(hr));
        hr = d3d_device->CreateCommandList(0, m_d3d_command_list_type, m_d3d_command_allocator.Get(), nullptr, IID_PPV_ARGS(&m_d3d_command_list));
        assert(SUCCEEDED(hr));

        m_upload_buffer = std::make_unique<adaptors::make_upload_buffer>(device);

        m_resource_state_tracker = std::make_unique<resource_state_tracker>();

        for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        {
            m_dynamic_descriptor_heap[i] = std::make_unique<dynamic_descriptor_heap>(device, static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
            m_descriptor_heaps[i] = nullptr;
        }
    }

    command_list::~command_list()
    {
        log::info("Destroyed a CommandList of type: {0} - Instance nr: {1}", conversions::to_string(m_d3d_command_list_type), instance_nr());
    }

    D3D12_COMMAND_LIST_TYPE command_list::get_command_list_type() const
    {
        return m_d3d_command_list_type;
    }

    device* command_list::get_device() const
    {
        return &m_device;
    }

    wrl::ComPtr<ID3D12GraphicsCommandList2> command_list::get_graphics_command_list() const
    {
        return m_d3d_command_list;
    }

    void command_list::transition_barrier(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource, bool flushBarriers)
    {
        if (resource)
        {
            // The "before" state is not important. It will be resolved by the resource state tracker.
            auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), D3D12_RESOURCE_STATE_COMMON, stateAfter, subresource);
            m_resource_state_tracker->resource_barrier(barrier);

            if (flushBarriers)
            {
                flush_resource_barriers();
            }
        }
    }

    void command_list::transition_barrier(const std::shared_ptr<resource>& resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource, bool flushBarriers)
    {
        if (resource)
        {
            transition_barrier(resource->get_d3d_resource(), stateAfter, subresource, flushBarriers);
        }
    }

    void command_list::flush_resource_barriers()
    {
        m_resource_state_tracker->flush_resource_barriers(shared_from_this());
    }

    void command_list::copy_resource(Microsoft::WRL::ComPtr<ID3D12Resource> dstRes, Microsoft::WRL::ComPtr<ID3D12Resource> srcRes)
    {
        transition_barrier(dstRes, D3D12_RESOURCE_STATE_COPY_DEST);
        transition_barrier(srcRes, D3D12_RESOURCE_STATE_COPY_SOURCE);

        flush_resource_barriers();

        m_d3d_command_list->CopyResource(dstRes.Get(), srcRes.Get());

        track_resource(dstRes);
        track_resource(srcRes);
    }

    void command_list::copy_resource(const std::shared_ptr<resource>& dstRes, const std::shared_ptr<resource>& srcRes)
    {
        assert(dstRes && srcRes);

        copy_resource(dstRes->get_d3d_resource(), srcRes->get_d3d_resource());
    }

    void command_list::resolve_subresource(const std::shared_ptr<resource>& dstRes, const std::shared_ptr<resource>& srcRes, u32 dstSubresource, u32 srcSubresource)
    {
        transition_barrier(dstRes, D3D12_RESOURCE_STATE_RESOLVE_DEST, dstSubresource);
        transition_barrier(srcRes, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, srcSubresource);

        flush_resource_barriers();

        auto f = dstRes->get_d3d_resource_desc().Format;
        m_d3d_command_list->ResolveSubresource(dstRes->get_d3d_resource().Get(), dstSubresource, srcRes->get_d3d_resource().Get(), srcSubresource, f);

        track_resource(srcRes);
        track_resource(dstRes);
    }


    ComPtr<ID3D12Resource> command_list::copy_buffer(size_t bufferSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags)
    {
        ComPtr<ID3D12Resource> d3d_resource;

        if (bufferSize == 0)
        {
            // This will result in a NULL resource (which may be desired to define a default null resource).
        }
        else
        {
            auto d3d_device = m_device.get_d3d_device();

            if (DX_FAILED(d3d_device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags),
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                IID_PPV_ARGS(&d3d_resource))))
            {
                log::error("Failed to CreateCommittedResource");
                return nullptr;
            }

            // Add the resource to the global resource state tracker.
            resource_state_tracker::add_global_resource_state(d3d_resource.Get(), D3D12_RESOURCE_STATE_COMMON);

            if (bufferData != nullptr)
            {
                // Create an upload resource to use as an intermediate buffer to copy the buffer resource 
                ComPtr<ID3D12Resource> upload_resource;

                if (DX_FAILED(d3d_device->CreateCommittedResource(
                    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                    D3D12_HEAP_FLAG_NONE,
                    &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(&upload_resource))))
                {
                    log::error("Failed to CreateComittedResource");
                    return nullptr;
                }

                D3D12_SUBRESOURCE_DATA subresource_data = {};
                subresource_data.pData = bufferData;
                subresource_data.RowPitch = bufferSize;
                subresource_data.SlicePitch = subresource_data.RowPitch;

                m_resource_state_tracker->transition_resource(d3d_resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST);
                flush_resource_barriers();

                UpdateSubresources(m_d3d_command_list.Get(), d3d_resource.Get(), upload_resource.Get(), 0, 0, 1, &subresource_data);

                // Add references to resources so they stay in scope until the command list is reset.
                track_resource(upload_resource);
            }
            track_resource(d3d_resource);
        }

        return d3d_resource;
    }

    std::shared_ptr<vertex_buffer> command_list::copy_vertex_buffer(size_t numVertices, size_t vertexStride, const void* vertexBufferData)
    {
        auto d3d_resource = copy_buffer(numVertices * vertexStride, vertexBufferData);

        std::shared_ptr<vertex_buffer> vertex_buffer = m_device.create_vertex_buffer(d3d_resource, numVertices, vertexStride);

        return vertex_buffer;
    }

    std::shared_ptr<index_buffer> command_list::copy_index_buffer(size_t numIndices, DXGI_FORMAT indexFormat, const void* indexBufferData)
    {
        size_t elementSize = indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4;

        auto d3d_resource = copy_buffer(numIndices * elementSize, indexBufferData);

        std::shared_ptr<index_buffer> index_buffer = m_device.create_index_buffer(d3d_resource, numIndices, indexFormat);

        return index_buffer;
    }

    std::shared_ptr<constant_buffer> command_list::copy_constant_buffer(size_t bufferSize, const void* bufferData)
    {
        auto d3d_resource = copy_buffer(bufferSize, bufferData);

        std::shared_ptr<constant_buffer> constant_buffer = m_device.create_constant_buffer(d3d_resource);

        return constant_buffer;
    }

    std::shared_ptr<byte_address_buffer> command_list::copy_byte_address_buffer(size_t bufferSize, const void* bufferData)
    {
        auto d3d_resource = copy_buffer(bufferSize, bufferData, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

        std::shared_ptr<byte_address_buffer> byte_address_buffer = m_device.create_byte_address_buffer(d3d_resource);

        return byte_address_buffer;
    }

    void command_list::set_primitive_topology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology)
    {
        m_d3d_command_list->IASetPrimitiveTopology(primitiveTopology);
    }

    void command_list::clear_texture(const std::shared_ptr<texture>& texture, const float clearColor[4])
    {
        assert(texture);

        transition_barrier(texture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, true);
        m_d3d_command_list->ClearRenderTargetView(texture->get_render_target_view(), clearColor, 0, nullptr);

        track_resource(texture);
    }

    void command_list::clear_depth_stencil_texture(const std::shared_ptr<texture>& texture, D3D12_CLEAR_FLAGS clearFlags, float depth, uint8_t stencil)
    {
        assert(texture);

        transition_barrier(texture, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, true);
        m_d3d_command_list->ClearDepthStencilView(texture->get_depth_stencil_view(), clearFlags, depth, stencil, 0, nullptr);

        track_resource(texture);
    }

    bool command_list::copy_texture_subresource(const std::shared_ptr<texture>& texture, u32 firstSubresource, u32 numSubresources, D3D12_SUBRESOURCE_DATA* subresourceData)
    {
        assert(texture);

        auto d3d_device = m_device.get_d3d_device();
        auto destination_resource = texture->get_d3d_resource();

        if (destination_resource)
        {
            // Resource must be in the copy-destination state.
            transition_barrier(texture, D3D12_RESOURCE_STATE_COPY_DEST);
            flush_resource_barriers();

            UINT64 required_size = GetRequiredIntermediateSize(destination_resource.Get(), firstSubresource, numSubresources);

            // Create a temporary (intermediate) resource for uploading the subresources
            ComPtr<ID3D12Resource> intermediate_resource;
            if (DX_FAILED(d3d_device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(required_size), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                IID_PPV_ARGS(&intermediate_resource))))
            {
                log::error("Failed to CreateCommittedResource");
                return false;
            }

            UpdateSubresources(m_d3d_command_list.Get(), destination_resource.Get(), intermediate_resource.Get(), 0,
                firstSubresource, numSubresources, subresourceData);

            track_resource(intermediate_resource);
            track_resource(destination_resource);
        }

        return true;
    }

    void command_list::set_graphics_dynamic_constant_buffer(u32 rootParameterIndex, size_t sizeInBytes, const void* bufferData)
    {
        // Constant buffers must be 256-byte aligned.
        auto heap_allocation = m_upload_buffer->allocate(sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        memcpy(heap_allocation.CPU, bufferData, sizeInBytes);

        m_d3d_command_list->SetGraphicsRootConstantBufferView(rootParameterIndex, heap_allocation.GPU);
    }

    void command_list::set_graphics_32_bit_constants(u32 rootParameterIndex, u32 numConstants, const void* constants)
    {
        m_d3d_command_list->SetGraphicsRoot32BitConstants(rootParameterIndex, numConstants, constants, 0);
    }

    void command_list::set_vertex_buffers(u32 startSlot, const std::vector<std::shared_ptr<vertex_buffer>>& vertexBuffers)
    {
        std::vector<D3D12_VERTEX_BUFFER_VIEW> views;
        views.reserve(vertexBuffers.size());

        for (auto vertex_buffer : vertexBuffers)
        {
            if (vertex_buffer)
            {
                transition_barrier(vertex_buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
                track_resource(vertex_buffer);

                views.push_back(vertex_buffer->get_vertex_buffer_view());
            }
        }

        m_d3d_command_list->IASetVertexBuffers(startSlot, views.size(), views.data());
    }

    void command_list::set_vertex_buffer(u32 slot, const std::shared_ptr<vertex_buffer>& vertexBuffer)
    {
        set_vertex_buffers(slot, { vertexBuffer });
    }

    void command_list::set_dynamic_vertex_buffer(u32 slot, size_t numVertices, size_t vertexSize, const void* vertexBufferData)
    {
        size_t bufferSize = numVertices * vertexSize;

        auto heap_allocation = m_upload_buffer->allocate(bufferSize, vertexSize);
        memcpy(heap_allocation.CPU, vertexBufferData, bufferSize);

        D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view = {};
        vertex_buffer_view.BufferLocation = heap_allocation.GPU;
        vertex_buffer_view.SizeInBytes = static_cast<UINT>(bufferSize);
        vertex_buffer_view.StrideInBytes = static_cast<UINT>(vertexSize);

        m_d3d_command_list->IASetVertexBuffers(slot, 1, &vertex_buffer_view);
    }

    void command_list::set_index_buffer(const std::shared_ptr<index_buffer>& indexBuffer)
    {
        if (indexBuffer)
        {
            transition_barrier(indexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
            track_resource(indexBuffer);

            m_d3d_command_list->IASetIndexBuffer(&(indexBuffer->get_index_buffer_view()));
        }
    }

    void command_list::set_dynamic_index_buffer(size_t numIndicies, DXGI_FORMAT indexFormat, const void* indexBufferData)
    {
        size_t index_size_in_bytes = indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4;
        size_t bufferSize = numIndicies * index_size_in_bytes;

        auto heap_allocation = m_upload_buffer->allocate(bufferSize, index_size_in_bytes);
        memcpy(heap_allocation.CPU, indexBufferData, bufferSize);

        D3D12_INDEX_BUFFER_VIEW index_buffer_view = {};
        index_buffer_view.BufferLocation = heap_allocation.GPU;
        index_buffer_view.SizeInBytes = static_cast<UINT>(bufferSize);
        index_buffer_view.Format = indexFormat;

        m_d3d_command_list->IASetIndexBuffer(&index_buffer_view);
    }

    void command_list::set_viewport(const D3D12_VIEWPORT& viewport)
    {
        set_viewports({ viewport });
    }

    void command_list::set_viewports(const std::vector<D3D12_VIEWPORT>& viewports)
    {
        assert(viewports.size() < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
        m_d3d_command_list->RSSetViewports(static_cast<UINT>(viewports.size()), viewports.data());
    }

    void command_list::set_scissor_rect(const D3D12_RECT& scissorRect)
    {
        set_scissor_rects({ scissorRect });
    }

    void command_list::set_scissor_rects(const std::vector<D3D12_RECT>& scissorRects)
    {
        assert(scissorRects.size() < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
        m_d3d_command_list->RSSetScissorRects(static_cast<UINT>(scissorRects.size()), scissorRects.data());
    }

    void command_list::set_pipeline_state(const std::shared_ptr<pipeline_state_object>& pipelineState)
    {
        assert(pipelineState);

        auto d3d_pipeline_state = pipelineState->get_d3d_pipeline_state_object().Get();
        if (m_pipeline_state != d3d_pipeline_state)
        {
            m_pipeline_state = d3d_pipeline_state;

            m_d3d_command_list->SetPipelineState(d3d_pipeline_state);

            track_resource(d3d_pipeline_state);
        }
    }

    void command_list::set_graphics_root_signature(const std::shared_ptr<root_signature>& rootSignature)
    {
        assert(rootSignature);

        auto d3d_root_signature = rootSignature->get_d3d_root_signature().Get();
        if (m_root_signature != d3d_root_signature)
        {
            m_root_signature = d3d_root_signature;

            for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
            {
                m_dynamic_descriptor_heap[i]->parse_root_signature(rootSignature);
            }

            m_d3d_command_list->SetGraphicsRootSignature(m_root_signature);

            track_resource(m_root_signature);
        }
    }

    void command_list::set_constant_buffer_view(u32 rootParameterIndex, const std::shared_ptr<constant_buffer>& buffer, D3D12_RESOURCE_STATES stateAfter, size_t bufferOffset)
    {
        if (buffer)
        {
            auto d3d_resource = buffer->get_d3d_resource();
            transition_barrier(d3d_resource, stateAfter);
            m_dynamic_descriptor_heap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->stage_inline_CBV(rootParameterIndex, d3d_resource->GetGPUVirtualAddress() + bufferOffset);
            track_resource(buffer);
        }
    }

    void command_list::set_constant_buffer_view(u32 rootParameterIndex, u32 descriptorOffset, const std::shared_ptr<constant_buffer_view>& cbv, D3D12_RESOURCE_STATES stateAfter)
    {
        assert(cbv);

        auto constant_buffer = cbv->get_constant_buffer();
        if (constant_buffer)
        {
            transition_barrier(constant_buffer, stateAfter);
            track_resource(constant_buffer);
        }

        m_dynamic_descriptor_heap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->stage_descriptors(rootParameterIndex, descriptorOffset, 1, cbv->get_descriptor_handle());
    }

    void command_list::set_shader_resource_view(u32 rootParameterIndex, const std::shared_ptr<buffer>& buffer, D3D12_RESOURCE_STATES stateAfter, size_t bufferOffset)
    {
        if (buffer)
        {
            auto d3d_resource = buffer->get_d3d_resource();
            transition_barrier(d3d_resource, stateAfter);
            m_dynamic_descriptor_heap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->stage_inline_SRV(rootParameterIndex, d3d_resource->GetGPUVirtualAddress() + bufferOffset);
            track_resource(buffer);
        }
    }

    void command_list::set_shader_resource_view(u32 rootParameterIndex, u32 descriptorOffset, const std::shared_ptr<shader_resource_view>& srv, D3D12_RESOURCE_STATES stateAfter, UINT firstSubresource, UINT numSubresources)
    {
        assert(srv);

        auto resource = srv->get_resource();
        if (resource)
        {
            if (numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
            {
                for (u32 i = 0; i < numSubresources; ++i)
                {
                    transition_barrier(resource, stateAfter, firstSubresource + i);
                }
            }
            else
            {
                transition_barrier(resource, stateAfter);
            }

            track_resource(resource);
        }

        m_dynamic_descriptor_heap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->stage_descriptors(rootParameterIndex, descriptorOffset, 1, srv->get_descriptor_handle());
    }

    void command_list::set_shader_resource_view(int32_t rootParameterIndex, u32 descriptorOffset, const std::shared_ptr<texture>& texture, D3D12_RESOURCE_STATES stateAfter, UINT firstSubresource, UINT numSubresources)
    {
        if (texture)
        {
            if (numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
            {
                for (u32 i = 0; i < numSubresources; ++i)
                {
                    transition_barrier(texture, stateAfter, firstSubresource + i);
                }
            }
            else
            {
                transition_barrier(texture, stateAfter);
            }

            track_resource(texture);

            m_dynamic_descriptor_heap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->stage_descriptors(rootParameterIndex, descriptorOffset, 1, texture->get_shader_resource_view());
        }
    }

    void command_list::set_unordered_access_view(u32 rootParameterIndex, u32 descriptorOffset, const std::shared_ptr<unordered_access_view>& uav, D3D12_RESOURCE_STATES stateAfter, UINT firstSubresource, UINT numSubresources)
    {
        assert(uav);

        auto resource = uav->get_resource();
        if (resource)
        {
            if (numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
            {
                for (u32 i = 0; i < numSubresources; ++i)
                {
                    transition_barrier(resource, stateAfter, firstSubresource + i);
                }
            }
            else
            {
                transition_barrier(resource, stateAfter);
            }

            track_resource(resource);
        }

        m_dynamic_descriptor_heap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->stage_descriptors(rootParameterIndex, descriptorOffset, 1, uav->get_descriptor_handle());
    }

    void command_list::set_unordered_access_view(u32 rootParameterIndex, u32 descriptorOffset, const std::shared_ptr<texture>& texture, UINT mip, D3D12_RESOURCE_STATES stateAfter, UINT firstSubresource, UINT numSubresources)
    {
        if (texture)
        {
            if (numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
            {
                for (u32 i = 0; i < numSubresources; ++i)
                {
                    transition_barrier(texture, stateAfter, firstSubresource + i);
                }
            }
            else
            {
                transition_barrier(texture, stateAfter);
            }

            track_resource(texture);

            m_dynamic_descriptor_heap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->stage_descriptors(rootParameterIndex, descriptorOffset, 1, texture->get_unordered_access_view(mip));
        }
    }

    void command_list::set_unordered_access_view(u32 rootParameterIndex, const std::shared_ptr<buffer>& buffer, D3D12_RESOURCE_STATES stateAfter, size_t bufferOffset)
    {
        if (buffer)
        {
            auto d3d_resource = buffer->get_d3d_resource();
            transition_barrier(d3d_resource, stateAfter);
            m_dynamic_descriptor_heap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->stage_inline_UAV(rootParameterIndex, d3d_resource->GetGPUVirtualAddress() + bufferOffset);
            track_resource(buffer);
        }
    }

    void command_list::set_render_target(const render_target& renderTarget)
    {
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> render_target_descriptors;
        render_target_descriptors.reserve(attachment_point::num_attachment_points);

        const auto& textures = renderTarget.get_textures();

        // Bind color targets (max of 8 render targets can be bound to the rendering pipeline.
        for (int i = 0; i < 8; ++i)
        {
            auto texture = textures[i];

            if (texture)
            {
                transition_barrier(texture, D3D12_RESOURCE_STATE_RENDER_TARGET);
                render_target_descriptors.push_back(texture->get_render_target_view());

                track_resource(texture);
            }
        }

        const auto& depth_texture = renderTarget.get_texture(attachment_point::depth_stencil);

        CD3DX12_CPU_DESCRIPTOR_HANDLE depth_stencil_descriptor(D3D12_DEFAULT);
        if (depth_texture)
        {
            transition_barrier(depth_texture, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            depth_stencil_descriptor = depth_texture->get_depth_stencil_view();

            track_resource(depth_texture);
        }

        D3D12_CPU_DESCRIPTOR_HANDLE* pDSV = depth_stencil_descriptor.ptr != 0 ? &depth_stencil_descriptor : nullptr;

        m_d3d_command_list->OMSetRenderTargets(static_cast<UINT>(render_target_descriptors.size()), render_target_descriptors.data(), FALSE, pDSV);
    }

    void command_list::draw(u32 vertexCount, u32 instanceCount, u32 startVertex, u32 startInstance)
    {
        flush_resource_barriers();

        for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        {
            m_dynamic_descriptor_heap[i]->commit_staged_descriptors_for_draw(*this);
        }

        m_d3d_command_list->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
    }

    void command_list::draw_indexed(u32 indexCount, u32 instanceCount, u32 startIndex, int32_t baseVertex, u32 startInstance)
    {
        flush_resource_barriers();

        for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        {
            m_dynamic_descriptor_heap[i]->commit_staged_descriptors_for_draw(*this);
        }

        m_d3d_command_list->DrawIndexedInstanced(indexCount, instanceCount, startIndex, baseVertex, startInstance);
    }

    bool command_list::close(const std::shared_ptr<command_list>& pendingCommandList)
    {
        // Flush any remaining barriers.
        flush_resource_barriers();

        m_d3d_command_list->Close();

        log::info("Closed CommandList of type: {0} - Instance nr: {1}", conversions::to_string(m_d3d_command_list_type), instance_nr());

        // Flush pending resource barriers.
        u32 num_pending_barriers = m_resource_state_tracker->flush_pending_resource_barriers(pendingCommandList);
        // Commit the final resource state to the global state.
        m_resource_state_tracker->commit_final_resource_states();

        return num_pending_barriers > 0;
    }

    void command_list::close()
    {
        flush_resource_barriers();
        m_d3d_command_list->Close();

        log::info("Closed CommandList of type: {0} - Instance nr: {1}", conversions::to_string(m_d3d_command_list_type), instance_nr());
    }

    bool command_list::reset()
    {
        if (DX_FAILED(m_d3d_command_allocator->Reset()))
        {
            log::error("Failed to reset ID3D12CommandAllocator");
            return false;
        }

        if (DX_FAILED(m_d3d_command_list->Reset(m_d3d_command_allocator.Get(), nullptr)))
        {
            log::error("Failed to reset ID3D12CommandList");
            return false;
        }

        log::info("Reset CommandList of type: {0} - Instance nr: {1}", conversions::to_string(m_d3d_command_list_type), instance_nr());

        m_resource_state_tracker->reset();
        m_upload_buffer->reset();

        release_tracked_objects();

        for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        {
            m_dynamic_descriptor_heap[i]->reset();
            m_descriptor_heaps[i] = nullptr;
        }

        m_root_signature = nullptr;
        m_pipeline_state = nullptr;

        return true;
    }

    void command_list::track_resource(wrl::ComPtr<ID3D12Object> object)
    {
        m_tracked_objects.push_back(object);
    }

    void command_list::track_resource(const std::shared_ptr<resource>& res)
    {
        assert(res);

        track_resource(res->get_d3d_resource());
    }

    void command_list::release_tracked_objects()
    {
        m_tracked_objects.clear();
    }

    void command_list::set_descriptor_heap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap)
    {
        if (m_descriptor_heaps[heapType] != heap)
        {
            m_descriptor_heaps[heapType] = heap;

            bind_descriptor_heaps();
        }
    }

    void command_list::bind_descriptor_heaps()
    {
        UINT num_desc_heaps = 0;
        ID3D12DescriptorHeap* desc_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {};

        for (u32 i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        {
            ID3D12DescriptorHeap* desc_heap = m_descriptor_heaps[i];
            if (desc_heap)
            {
                desc_heaps[num_desc_heaps++] = desc_heap;
            }
        }

        m_d3d_command_list->SetDescriptorHeaps(num_desc_heaps, desc_heaps);
    }
}