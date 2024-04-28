#include "scene_node.h"
#include "mesh.h"

namespace cera
{
    scene_node::scene_node(const DirectX::XMMATRIX& localTransform)
        : m_name("scene_node")
    {
        m_aligned_data = (aligned_data*)_aligned_malloc(sizeof(aligned_data), 16);
        m_aligned_data->m_local_transform = localTransform;
        m_aligned_data->m_inverse_transform = XMMatrixInverse(nullptr, localTransform);
    }

    scene_node::~scene_node()
    {
        _aligned_free(m_aligned_data);
    }

    void scene_node::draw(const std::shared_ptr<command_list>& commandList)
    {
        for (auto& m : m_meshes)
        {
            m->draw(*commandList);
        }

        for (auto& child : m_children)
        {
            child->draw(commandList);
        }
    }

    const std::string& scene_node::get_name() const
    {
        return m_name;
    }

    void scene_node::set_name(const std::string& name)
    {
        m_name = name;
    }

    DirectX::XMMATRIX scene_node::get_local_transform() const
    {
        return m_aligned_data->m_local_transform;
    }

    void scene_node::set_local_transform(const DirectX::XMMATRIX& localTransform)
    {
        m_aligned_data->m_local_transform = localTransform;
        m_aligned_data->m_inverse_transform = XMMatrixInverse(nullptr, localTransform);
    }

    DirectX::XMMATRIX scene_node::get_inverse_local_transform() const
    {
        return m_aligned_data->m_inverse_transform;
    }

    DirectX::XMMATRIX scene_node::get_world_transform() const
    {
        return m_aligned_data->m_local_transform * get_parent_world_transform();
    }

    DirectX::XMMATRIX scene_node::get_inverse_world_transform() const
    {
        return XMMatrixInverse(nullptr, get_world_transform());
    }

    DirectX::XMMATRIX scene_node::get_parent_world_transform() const
    {
        DirectX::XMMATRIX parent_transform = DirectX::XMMatrixIdentity();
        if (auto parent_node = m_parent_node.lock())
        {
            parent_transform = parent_node->get_world_transform();
        }

        return parent_transform;
    }

    void scene_node::add_child(const std::shared_ptr<scene_node>& childNode)
    {
        if (childNode)
        {
            node_list::iterator iter = std::find(m_children.begin(), m_children.end(), childNode);
            if (iter == m_children.end())
            {
                DirectX::XMMATRIX world_transform = childNode->get_world_transform();
                childNode->m_parent_node = shared_from_this();
                DirectX::XMMATRIX local_transform = world_transform * get_inverse_world_transform();
                childNode->set_local_transform(local_transform);
                m_children.push_back(childNode);
                if (!childNode->get_name().empty())
                {
                    m_children_by_name.emplace(childNode->get_name(), childNode);
                }
            }
        }
    }

    void scene_node::remove_child(const std::shared_ptr<scene_node>& childNode)
    {
        if (childNode)
        {
            node_list::const_iterator iter = std::find(m_children.begin(), m_children.end(), childNode);
            if (iter != m_children.cend())
            {
                childNode->set_parent(nullptr);
                m_children.erase(iter);

                // Also remove it from the name map.
                node_name_map::iterator iter2 = m_children_by_name.find(childNode->get_name());
                if (iter2 != m_children_by_name.end())
                {
                    m_children_by_name.erase(iter2);
                }
            }
            else
            {
                // Maybe the child appears deeper in the scene graph.
                for (auto child : m_children)
                {
                    child->remove_child(childNode);
                }
            }
        }
    }

    void scene_node::set_parent(const std::shared_ptr<scene_node>& parentNode)
    {
        // Parents own their children.. If this node is not owned
        // by anyone else, it will cease to exist if we remove it from it's parent.
        // As a precaution, store myself as a shared pointer so I don't get deleted
        // half-way through this function!
        // Technically self deletion shouldn't occur because the thing invoking this function
        // should have a shared_ptr to it.
        std::shared_ptr<scene_node> me = shared_from_this();
        if (parentNode)
        {
            parentNode->add_child(me);
        }
        else if (auto parent = m_parent_node.lock())
        {
            // Setting parent to NULL.. remove from current parent and reset parent node.
            auto world_transform = get_world_transform();
            parent->remove_child(me);
            m_parent_node.reset();
            set_local_transform(world_transform);
        }
    }

    size_t scene_node::add_mesh(const std::shared_ptr<mesh>& mesh)
    {
        size_t index = (size_t)-1;
        if (mesh)
        {
            mesh_list::const_iterator iter = std::find(m_meshes.begin(), m_meshes.end(), mesh);
            if (iter == m_meshes.cend())
            {
                index = m_meshes.size();
                m_meshes.push_back(mesh);
            }
            else
            {
                index = iter - m_meshes.begin();
            }
        }

        return index;
    }

    void scene_node::remove_mesh(const std::shared_ptr<mesh>& mesh)
    {
        if (mesh)
        {
            mesh_list::const_iterator iter = std::find(m_meshes.begin(), m_meshes.end(), mesh);
            if (iter != m_meshes.end())
            {
                m_meshes.erase(iter);
            }
        }
    }

    std::shared_ptr<mesh> scene_node::get_mesh(size_t pos)
    {
        std::shared_ptr<mesh> mesh = nullptr;

        if (pos < m_meshes.size()) 
        {
            mesh = m_meshes[pos];
        }

        return mesh;
    }
}