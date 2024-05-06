#pragma once

#include "render/d3dx12_declarations.h"

#include <memory>
#include <map>
#include <vector>
#include <string>

namespace cera
{
    class mesh;
    class command_list;

    class scene_node : public std::enable_shared_from_this<scene_node>
    {
    public:
        explicit scene_node(const DirectX::XMMATRIX& localTransform = DirectX::XMMatrixIdentity());
        virtual ~scene_node();

        void draw(const std::shared_ptr<command_list>& commandList);

        /**
         * Assign a name to the scene node so it can be searched for later.
         */
        const std::string& get_name() const;
        void               set_name(const std::string& name);

        /**
         * Get the scene nodes local (relative to its parent's transform).
         */
        DirectX::XMMATRIX get_local_transform() const;
        void              set_local_transform(const DirectX::XMMATRIX& localTransform);

        /**
         * Get the inverse of the local transform.
         */
        DirectX::XMMATRIX get_inverse_local_transform() const;

        /**
         * Get the scene node's world transform (concatenated with its parents
         * world transform).
         */
        DirectX::XMMATRIX get_world_transform() const;

        /**
         * Get the inverse of the world transform (concatenated with its parent's
         * world transform).
         */
        DirectX::XMMATRIX get_inverse_world_transform() const;

        /**
         * Add a child node to this scene node.
         * NOTE: Circular references are not checked.
         * A scene node "owns" its children. If the root node
         * is deleted, all of its children are deleted if nothing
         * else is referencing them.
         */
        void add_child(const std::shared_ptr<scene_node>& childNode);
        void remove_child(const std::shared_ptr<scene_node>& childNode);
        void set_parent(const std::shared_ptr<scene_node>& parentNode);

        /**
         * Add a mesh to this scene node.
         * @returns The index of the mesh in the mesh list.
         */
        size_t add_mesh(const std::shared_ptr<mesh>& mesh);
        void remove_mesh(const std::shared_ptr<mesh>& mesh);

        /**
         * Get a mesh in the list of meshes for this node.
         */
        std::shared_ptr<mesh> get_mesh(size_t index = 0);

    private:
        DirectX::XMMATRIX get_parent_world_transform() const;

    private:
        using node_ptr      = std::shared_ptr<scene_node>;
        using node_list     = std::vector<node_ptr>;
        using node_name_map = std::multimap<std::string, node_ptr>;
        using mesh_list     = std::vector<std::shared_ptr<mesh>>;

        std::string m_name;

        // This data must be aligned to a 16-byte boundary.
        // The only way to guarantee this, is to allocate this
        // structure in aligned memory.
        struct alignas(16) aligned_data
        {
            DirectX::XMMATRIX m_local_transform;
            DirectX::XMMATRIX m_inverse_transform;
        } * m_aligned_data;

        std::weak_ptr<scene_node> m_parent_node;

        node_list                 m_children;
        node_name_map             m_children_by_name;

        mesh_list                 m_meshes;
    };
}