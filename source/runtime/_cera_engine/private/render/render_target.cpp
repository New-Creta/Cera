#include "render/render_target.h"
#include "render/texture.h"

namespace cera
{
    render_target::render_target()
        : m_textures(attachment_point::num_attachment_points)
        , m_size(0, 0)
    {}

    // Attach a texture to the render target.
    // The texture will be copied into the texture array.
    void render_target::attach_texture( attachment_point attachmentPoint, const std::shared_ptr<texture>& texture )
    {
        m_textures[attachmentPoint] = texture;

        if (texture && texture->get_d3d_resource())
        {
            auto desc = texture->get_d3d_resource_desc();

            m_size.x = static_cast<uint32_t>(desc.Width);
            m_size.y = static_cast<uint32_t>(desc.Height);
        }
    }

    // Resize all of the textures associated with the render target.
    void render_target::resize(DirectX::XMUINT2 size)
    {
        m_size = size;
        for (auto texture : m_textures) 
        { 
            if (texture)
            {
                texture->resize(m_size.x, m_size.y);
            }
        }
    }

    void render_target::resize(uint32_t width, uint32_t height)
    {
        resize(DirectX::XMUINT2(width, height));
    }

    DirectX::XMUINT2 render_target::get_size() const
    {
        return m_size;
    }

    uint32_t render_target::get_width() const
    {
        return m_size.x;
    }

    uint32_t render_target::get_height() const
    {
        return m_size.y;
    }

    D3D12_VIEWPORT render_target::get_viewport(DirectX::XMFLOAT2 scale, DirectX::XMFLOAT2 bias, float minDepth, float maxDepth) const
    {
        UINT64 width = 0;
        UINT   height = 0;

        for (int i = attachment_point::color_0; i <= attachment_point::color_7; ++i)
        {
            auto texture = m_textures[i];
            if (texture)
            {
                auto desc = texture->get_d3d_resource_desc();
                width = std::max(width, desc.Width);
                height = std::max(height, desc.Height);
            }
        }

        D3D12_VIEWPORT viewport = 
        {
            (width * bias.x),    // Top Left X
            (height * bias.y),   // Top Left Y
            (width * scale.x),   // Width
            (height * scale.y),  // Height
            minDepth,            // Min Depth
            maxDepth             // Max Depth
        };

        return viewport;
    }

    const std::shared_ptr<texture>& render_target::get_texture( attachment_point attachmentPoint ) const
    {
        return m_textures[attachmentPoint];
    }

    // Get a list of the textures attached to the render target.
    // This method is primarily used by the command_list when binding the
    // render target to the output merger stage of the rendering pipeline.
    const std::vector<std::shared_ptr<texture>>& render_target::get_textures() const
    {
        return m_textures;
    }

    D3D12_RT_FORMAT_ARRAY render_target::get_render_target_formats() const
    {
        D3D12_RT_FORMAT_ARRAY rtv_formats = {};

        for ( int i = attachment_point::color_0; i <= attachment_point::color_7; ++i )
        {
            auto texture = m_textures[i];
            if ( texture )
            {
                rtv_formats.RTFormats[rtv_formats.NumRenderTargets++] = texture->get_d3d_resource_desc().Format;
            }
        }

        return rtv_formats;
    }

    DXGI_FORMAT render_target::get_depth_stencil_format() const
    {
        DXGI_FORMAT dsv_format = DXGI_FORMAT_UNKNOWN;

        auto depth_stencil_texture = m_textures[attachment_point::depth_stencil];
        if ( depth_stencil_texture )
        {
            dsv_format = depth_stencil_texture->get_d3d_resource_desc().Format;
        }

        return dsv_format;
    }

    DXGI_SAMPLE_DESC render_target::get_sample_desc() const
    {
        DXGI_SAMPLE_DESC sample_desc = { 1, 0 };
        for (int i = attachment_point::color_0; i <= attachment_point::color_7; ++i)
        {
            auto texture = m_textures[i];
            if (texture)
            {
                sample_desc = texture->get_d3d_resource_desc().SampleDesc;
                break;
            }
        }

        return sample_desc;
    }

    void render_target::reset()
    {
        m_textures = render_target_list(attachment_point::num_attachment_points);
    }
}