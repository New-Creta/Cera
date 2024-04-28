#pragma once

#include "imgui.h"

#include "device/windows_declarations.h"

#include <memory>

namespace cera
{
    class device;
    class texture;
    class shader_resource_view;
    class root_signature;
    class pipeline_state_object;
    class render_target;
    class command_list;

    class gui
    {
    public:
        /**
         * Window message handler. This needs to be called by the application to allow ImGui to handle input messages.
         */
        LRESULT wnd_proc_handler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

        /**
         * Begin a new ImGui frame. Do this before calling any ImGui functions that modifies ImGui's render context.
         */
        void new_frame();
        /**
         * Render ImgGui to the given render target.
         */
        void draw(const std::shared_ptr<command_list>& commandList, const render_target& renderTarget);

    protected:
        gui(device& device, void* hwnd, const render_target& renderTarget);
        virtual ~gui();

    private:
        /**
         * Destroy ImGui context.
         */
        void destroy();

    private:
        device&                                 m_device;
        ImGuiContext*                           m_imgui;
        std::shared_ptr<texture>                m_font_texture;
        std::shared_ptr<shader_resource_view>   m_font_srv;
        std::shared_ptr<root_signature>         m_root_signature;
        std::shared_ptr<pipeline_state_object>  m_pipeline_state_object;
    };
}