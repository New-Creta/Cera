#pragma once

#include "rhi.h"

namespace cera
{
    namespace dxgi
    {
        class adapter;
    }

    namespace renderer
    {
        class rhi_directx : public rhi
        {
          public:
            rhi_directx(const std::shared_ptr<dxgi::adapter>& adapter);

            void initialize() override;

            void post_initialize() override;

            void shutdown() override;

            std::shared_ptr<rhi_byte_address_buffer> create_byte_address_buffer(size_t buffer_size) override;

            std::shared_ptr<rhi_texture> create_texture(const rhi_texture_desc& desc, const rhi_clear_value_desc& clear_value) override;

            std::shared_ptr<rhi_index_buffer> create_index_buffer(size_t num_indices, rhi_format index_format) override;

            std::shared_ptr<rhi_vertex_buffer> create_vertex_buffer(size_t num_vertices, size_t vertex_stride) override;

            std::shared_ptr<rhi_pipeline_state_object> create_pipeline_state_object(const rhi_pipeline_state_object_desc& pipeline_state_stream) override;

            std::shared_ptr<rhi_shader_resource_view> create_shader_resource_view(const std::shared_ptr<rhi_resource>& in_resource, const rhi_shader_resource_view_desc& srv) override;

            std::shared_ptr<rhi_unordered_address_view> create_unordered_access_view(const std::shared_ptr<rhi_resource>& in_resource, const std::shared_ptr<rhi_resource>& in_counter_resource, const rhi_unordered_access_view_desc& uav) override;

          private:
            const std::shared_ptr<dxgi::adapter> m_adapter;

        };
    } // namespace renderer
} // namespace cera