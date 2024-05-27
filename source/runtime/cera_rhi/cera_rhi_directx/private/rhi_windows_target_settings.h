#pragma once

#include <string>
#include <unordered_map>

namespace cera
{
    namespace renderer
    {
        extern std::string g_default_graphics_rhi_dx11;
        extern std::string g_default_graphics_rhi_dx12;
        extern std::string g_default_graphics_rhi_opengl;

        enum class shader_platform;

        const std::unordered_map<std::string, std::vector<std::string>>& get_windows_target_settings();

        shader_platform shader_format_to_shader_platform(const std::string& shader_format);
    }
}