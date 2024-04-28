#pragma once

#include "entrypoint.h"
#include "window.h"
#include "application.h"
#include "camera.h"

#include "render/d3dx12_declarations.h"
#include "render/render_target.h"

#include "device/windows_types.h"

namespace cera
{
    class root_signature;
    class pipeline_state_object;
    class command_list;
    class scene;

    class demo : public abstract_game
    {
    public:
        demo();

    public:
        bool initialize() override;
        bool load_content() override;

        void on_update(const events::update_args& e) override;
        void on_render(const events::render_args& e) override;
        void on_render_gui(const events::render_gui_args& e) override;
        void on_resize(const events::resize_args& e) override;

        void on_key_pressed(const events::key_args& e) override;
        void on_key_released(const events::key_args& e) override;
        void on_mouse_moved(const events::mouse_motion_args& e) override;

        void unload_content() override;
        void destroy() override;

    private:
        void on_render_scene(const std::shared_ptr<command_list>& commandList);

    private:
        bool m_allow_fullscreen_toggle;

        std::shared_ptr<root_signature> m_root_signature;
        std::shared_ptr<pipeline_state_object> m_pipeline_state_object;

        render_target m_render_target;

        D3D12_VIEWPORT m_viewport;
        D3D12_RECT m_scissor_rect;

        std::shared_ptr<scene> m_cube;
        std::shared_ptr<scene> m_cylinder;
        std::shared_ptr<scene> m_sphere;
        std::shared_ptr<scene> m_plane;

    private:
        camera m_camera;
        struct alignas(16) CameraData
        {
            DirectX::XMVECTOR m_initial_camera_pos;
            DirectX::XMVECTOR m_initial_camera_rot;
        };
        CameraData* m_paligned_camera_data;

        // Camera controller
        float m_forward;
        float m_backward;
        float m_left;
        float m_right;
        float m_up;
        float m_down;

        float m_pitch;
        float m_yaw;
    };
}