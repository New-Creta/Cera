#pragma once

#include "device/windows_types.h"

#include "render/buffer.h"
#include "render/d3dx12_declarations.h"

namespace cera
{
    class vertex_buffer : public buffer
    {
    public:
        size_t get_num_vertices() const;

        size_t get_vertex_stride() const;

        /**
         * Get the vertex buffer view for binding to the Input Assembler stage.
         */
        D3D12_VERTEX_BUFFER_VIEW get_vertex_buffer_view() const;

    protected:
        vertex_buffer(device& device, size_t numVertices, size_t vertexStride);
        vertex_buffer(device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numVertices, size_t vertexStride);
        ~vertex_buffer() override;

        void create_vertex_buffer_view();

    private:
        size_t m_num_vertices;
        size_t m_vertex_stride;

        D3D12_VERTEX_BUFFER_VIEW m_vertex_buffer_view;
    };
}