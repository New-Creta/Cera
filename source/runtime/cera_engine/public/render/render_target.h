#pragma once

#include "util/types.h"

#include "render/d3dx12_declarations.h"

#include <vector>

// Don't use scoped enums to avoid the explicit cast required to use these as 
// array indices.
enum attachment_point
{
    color_0,
    color_1,
    color_2,
    color_3,
    color_4,
    color_5,
    color_6,
    color_7,

    depth_stencil,
    
    num_attachment_points
};

namespace cera
{
    class texture;

    class render_target
    {
    public:
        // Create an empty render target.
        render_target();

        render_target( const render_target& copy ) = default;
        render_target( render_target&& copy ) = default;

        render_target& operator=( const render_target& other ) = default;
        render_target& operator=( render_target&& other ) = default;

        /**
         * Attach a texture to a given attachment point.
         *
         * @param attachmentPoint The point to attach the texture to.
         * @param [texture] Optional texture to bind to the render target. Specify nullptr to remove the texture.
         */
        void attach_texture( attachment_point attachmentPoint, const std::shared_ptr<texture>& texture );

        // Resize all of the textures associated with the render target.
        void                resize(DirectX::XMUINT2 size);
        void                resize(u32 width, u32 height);
        DirectX::XMUINT2    get_size() const;
        u32                 get_width() const;
        u32                 get_height() const;

        // Get a viewport for this render target.
        // The scale and bias parameters can be used to specify a split-screen
        // viewport (the bias parameter is normalized in the range [0...1]).
        // By default, a fullscreen viewport is returned.
        D3D12_VIEWPORT get_viewport(DirectX::XMFLOAT2 scale = { 1.0f, 1.0f }, DirectX::XMFLOAT2 bias = { 0.0f, 0.0f }, float minDepth = 0.0f, float maxDepth = 1.0f) const;

        const std::shared_ptr<texture>& get_texture( attachment_point attachmentPoint ) const;
        // Get a list of the textures attached to the render target.
        // This method is primarily used by the CommandList when binding the
        // render target to the output merger stage of the rendering pipeline.
        const std::vector<std::shared_ptr<texture>>& get_textures() const;

        // Get the render target formats of the textures currently 
        // attached to this render target object.
        // This is needed to configure the Pipeline state object.
        D3D12_RT_FORMAT_ARRAY get_render_target_formats() const;

        // Get the format of the attached depth/stencil buffer.
        DXGI_FORMAT get_depth_stencil_format() const;

        // Get the sample description of the render target.
        DXGI_SAMPLE_DESC get_sample_desc() const;

        // Reset all textures
        void reset();

    private:
        using render_target_list = std::vector<std::shared_ptr<texture>>;

        render_target_list m_textures;
        DirectX::XMUINT2 m_size;
    };
}