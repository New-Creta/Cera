#include "demo.h"
#include "scene.h"

#include "render/device.h"
#include "render/command_queue.h"
#include "render/d3dx12_call.h"
#include "render/swapchain.h"
#include "render/root_signature.h"
#include "render/pipeline_state_object.h"
#include "render/texture.h"
#include "render/command_list.h"
#include "render/command_queue.h"
#include "render/vertex_types.h"
#include "render/mesh_factory.h"

#include "util/log.h"

#include "imgui.h"

#include <wrl.h>
#include <algorithm>

#if defined(min)
#undef min
#endif
#if defined(max)
#undef max
#endif

namespace cera
{
    // An enum for root signature parameters.
    // I'm not using scoped enums to avoid the explicit cast that would be required
    // to use these as root indices in the root signature.
    enum root_parameters
    {
        matrices_cb,         // ConstantBuffer<Mat> MatCB : register(b0);
        num_root_parameters
    };

    app_creation_params entry()
    {
        app_creation_params params;

        params.game = std::make_unique<demo>();
        params.window_width = 1600;
        params.window_height = 900;
        params.use_warp = false;

        return params;
    }

    demo::demo()
        :m_allow_fullscreen_toggle(true)
        ,m_scissor_rect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
        ,m_viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, 0.0f, 0.0f))
        ,m_forward(0)
        ,m_backward(0)
        ,m_left(0)
        ,m_right(0)
        ,m_up(0)
        ,m_down(0)
        ,m_pitch(0)
        ,m_yaw(0)
    {
        DirectX::XMVECTOR camera_pos = DirectX::XMVectorSet(0, 5, -25, 1);
        DirectX::XMVECTOR camera_target = DirectX::XMVectorSet(0, 5, 0, 1);
        DirectX::XMVECTOR camera_up = DirectX::XMVectorSet(0, 1, 0, 0);

        m_camera.set_LookAt(camera_pos, camera_target, camera_up);

        m_paligned_camera_data = (CameraData*)_aligned_malloc(sizeof(CameraData), 16);

        m_paligned_camera_data->m_initial_camera_pos = m_camera.get_Translation();
        m_paligned_camera_data->m_initial_camera_rot = m_camera.get_Rotation();
    }

    bool demo::initialize()
    {
        s32 client_width = application::get()->get_window()->get_client_width();
        s32 client_height = application::get()->get_window()->get_client_height();

        m_scissor_rect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
        m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(client_width), static_cast<float>(client_height));

        return true;
    }

    bool demo::load_content()
    {
        s32 client_width = application::get()->get_window()->get_client_width();
        s32 client_height = application::get()->get_window()->get_client_height();

        auto& device = application::get()->get_device();

        auto& command_queue = device->get_command_queue(D3D12_COMMAND_LIST_TYPE_COPY);
        auto  command_list = command_queue.get_command_list();

        m_cube = mesh_factory::create_cube(command_list, DirectX::XMFLOAT3(0.85f, 0.0f, 0.0f));
        m_sphere = mesh_factory::create_sphere(command_list, DirectX::XMFLOAT3(0.0f, 0.85f, 0.0f));
        m_cylinder = mesh_factory::create_cylinder(command_list, DirectX::XMFLOAT3(0.0f, 0.0f, 0.85f));
        m_plane = mesh_factory::create_plane(command_list, DirectX::XMFLOAT3(0.85f, 0.85f, 0.85f));

        command_queue.execute_command_list(command_list);

        // Load the vertex shader.
        Microsoft::WRL::ComPtr<ID3DBlob> vertex_shader_blob;
        if (DX_FAILED(D3DReadFileToBlob(L"VertexShader.cso", &vertex_shader_blob)))
        {
            log::error("Failed to read compiled vertex shader from file.");
            return false;
        }

        // Load the pixel shader.
        Microsoft::WRL::ComPtr<ID3DBlob> pixel_shader_blob;
        if (DX_FAILED(D3DReadFileToBlob(L"PixelShader.cso", &pixel_shader_blob)))
        {
            log::error("Failed to read compiled pixel shader from file.");
            return false;
        }

        // Create a root signature.
        // Allow input layout and deny unnecessary access to certain pipeline stages.
        D3D12_ROOT_SIGNATURE_FLAGS root_signature_flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        CD3DX12_ROOT_PARAMETER1 root_parameters[root_parameters::num_root_parameters];
        root_parameters[root_parameters::matrices_cb].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC root_signature_description;
        root_signature_description.Init_1_1(root_parameters::num_root_parameters, root_parameters, 0, nullptr, root_signature_flags);

        m_root_signature = device->create_root_signature(root_signature_description.Desc_1_1);
            
        // Setup the pipeline state.
        struct pipeline_state_stream
        {
            CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        p_root_signature;
            CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          input_layout;
            CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    primitive_topology_type;
            CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
            CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
            CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSV_format;
            CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTV_formats;
            CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           sample_desc;
        } pipeline_state_stream;

        // Create a color buffer with sRGB for gamma correction.
        DXGI_FORMAT back_buffer_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        DXGI_FORMAT depth_buffer_format = DXGI_FORMAT_D32_FLOAT;

        // Check the best multisample quality level that can be used for the given back buffer format.
        DXGI_SAMPLE_DESC sample_desc = device->get_multisample_quality_levels(back_buffer_format);

        D3D12_RT_FORMAT_ARRAY rtv_formats = {};
        rtv_formats.NumRenderTargets = 1;
        rtv_formats.RTFormats[0] = back_buffer_format;

        pipeline_state_stream.p_root_signature = m_root_signature->get_d3d_root_signature().Get();
        pipeline_state_stream.input_layout = vertex_pos_color::input_layout;
        pipeline_state_stream.primitive_topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pipeline_state_stream.VS = CD3DX12_SHADER_BYTECODE(vertex_shader_blob.Get());
        pipeline_state_stream.PS = CD3DX12_SHADER_BYTECODE(pixel_shader_blob.Get());
        pipeline_state_stream.DSV_format = depth_buffer_format;
        pipeline_state_stream.RTV_formats = rtv_formats;
        pipeline_state_stream.sample_desc = sample_desc;

        m_pipeline_state_object = device->create_pipeline_state_object(pipeline_state_stream);

        // Create an off-screen render target with a single color buffer and a depth buffer.
        auto color_desc = CD3DX12_RESOURCE_DESC::Tex2D(back_buffer_format, client_width, client_height, 1, 1, sample_desc.Count, sample_desc.Quality, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

        D3D12_CLEAR_VALUE color_clear_value;
        color_clear_value.Format = color_desc.Format;
        color_clear_value.Color[0] = 0.4f;
        color_clear_value.Color[1] = 0.6f;
        color_clear_value.Color[2] = 0.9f;
        color_clear_value.Color[3] = 1.0f;

        auto color_texture = device->create_texture(color_desc, &color_clear_value);
        color_texture->set_resource_name(L"Color Render Target");

        // Create a depth buffer.
        auto depth_desc = CD3DX12_RESOURCE_DESC::Tex2D(depth_buffer_format, client_width, client_height, 1, 1, sample_desc.Count, sample_desc.Quality, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
        D3D12_CLEAR_VALUE depth_clear_value;
        depth_clear_value.Format = depth_desc.Format;
        depth_clear_value.DepthStencil = { 1.0f, 0 };

        auto depth_texture = device->create_texture(depth_desc, &depth_clear_value);
        depth_texture->set_resource_name(L"Depth Render Target");

        // Attach the textures to the render target.
        m_render_target.attach_texture(attachment_point::color_0, color_texture);
        m_render_target.attach_texture(attachment_point::depth_stencil, depth_texture);

        command_queue.flush();  // Wait for loading operations to complete before rendering the first frame.

        return true;
    }

    void demo::on_update(const events::update_args& e)
    {
        // Update the camera.
        float speed_multipler = 4.0f;

        DirectX::XMVECTOR cameraTranslate = DirectX::operator*(speed_multipler * static_cast<float>(e.elapsed_time), DirectX::XMVectorSet(m_right - m_left, 0.0f, m_forward - m_backward, 1.0f));
        DirectX::XMVECTOR cameraPan = DirectX::operator*(speed_multipler * static_cast<float>(e.elapsed_time), DirectX::XMVectorSet(0.0f, m_up - m_down, 0.0f, 1.0f));
        m_camera.Translate(cameraTranslate, Space::Local);
        m_camera.Translate(cameraPan, Space::Local);

        DirectX::XMVECTOR cameraRotation = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(m_pitch), DirectX::XMConvertToRadians(m_yaw), 0.0f);
        m_camera.set_Rotation(cameraRotation);

        DirectX::XMMATRIX viewMatrix = m_camera.get_ViewMatrix();
    }

    void demo::on_render(const events::render_args& e)
    {
        // Clear the render targets.
        {
            FLOAT clear_color[] = { 0.4f, 0.6f, 0.9f, 1.0f };

            e.command_list->clear_texture(m_render_target.get_texture(attachment_point::color_0), clear_color);
            e.command_list->clear_depth_stencil_texture(m_render_target.get_texture(attachment_point::depth_stencil), D3D12_CLEAR_FLAG_DEPTH);
        }

        e.command_list->set_pipeline_state(m_pipeline_state_object);
        e.command_list->set_graphics_root_signature(m_root_signature);

        e.command_list->set_viewport(m_viewport);
        e.command_list->set_scissor_rect(m_scissor_rect);

        e.command_list->set_render_target(m_render_target);

        on_render_scene(e.command_list);

        // Resolve the MSAA render target to the swapchain's backbuffer.
        auto  swapchain = application::get()->get_swapchain();
        auto& swapchain_RT = swapchain->get_render_target();
        auto  swapchain_back_buffer = swapchain_RT.get_texture(attachment_point::color_0);
        auto  msaa_render_target = m_render_target.get_texture(attachment_point::color_0);

        e.command_list->resolve_subresource(swapchain_back_buffer, msaa_render_target);
    }

    void demo::on_render_gui(const events::render_gui_args& e)
    {
        static bool show_demo_window = true;

        if (show_demo_window)
        {
            ImGui::ShowDemoWindow(&show_demo_window);
        }
    }

    void demo::on_resize(const events::resize_args& e)
    {
        m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(e.width), static_cast<float>(e.height));        
        m_camera.set_Projection(55.0f, e.width / (float)e.height, 0.1f, 100.0f);
        m_render_target.resize(e.width, e.height);
    }

    void demo::on_key_pressed(const events::key_args& e)
    {
        switch (e.key)
        {
        case key_code::Escape:
            application::get()->quit();
            break;
        case key_code::Enter:
            if (e.alt)
            {
        case key_code::F11:
            if (m_allow_fullscreen_toggle)
            {
                application::get()->get_window()->toggle_fullscreen();
                m_allow_fullscreen_toggle = false;
            }
            break;
            }
        case key_code::V:
            application::get()->get_swapchain()->toggle_v_sync();
            break;
        case key_code::R:
            // Reset camera transform
            m_camera.set_Translation(m_paligned_camera_data->m_initial_camera_pos);
            m_camera.set_Rotation(m_paligned_camera_data->m_initial_camera_rot);
            m_pitch = 0.0f;
            m_yaw = 0.0f;
            break;
        case key_code::Up:
        case key_code::W:
            m_forward = 1.0f;
            break;
        case key_code::Left:
        case key_code::A:
            m_left = 1.0f;
            break;
        case key_code::Down:
        case key_code::S:
            m_backward = 1.0f;
            break;
        case key_code::Right:
        case key_code::D:
            m_right = 1.0f;
            break;
        case key_code::Q:
            m_down = 1.0f;
            break;
        case key_code::E:
            m_up = 1.0f;
            break;
        }
    }

    void demo::on_key_released(const events::key_args& e)
    {
        switch (e.key)
        {
        case key_code::Enter:
            if (e.alt)
            {
        case key_code::F11:
                m_allow_fullscreen_toggle = true;
            }
            break;
        case key_code::Up:
        case key_code::W:
            m_forward = 0.0f;
            break;
        case key_code::Left:
        case key_code::A:
            m_left = 0.0f;
            break;
        case key_code::Down:
        case key_code::S:
            m_backward = 0.0f;
            break;
        case key_code::Right:
        case key_code::D:
            m_right = 0.0f;
            break;
        case key_code::Q:
            m_down = 0.0f;
            break;
        case key_code::E:
            m_up = 0.0f;
            break;
        }
    }

    void demo::on_mouse_moved(const events::mouse_motion_args& e)
    {
        const float mouse_speed = 0.1f;

        if (!ImGui::GetIO().WantCaptureMouse)
        {
            if (e.left_button)
            {
                m_pitch += e.rel_y * mouse_speed;
                m_pitch = std::clamp(m_pitch, -90.0f, 90.0f);

                m_yaw += e.rel_x * mouse_speed;
            }
        }
    }

    void demo::unload_content()
    {
        m_cube.reset();
        m_sphere.reset();
        m_cylinder.reset();
        m_plane.reset();

        m_render_target.reset();

        m_root_signature.reset();
        m_pipeline_state_object.reset();
    }

    void demo::destroy()
    {
        // Nothing to implement
    }

    void demo::on_render_scene(const std::shared_ptr<command_list>& commandList)
    {
        s32 client_width = application::get()->get_window()->get_client_width();
        s32 client_height = application::get()->get_window()->get_client_height();

        // Update the view matrix.
        const DirectX::XMVECTOR eye_position = DirectX::XMVectorSet(0, 0, -10, 1);
        const DirectX::XMVECTOR focus_point = DirectX::XMVectorSet(0, 0, 0, 1);
        const DirectX::XMVECTOR up_direction = DirectX::XMVectorSet(0, 1, 0, 0);
        const DirectX::XMMATRIX view_matrix = m_camera.get_ViewMatrix();
        const DirectX::XMMATRIX view_projection_matrix = view_matrix * m_camera.get_ProjectionMatrix();

        // Draw cube
        // 
        // Calculate MVP
        DirectX::XMMATRIX translation_matrix = DirectX::XMMatrixTranslation(0.0f, 1.0f, 0.0f);
        DirectX::XMMATRIX rotation_matrix = DirectX::XMMatrixIdentity();
        DirectX::XMMATRIX scale_matrix = DirectX::XMMatrixScaling(2.0f, 2.0f, 2.0f);
        DirectX::XMMATRIX world_matrix = scale_matrix * rotation_matrix * translation_matrix;
        DirectX::XMMATRIX world_view_projection_matrix = DirectX::XMMatrixMultiply(world_matrix, view_projection_matrix);

        commandList->set_graphics_dynamic_constant_buffer(root_parameters::matrices_cb, world_view_projection_matrix);

        m_cube->draw(commandList);

        // Draw plane
        // 
        // Calculate MVP
        translation_matrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
        rotation_matrix = DirectX::XMMatrixIdentity();
        scale_matrix = DirectX::XMMatrixScaling(15.0f, 1.0f, 25.0f);
        world_matrix = scale_matrix * rotation_matrix * translation_matrix;
        world_view_projection_matrix = DirectX::XMMatrixMultiply(world_matrix, view_projection_matrix);

        commandList->set_graphics_dynamic_constant_buffer(root_parameters::matrices_cb, world_view_projection_matrix);

        m_plane->draw(commandList);

        for (int i = 0; i < 5; ++i)
        {
            DirectX::XMMATRIX left_cyl_world = DirectX::XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f);
            // Calculate MVP
            rotation_matrix = DirectX::XMMatrixIdentity();
            scale_matrix = DirectX::XMMatrixScaling(1.0f, 3.0f, 1.0f);
            world_matrix = scale_matrix * rotation_matrix * left_cyl_world;
            world_view_projection_matrix = DirectX::XMMatrixMultiply(world_matrix, view_projection_matrix);

            commandList->set_graphics_dynamic_constant_buffer(root_parameters::matrices_cb, world_view_projection_matrix);

            m_cylinder->draw(commandList);

            DirectX::XMMATRIX right_cyl_world = DirectX::XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f);
            // Calculate MVP
            rotation_matrix = DirectX::XMMatrixIdentity();
            scale_matrix = DirectX::XMMatrixScaling(1.0f, 3.0f, 1.0f);
            world_matrix = scale_matrix * rotation_matrix * right_cyl_world;
            world_view_projection_matrix = DirectX::XMMatrixMultiply(world_matrix, view_projection_matrix);

            commandList->set_graphics_dynamic_constant_buffer(root_parameters::matrices_cb, world_view_projection_matrix);

            m_cylinder->draw(commandList);

            DirectX::XMMATRIX left_sphere_world = DirectX::XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f);
            // Calculate MVP
            rotation_matrix = DirectX::XMMatrixIdentity();
            scale_matrix = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
            world_matrix = scale_matrix * rotation_matrix * left_sphere_world;
            world_view_projection_matrix = DirectX::XMMatrixMultiply(world_matrix, view_projection_matrix);

            commandList->set_graphics_dynamic_constant_buffer(root_parameters::matrices_cb, world_view_projection_matrix);

            m_sphere->draw(commandList);

            DirectX::XMMATRIX right_sphere_world = DirectX::XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f);
            // Calculate MVP
            rotation_matrix = DirectX::XMMatrixIdentity();
            scale_matrix = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
            world_matrix = scale_matrix * rotation_matrix * right_sphere_world;
            world_view_projection_matrix = DirectX::XMMatrixMultiply(world_matrix, view_projection_matrix);

            commandList->set_graphics_dynamic_constant_buffer(root_parameters::matrices_cb, world_view_projection_matrix);

            m_sphere->draw(commandList);
        }
    }
}