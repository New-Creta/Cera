#pragma once

#include <string>

namespace cera
{
    namespace renderer
    {
        namespace shader_formats
        {
            extern const std::string g_name_d3d_sm5;
            extern const std::string g_name_d3d_sm6;
            extern const std::string g_name_ogl_sm5; // OpenGL shader model 4.3 is equal to DirectX shader model 5
        } // namespace shader_formats
    }
}