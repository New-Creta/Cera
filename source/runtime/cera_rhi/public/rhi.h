#pragma once

#include <memory>

namespace cera
{
    class rhi_resource;
    class rhi_byte_address_buffer;
    class rhi_texture;
    class rhi_index_buffer;
    class rhi_vertex_buffer;
    class rhi_pipeline_state_object;
    class rhi_constant_buffer_view;
    class rhi_shader_resource_view;
    class rhi_unordered_address_view;

    struct rhi_texture_desc;
    struct rhi_clear_value_desc;
    struct rhi_shader_resource_view_desc;
    struct rhi_unordered_access_view_desc;

    enum class rhi_format;

    class rhi
    {
    public:
        /**
         * Create a ByteAddressBuffer resource.
         *
         * @param resDesc A description of the resource.
         */
        std::shared_ptr<rhi_byte_address_buffer> create_byte_address_buffer(size_t bufferSize);

        /**
         * Create a Texture resource.
         *
         * @param resourceDesc A description of the texture to create.
         * @param [clearVlue] Optional optimized clear value for the texture.
         * @param [textureUsage] Optional texture usage flag provides a hint about how the texture will be used.
         *
         * @returns A pointer to the created texture.
         */
        std::shared_ptr<rhi_texture> create_texture(const rhi_texture_desc& desc, const rhi_clear_value_desc& clear_value);

        std::shared_ptr<rhi_index_buffer> create_index_buffer(size_t num_indices, rhi_format index_format);
        std::shared_ptr<rhi_vertex_buffer> create_vertex_buffer(size_t num_vertices, size_t vertex_stride);

        template<class pipeline_state_stream>
        std::shared_ptr<rhi_pipeline_state_object> create_pipeline_state_object(pipeline_state_stream& pipeline_state_stream);

        std::shared_ptr<rhi_shader_resource_view> create_shader_resource_view(const std::shared_ptr<rhi_resource>& in_resource, const rhi_shader_resource_view_desc& srv);
        std::shared_ptr<rhi_unordered_address_view> create_unordered_access_view(const std::shared_ptr<rhi_resource>& in_resource, const std::shared_ptr<rhi_resource>& in_counter_resource = nullptr, const rhi_unordered_access_view_desc& uav);

    };
}