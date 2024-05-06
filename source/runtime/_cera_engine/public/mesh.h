#pragma once

#include "util/types.h"

#include "render/d3dx12_declarations.h"

#include <map>
#include <memory>

namespace cera
{
    class vertex_buffer;
    class index_buffer;

    class command_list;

    class mesh
    {
    public:
        using buffer_map = std::map<u32, std::shared_ptr<vertex_buffer>>;

        mesh();
        ~mesh();

        void                                    set_primitive_topology(D3D12_PRIMITIVE_TOPOLOGY primitiveToplogy);
        D3D12_PRIMITIVE_TOPOLOGY                get_primitive_topology() const;

        void                                    set_vertex_buffer(u32 slotID, const std::shared_ptr<vertex_buffer>& vertexBuffer);
        std::shared_ptr<vertex_buffer>          get_vertex_buffer(u32 slotID) const;

        const buffer_map&                       get_vertex_buffers() const;

        void                                    set_index_buffer(const std::shared_ptr<index_buffer>& indexBuffer);
        std::shared_ptr<index_buffer>           get_index_buffer() const;

        /**
         * Get the number if indices in the index buffer.
         * If no index buffer is bound to the mesh, this function returns 0.
         */
        size_t                                  get_index_count() const;

        /**
         * Get the number of vertices in the mesh.
         * If this mesh does not have a vertex buffer, the function returns 0.
         */
        size_t                                  get_vertex_count() const;

        /**
         * Draw the mesh to a CommandList.
         *
         * @param commandList The command list to draw to.
         * @param instanceCount The number of instances to draw.
         * @param startInstance The offset added to the instance ID when reading from the instance buffers.
         */
        void                                    draw(command_list& commandList, u32 instanceCount = 1, u32 startInstance = 0);

    private:
        buffer_map                      m_vertex_buffers;
        std::shared_ptr<index_buffer>   m_index_buffer;

        D3D12_PRIMITIVE_TOPOLOGY        m_primitive_topology;
    };
}