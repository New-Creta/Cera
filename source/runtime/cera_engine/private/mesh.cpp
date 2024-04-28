#include "mesh.h"

#include "render/index_buffer.h"
#include "render/vertex_buffer.h"
#include "render/command_list.h"

namespace cera
{
    mesh::mesh()
        :m_primitive_topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
    {}

    mesh::~mesh() = default;

    void mesh::set_primitive_topology(D3D12_PRIMITIVE_TOPOLOGY primitiveToplogy)
    {
        m_primitive_topology = primitiveToplogy;
    }

    D3D12_PRIMITIVE_TOPOLOGY mesh::get_primitive_topology() const
    {
        return m_primitive_topology;
    }

    void mesh::set_vertex_buffer(u32 slotID, const std::shared_ptr<vertex_buffer>& vertexBuffer)
    {
        m_vertex_buffers[slotID] = vertexBuffer;
    }

    std::shared_ptr<vertex_buffer> mesh::get_vertex_buffer(u32 slotID) const
    {
        auto itr = m_vertex_buffers.find(slotID);
        auto vertex_buffer = itr != m_vertex_buffers.end() ? itr->second : nullptr;

        return vertex_buffer;
    }

    const mesh::buffer_map& mesh::get_vertex_buffers() const
    {
        return m_vertex_buffers;
    }

    void mesh::set_index_buffer(const std::shared_ptr<index_buffer>& indexBuffer)
    {
        m_index_buffer = indexBuffer;
    }

    std::shared_ptr<index_buffer> mesh::get_index_buffer() const
    {
        return m_index_buffer;
    }

    size_t mesh::get_index_count() const
    {
        size_t index_count = 0;
        if (m_index_buffer)
        {
            index_count = m_index_buffer->get_num_indices();
        }
        return index_count;
    }

    size_t mesh::get_vertex_count() const
    {
        size_t vertex_count = 0;

        // To count the number of vertices in the mesh, just take the number of vertices in the first vertex buffer.
        buffer_map::const_iterator itr = m_vertex_buffers.cbegin();
        if (itr != m_vertex_buffers.cend())
        {
            vertex_count = itr->second->get_num_vertices();
        }

        return vertex_count;
    }

    void mesh::draw(command_list& commandList, u32 instanceCount, u32 startInstance)
    {
        commandList.set_primitive_topology(get_primitive_topology());

        for (auto vertexBuffer : m_vertex_buffers)
        {
            commandList.set_vertex_buffer(vertexBuffer.first, vertexBuffer.second);
        }

        auto indexCount = get_index_count();
        auto vertexCount = get_vertex_count();

        if (indexCount > 0)
        {
            commandList.set_index_buffer(m_index_buffer);
            commandList.draw_indexed(indexCount, instanceCount, 0u, 0u, startInstance);
        }
        else if (vertexCount > 0)
        {
            commandList.draw(vertexCount, instanceCount, 0u, startInstance);
        }
    }
}