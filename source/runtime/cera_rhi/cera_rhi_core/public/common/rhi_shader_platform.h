#pragma once

namespace cera
{
    namespace renderer
    {
        enum class shader_platform
        {
            NONE,
            // Supported Direct3D shader models
            D3D_SM5,
            D3D_SM6,
            // Supported OpenGL shader models
            GL_SM4_3,
            // Supported OpenGL ES shader models
            GLES_3_1,
            GLES_3_2
        };
    } // namespace renderer
}