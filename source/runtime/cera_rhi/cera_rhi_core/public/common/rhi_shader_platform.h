#pragma once

#include "util/types.h"

namespace cera
{
    namespace renderer
    {
        enum class shader_platform
        {
            NONE,
            // Supported Direct3D shader models
            D3D_SM5,    // Direct 3D shader model 5 mostly used on DX11
            D3D_SM6,    // Direct 3D shader model 6 mostly used on DX12
            // Supported OpenGL shader models
            OGL_SM5,     // OpenGL shader model 4.3 is equal to DirectX shader model 5
            
            // Number of supported platforms
            NUM
        };

        static constexpr s32 g_num_supported_shader_platform = static_cast<s32>(shader_platform::NUM);
    } // namespace renderer
}