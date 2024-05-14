#pragma once

#include "util/types.h"

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
             * Feature level defined by the core capabilities of OpenGL ES3.1 & Metal/Vulkan. 
             */
            ES3_1,

            /**
             * Feature level defined by the capabilities of DX11 Shader Model 5.
             *   Compute shaders with shared memory, group sync, UAV writes, integer atomics
             *   Indirect drawing
             *   Pixel shaders with UAV writes
             *   Cubemap arrays
             *   Read-only depth or stencil views (eg read depth buffer as SRV while depth test and stencil write)
             * Tessellation is not considered part of Feature Level SM5 and has a separate capability flag.
             */
            SM5,

            /**
             * Feature level defined by the capabilities of DirectX 12 hardware feature level 12_2 with Shader Model 6.5
             *   Raytracing Tier 1.1
             *   Mesh and Amplification shaders
             *   Variable rate shading
             *   Sampler feedback
             *   Resource binding tier 3
             */
            SM6,

             /**
              * Unspecified feature level
              * This can be the case when we have no use for rendering capabilities within our program.
              */
            UNSPECIFIED,

            /**
             * Number of feature levels available
             */
            NUM
        };

        static constexpr s32 g_num_feature_levels = static_cast<s32>(feature_level::NUM);
    } // namespace renderer
}