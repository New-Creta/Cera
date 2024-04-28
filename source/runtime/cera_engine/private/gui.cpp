#include "gui.h"

#include "ImGUI_PS.h"
#include "ImGUI_VS.h"
#include "imgui_impl_win32.h"

#include "render/command_queue.h"
#include "render/command_list.h"
#include "render/device.h"
#include "render/texture.h"
#include "render/root_signature.h"
#include "render/render_target.h"

// Windows message handler for ImGui.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace cera
{
    // Root parameters for the ImGui root signature.
    enum root_parameters
    {
        matrixcb,           // cbuffer vertexBuffer : register(b0)
        font_texture,       // Texture2D texture0 : register(t0);
        num_root_parameters
    };

    namespace internal
    {
        //--------------------------------------------------------------------------------------
        // Get surface information for a particular format
        //--------------------------------------------------------------------------------------
        void get_surface_info(_In_ size_t width, _In_ size_t height, _In_ DXGI_FORMAT fmt, size_t* outNumBytes, _Out_opt_ size_t* outRowBytes, _Out_opt_ size_t* outNumRows)
        {
            size_t num_bytes = 0;
            size_t row_bytes = 0;
            size_t num_rows = 0;

            bool   bc = false;
            bool   packed = false;
            bool   planar = false;
            size_t bpe = 0;
            switch (fmt)
            {
            case DXGI_FORMAT_BC1_TYPELESS:
            case DXGI_FORMAT_BC1_UNORM:
            case DXGI_FORMAT_BC1_UNORM_SRGB:
            case DXGI_FORMAT_BC4_TYPELESS:
            case DXGI_FORMAT_BC4_UNORM:
            case DXGI_FORMAT_BC4_SNORM:
                bc = true;
                bpe = 8;
                break;

            case DXGI_FORMAT_BC2_TYPELESS:
            case DXGI_FORMAT_BC2_UNORM:
            case DXGI_FORMAT_BC2_UNORM_SRGB:
            case DXGI_FORMAT_BC3_TYPELESS:
            case DXGI_FORMAT_BC3_UNORM:
            case DXGI_FORMAT_BC3_UNORM_SRGB:
            case DXGI_FORMAT_BC5_TYPELESS:
            case DXGI_FORMAT_BC5_UNORM:
            case DXGI_FORMAT_BC5_SNORM:
            case DXGI_FORMAT_BC6H_TYPELESS:
            case DXGI_FORMAT_BC6H_UF16:
            case DXGI_FORMAT_BC6H_SF16:
            case DXGI_FORMAT_BC7_TYPELESS:
            case DXGI_FORMAT_BC7_UNORM:
            case DXGI_FORMAT_BC7_UNORM_SRGB:
                bc = true;
                bpe = 16;
                break;

            case DXGI_FORMAT_R8G8_B8G8_UNORM:
            case DXGI_FORMAT_G8R8_G8B8_UNORM:
            case DXGI_FORMAT_YUY2:
                packed = true;
                bpe = 4;
                break;

            case DXGI_FORMAT_Y210:
            case DXGI_FORMAT_Y216:
                packed = true;
                bpe = 8;
                break;

            case DXGI_FORMAT_NV12:
            case DXGI_FORMAT_420_OPAQUE:
                planar = true;
                bpe = 2;
                break;             
            case DXGI_FORMAT_P010:
            case DXGI_FORMAT_P016:
                planar = true;
                bpe = 4;
                break;
            }

            if (bc)
            {
                size_t num_blocks_wide = 0;
                if (width > 0)
                {
                    num_blocks_wide = std::max<size_t>(1, (width + 3) / 4);
                }
                size_t num_blocks_high = 0;
                if (height > 0)
                {
                    num_blocks_high = std::max<size_t>(1, (height + 3) / 4);
                }
                row_bytes = num_blocks_wide * bpe;
                num_rows = num_blocks_high;
                num_bytes = row_bytes * num_blocks_high;
            }
            else if (packed)
            {
                row_bytes = ((width + 1) >> 1) * bpe;
                num_rows = height;
                num_bytes = row_bytes * height;
            }
            else if (fmt == DXGI_FORMAT_NV11)
            {
                row_bytes = ((width + 3) >> 2) * 4;
                num_rows = height * 2;  // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
                num_bytes = row_bytes * num_rows;
            }
            else if (planar)
            {
                row_bytes = ((width + 1) >> 1) * bpe;
                num_bytes = (row_bytes * height) + ((row_bytes * height + 1) >> 1);
                num_rows = height + ((height + 1) >> 1);
            }
            else
            {
                size_t bpp = DirectX::BitsPerPixel(fmt);
                row_bytes = (width * bpp + 7) / 8;  // round up to nearest byte
                num_rows = height;
                num_bytes = row_bytes * height;
            }

            if (outNumBytes)
            {
                *outNumBytes = num_bytes;
            }
            if (outRowBytes)
            {
                *outRowBytes = row_bytes;
            }
            if (outNumRows)
            {
                *outNumRows = num_rows;
            }
        }
    }

    gui::gui(device& device, void* hwnd, const render_target& renderTarget)
        :m_device(device)
        ,m_imgui(nullptr)
    {
        m_imgui = ImGui::CreateContext();

        ImGui::SetCurrentContext(m_imgui);

        bool result = ImGui_ImplWin32_Init((HWND)hwnd);

        assert(result && "Failed to initialize ImGui");

        ImGuiIO& io = ImGui::GetIO();

        io.FontGlobalScale = ::GetDpiForWindow((HWND)hwnd) / 96.0f;
        // Allow user UI scaling using CTRL+Mouse Wheel scrolling
        io.FontAllowUserScaling = true;

        // Build texture atlas
        unsigned char* pixel_data = nullptr;
        int            width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixel_data, &width, &height);

        auto& command_queue = device.get_command_queue(D3D12_COMMAND_LIST_TYPE_COPY);
        auto  command_list = command_queue.get_command_list();

        auto font_texture_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height);

        m_font_texture = device.create_texture(font_texture_desc);
        m_font_texture->set_resource_name(L"ImGui Font Texture");
        m_font_srv = device.create_shader_resource_view(m_font_texture);

        size_t row_pitch, slice_pitch;
        internal::get_surface_info(width, height, DXGI_FORMAT_R8G8B8A8_UNORM, &slice_pitch, &row_pitch, nullptr);

        D3D12_SUBRESOURCE_DATA subresource_data;
        subresource_data.pData = pixel_data;
        subresource_data.RowPitch = row_pitch;
        subresource_data.SlicePitch = slice_pitch;

        command_list->copy_texture_subresource(m_font_texture, 0, 1, &subresource_data);

        command_queue.execute_command_list(command_list);

        auto d3d_device = m_device.get_d3d_device();

        // Create the root signature for the ImGUI shaders.
        // Allow input layout and deny unnecessary access to certain pipeline stages.
        D3D12_ROOT_SIGNATURE_FLAGS root_signature_flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        CD3DX12_DESCRIPTOR_RANGE1 descriptor_range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

        CD3DX12_ROOT_PARAMETER1 root_parameters[root_parameters::num_root_parameters];
        root_parameters[root_parameters::matrixcb].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
        root_parameters[root_parameters::font_texture].InitAsDescriptorTable(1, &descriptor_range, D3D12_SHADER_VISIBILITY_PIXEL);

        CD3DX12_STATIC_SAMPLER_DESC linear_repeat_sampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT);
        linear_repeat_sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        linear_repeat_sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC root_signature_description;
        root_signature_description.Init_1_1(root_parameters::num_root_parameters, root_parameters, 1, &linear_repeat_sampler, root_signature_flags);

        m_root_signature = m_device.create_root_signature(root_signature_description.Desc_1_1);

        // clang-format off
        const D3D12_INPUT_ELEMENT_DESC inputLayout[] = 
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(ImDrawVert, pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(ImDrawVert, uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(ImDrawVert, col), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };
        // clang-format on

        D3D12_BLEND_DESC blend_desc = {};
        blend_desc.RenderTarget[0].BlendEnable = true;
        blend_desc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blend_desc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        blend_desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blend_desc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
        blend_desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        blend_desc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        D3D12_RASTERIZER_DESC rasterizer_desc = {};
        rasterizer_desc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizer_desc.CullMode = D3D12_CULL_MODE_NONE;
        rasterizer_desc.FrontCounterClockwise = FALSE;
        rasterizer_desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        rasterizer_desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        rasterizer_desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rasterizer_desc.DepthClipEnable = true;
        rasterizer_desc.MultisampleEnable = FALSE;
        rasterizer_desc.AntialiasedLineEnable = FALSE;
        rasterizer_desc.ForcedSampleCount = 0;
        rasterizer_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        D3D12_DEPTH_STENCIL_DESC depth_stencil_desc = {};
        depth_stencil_desc.DepthEnable = false;
        depth_stencil_desc.StencilEnable = false;

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
            CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC            blend_desc;
            CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER            rasterizer_state;
            CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL         depth_stencil_state;
        } pipeline_state_stream;

        pipeline_state_stream.p_root_signature = m_root_signature->get_d3d_root_signature().Get();
        pipeline_state_stream.input_layout = { inputLayout, 3 };
        pipeline_state_stream.primitive_topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pipeline_state_stream.VS = { g_ImGUI_VS, sizeof(g_ImGUI_VS) };
        pipeline_state_stream.PS = { g_ImGUI_PS, sizeof(g_ImGUI_PS) };
        pipeline_state_stream.RTV_formats = renderTarget.get_render_target_formats();
        pipeline_state_stream.sample_desc = renderTarget.get_sample_desc();
        pipeline_state_stream.blend_desc = CD3DX12_BLEND_DESC(blend_desc);
        pipeline_state_stream.rasterizer_state = CD3DX12_RASTERIZER_DESC(rasterizer_desc);
        pipeline_state_stream.depth_stencil_state = CD3DX12_DEPTH_STENCIL_DESC(depth_stencil_desc);

        m_pipeline_state_object = device.create_pipeline_state_object(pipeline_state_stream);
    }

    gui::~gui()
    {
        destroy();
    }

    LRESULT gui::wnd_proc_handler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
    }

    void gui::new_frame()
    {
        ImGui::SetCurrentContext(m_imgui);
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void gui::draw(const std::shared_ptr<command_list>& commandList, const render_target& renderTarget)
    {
        assert(commandList);

        ImGui::SetCurrentContext(m_imgui);
        ImGui::Render();

        ImGuiIO& io = ImGui::GetIO();
        ImDrawData* draw_data = ImGui::GetDrawData();

        // Check if there is anything to render.
        if (!draw_data || draw_data->CmdListsCount == 0)
            return;

        ImVec2 display_pos = draw_data->DisplayPos;

        commandList->set_pipeline_state(m_pipeline_state_object);
        commandList->set_graphics_root_signature(m_root_signature);
        commandList->set_render_target(renderTarget);

        // Set root arguments.
        //    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixOrthographicRH( draw_data->DisplaySize.x,
        //    draw_data->DisplaySize.y, 0.0f, 1.0f );
        float L = draw_data->DisplayPos.x;
        float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
        float T = draw_data->DisplayPos.y;
        float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
        float mvp[4][4] = {
            { 2.0f / (R - L), 0.0f, 0.0f, 0.0f },
            { 0.0f, 2.0f / (T - B), 0.0f, 0.0f },
            { 0.0f, 0.0f, 0.5f, 0.0f },
            { (R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f },
        };

        commandList->set_graphics_32_bit_constants(root_parameters::matrixcb, mvp);
        commandList->set_shader_resource_view(root_parameters::font_texture, 0, m_font_srv, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        D3D12_VIEWPORT viewport = {};
        viewport.Width = draw_data->DisplaySize.x;
        viewport.Height = draw_data->DisplaySize.y;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        commandList->set_viewport(viewport);
        commandList->set_primitive_topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        const DXGI_FORMAT index_format = sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

        // It may happen that ImGui doesn't actually render anything. In this case,
        // any pending resource barriers in the commandList will not be flushed (since
        // resource barriers are only flushed when a draw command is executed).
        // In that case, manually flushing the resource barriers will ensure that
        // they are properly flushed before exiting this function.
        commandList->flush_resource_barriers();

        for (int i = 0; i < draw_data->CmdListsCount; ++i)
        {
            const ImDrawList* draw_list = draw_data->CmdLists[i];

            commandList->set_dynamic_vertex_buffer(0, draw_list->VtxBuffer.size(), sizeof(ImDrawVert), draw_list->VtxBuffer.Data);
            commandList->set_dynamic_index_buffer(draw_list->IdxBuffer.size(), index_format, draw_list->IdxBuffer.Data);

            int index_offset = 0;
            for (int j = 0; j < draw_list->CmdBuffer.size(); ++j)
            {
                const ImDrawCmd& drawCmd = draw_list->CmdBuffer[j];
                if (drawCmd.UserCallback)
                {
                    drawCmd.UserCallback(draw_list, &drawCmd);
                }
                else
                {
                    ImVec4     clip_rect = drawCmd.ClipRect;
                    D3D12_RECT scissor_rect;
                    scissor_rect.left = static_cast<LONG>(clip_rect.x - display_pos.x);
                    scissor_rect.top = static_cast<LONG>(clip_rect.y - display_pos.y);
                    scissor_rect.right = static_cast<LONG>(clip_rect.z - display_pos.x);
                    scissor_rect.bottom = static_cast<LONG>(clip_rect.w - display_pos.y);

                    if (scissor_rect.right - scissor_rect.left > 0.0f && scissor_rect.bottom - scissor_rect.top > 0.0)
                    {
                        commandList->set_scissor_rect(scissor_rect);
                        commandList->draw_indexed(drawCmd.ElemCount, 1, index_offset);
                    }
                }

                index_offset += drawCmd.ElemCount;
            }
        }
    }
    
    void gui::destroy()
    {
        ImGui::EndFrame();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext(m_imgui);
        m_imgui = nullptr;
    }
}