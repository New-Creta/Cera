#include "rhi_factory.h"
#include "rhi_null.h"

namespace cera
{
    namespace rhi_factory
    {
        std::unique_ptr<rhi> create()
        {
            return std::make_unique<rhi_null>();
        }
    }

    void rhi_null::initialize()
    {

    }

    void rhi_null::post_initialize()
    {

    }

    void rhi_null::shutdown()
    {

    }

    std::shared_ptr<rhi_byte_address_buffer> rhi_null::create_byte_address_buffer(size_t /*buffer_size*/)
    {
        return nullptr;
    }
    std::shared_ptr<rhi_texture> rhi_null::create_texture(const rhi_texture_desc& /*desc*/, const rhi_clear_value_desc& /*clear_value*/)
    {
        return nullptr;
    }
    std::shared_ptr<rhi_index_buffer> rhi_null::create_index_buffer(size_t /*num_indices*/, rhi_format /*index_format*/)
    {
        return nullptr;
    }
    std::shared_ptr<rhi_vertex_buffer> rhi_null::create_vertex_buffer(size_t /*num_vertices*/, size_t /*vertex_stride*/)
    {
        return nullptr;
    }
    std::shared_ptr<rhi_pipeline_state_object> rhi_null::create_pipeline_state_object(const rhi_pipeline_state_object_desc& /*pipeline_state_stream*/)
    {
        return nullptr;
    }
    std::shared_ptr<rhi_shader_resource_view> rhi_null::create_shader_resource_view(const std::shared_ptr<rhi_resource>& /*in_resource*/, const rhi_shader_resource_view_desc& /*srv*/)
    {
        return nullptr;
    }
    std::shared_ptr<rhi_unordered_address_view> rhi_null::create_unordered_access_view(const std::shared_ptr<rhi_resource>& /*in_resource*/, const std::shared_ptr<rhi_resource>& /*in_counter_resource*/, const rhi_unordered_access_view_desc& /*uav*/)
    {
        return nullptr;
    }
}