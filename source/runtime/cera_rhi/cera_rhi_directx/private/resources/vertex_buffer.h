#pragma once

#include "util/types.h"
#include "util/windows_types.h"

#include "resources/buffer.h"

namespace cera
{
    namespace renderer
    {
        class VertexBuffer : public Buffer
        {
        public:
            RESOURCE_CLASS_TYPE(VertexBuffer);

            size_t num_vertices() const;

            size_t vertex_stride() const;

            /**
             * Get the vertex buffer view for binding to the Input Assembler stage.
             */
            D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view() const;

        protected:
            VertexBuffer(d3d12_device& device, size_t numVertices, size_t vertexStride);
            VertexBuffer(d3d12_device& device, wrl::com_ptr<ID3D12Resource> resource, size_t numVertices, size_t vertexStride);
            ~VertexBuffer() override;

            void create_vertex_buffer_view();

        private:
            size_t m_num_vertices;
            size_t m_vertex_stride;

            D3D12_VERTEX_BUFFER_VIEW m_vertex_buffer_view;
        };
    }
}