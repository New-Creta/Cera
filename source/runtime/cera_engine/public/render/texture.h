#pragma once

#include "util/types.h"

#include "device/windows_declarations.h"
#include "device/windows_types.h"

#include "render/d3dx12_declarations.h"
#include "render/resource.h"
#include "render/descriptor_allocation.h"

namespace cera
{
    class device;

    class texture : public resource
    {
    public:
        /**
        * Resize the texture.
        */
        bool resize(u32 width, u32 height, u32 depthOrArraySize = 1 );

        /**
        * Get the RTV for the texture.
        */
        D3D12_CPU_DESCRIPTOR_HANDLE get_render_target_view() const;

        /**
        * Get the DSV for the texture.
        */
        D3D12_CPU_DESCRIPTOR_HANDLE get_depth_stencil_view() const;

        /**
        * Get the SRV for a resource.
        */
        D3D12_CPU_DESCRIPTOR_HANDLE get_shader_resource_view() const;

        /**
        * Get the UAV for the texture at a specific mip level.
        * Note: Only only supported for 1D and 2D textures.
        */
        D3D12_CPU_DESCRIPTOR_HANDLE get_unordered_access_view(u32 mip) const;

        bool check_SRV_support();
        bool check_RTV_support();
        bool check_UAV_support();
        bool check_DSV_support();

        /**
        * Check to see if the image format has an alpha channel.
        */
        bool has_alpha() const;

        /**
        * Check the number of bits per pixel.
        */
        size_t bits_per_pixel() const;

        static bool is_UAV_compatible_format(DXGI_FORMAT format);
        static bool is_SRGB_format(DXGI_FORMAT format);
        static bool is_BGR_format(DXGI_FORMAT format);
        static bool is_depth_format(DXGI_FORMAT format);

        // Return a typeless format from the given format.
        static DXGI_FORMAT get_typeless_format(DXGI_FORMAT format);
        // Return an sRGB format in the same format family.
        static DXGI_FORMAT get_SRGB_format(DXGI_FORMAT format);
        static DXGI_FORMAT get_UAV_compatable_format(DXGI_FORMAT format);

    protected:
        texture(device& device, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue = nullptr);
        texture(device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue = nullptr);
        ~texture() override;

        /**
        * Create SRV and UAVs for the resource.
        */
        void create_views();

    private:
        descriptor_allocation m_render_target_view;
        descriptor_allocation m_depth_stencil_view;
        descriptor_allocation m_shader_resource_view;
        descriptor_allocation m_unordered_access_view;
    };

}
