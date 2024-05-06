#include "scene.h"
#include "scene_node.h"

namespace cera
{
    scene::scene()
        :m_root_node(nullptr)
    {}

    scene::~scene() = default;

    void scene::set_root_node(const std::shared_ptr<scene_node>& root)
    {
        m_root_node = root;
    }

    const std::shared_ptr<scene_node>& scene::get_root_node() const
    {
        return m_root_node;
    }

    void scene::draw(const std::shared_ptr<command_list>& commandList)
    {
        m_root_node->draw(commandList);
    }
}