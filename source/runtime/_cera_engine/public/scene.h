#pragma once

#include <memory>

namespace cera
{
    class scene_node;
    class command_list;

    class scene
    {
    public:
        scene();
        ~scene();

        void set_root_node(const std::shared_ptr<scene_node>& sceneNode);
        const std::shared_ptr<scene_node>& get_root_node() const;

        void draw(const std::shared_ptr<command_list>& commandList);

    private:
        std::shared_ptr<scene_node> m_root_node;
    };
}