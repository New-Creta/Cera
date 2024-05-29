#pragma once

#include "util/types.h"

#include <string>

namespace cera
{
    namespace renderer
    {
        /**
         * The RHI's feature level indicates what level of support can be relied upon.
         *
         * Note: these are named after graphics API's like ES3, but a feature level can be used with a different API (eg feature_level::ES3.1 on D3D11)
         * As long as the graphics API supports all the features of the feature level (eg no ERHIFeatureLevel::SM5 on OpenGL ES3.1)
         */
        enum class feature_level
        {
            /**
             * Unspecified feature level
             * This can be the case when we have no use for rendering capabilities within our program.
             */
            UNSPECIFIED,

            /**
             * Feature level defined by the capabilities of DX11 Shader Model 5.
             *   Compute shaders with shared memory, group sync, UAV writes, integer atomics
             *   Indirect drawing
             *   Pixel shaders with UAV writes
             *   Cubemap arrays
             *   Read-only depth or stencil views (eg read depth buffer as SRV while depth test and stencil write)
             * Tessellation is not considered part of Feature Level SM5 and has a separate capability flag.
             */
            D3D_SM5,

            /**
             * Feature level defined by the capabilities of DirectX 12 hardware feature level 12_2 with Shader Model 6.5
             *   Raytracing Tier 1.1
             *   Mesh and Amplification shaders
             *   Variable rate shading
             *   Sampler feedback
             *   resource binding tier 3
             */
            D3D_SM6,

            /** 
             * Feature level defined by the core capabilities of OpenGL 4.3
             * OpenGL shader model 4.3 is equal to DirectX shader model 5
             */
            OGL_SM5,

            /**
             * Number of feature levels available
             */
            NUM
        };

        static constexpr s32 g_num_feature_levels = static_cast<s32>(feature_level::NUM);

        namespace conversions
        {
            inline const std::string to_string(feature_level in_feature_level)
            {
                switch (in_feature_level)
                {
                case feature_level::OGL_SM5:
                    return "OpenGL Shader Model 4.3";
                case feature_level::D3D_SM5:
                    return "Direct 3D Shader Model 5";
                case feature_level::D3D_SM6:
                    return "Direct 3D Shader Model 6";
                default:
                    return "<unknown>";
                }
            }
        }
    } // namespace renderer
}