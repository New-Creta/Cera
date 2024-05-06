#pragma once

#include "util/types.h"

#include "rhi_resource.h"

namespace cera
{
    struct rhi_texture_desc
    {

    };

    class rhi_texture_desc_builder
    {

    };

    struct rhi_clear_value_desc
    {

    };

    class rhi_texture : public rhi_resource
    {
    public:
        ~rhi_texture() override = default;

        /**
        * Resize the texture.
        */
        virtual bool resize(u32 width, u32 height, u32 depth_or_array_size = 1 ) = 0;

        virtual bool check_SRV_support() = 0;
        virtual bool check_RTV_support() = 0;
        virtual bool check_UAV_support() = 0;
        virtual bool check_DSV_support() = 0;

        /**
        * Check to see if the image format has an alpha channel.
        */
        virtual bool has_alpha() const = 0;

        /**
        * Check the number of bits per pixel.
        */
        virtual size_t bits_per_pixel() const = 0;
    };
}