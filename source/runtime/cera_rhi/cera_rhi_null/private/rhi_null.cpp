#include "rhi_factory.h"
#include "rhi_null.h"
#include "rhi_globals.h"

#include "common/rhi_feature_level.h"
#include "common/rhi_shader_platform.h"

#include "util/assert.h"

namespace cera
{
    namespace renderer
    {
        namespace rhi_factory
        {
            bool is_supported()
            {
                // null rhi is always supported as no rendering features will be initialized
                return true;
            }

            std::unique_ptr<rhi> create()
            {
                return std::make_unique<rhi_null>();
            }
        } // namespace rhi_factory

        void rhi_null::initialize()
        {
#ifdef CERA_PLATFORM_WINDOWS
            g_shader_platform[feature_level::UNSPECIFIED] = shader_platform::NONE;
#endif
            CERA_ASSERT_X(!g_rhi_initialized, "rhi was already initialized!");

            g_rhi_initialized = true;
        }

        void rhi_null::post_initialize()
        {
            // Nothing to implement
        }

        void rhi_null::shutdown()
        {
            // Nothing to implement
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
    } // namespace renderer
}