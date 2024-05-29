#pragma once

#include "util/windows_types.h"
#include "util/blob.h"

#include "directx_util.h"

#include "common/rhi_shader_type.h"

#include <memory>
#include <string>

namespace cera
{
  namespace renderer
  {
    class RootSignature;
    class d3d12_device;

    struct ShaderInfo
    {
      D3D12_INPUT_LAYOUT_DESC input_layout;

      std::shared_ptr<RootSignature> root_signature;

      wrl::com_ptr<ID3DBlob> vertex_shader;
      wrl::com_ptr<ID3DBlob> pixel_shader;
    };

    namespace shader_library
    {
      namespace root_parameters
      {
        // An enum for root signature parameters.
        // I'm not using scoped enums to avoid the explicit cast that would be required
        // to use these as root indices in the root signature.
        enum Unlit
        {
          MATRICES_CB, // ConstantBuffer<Mat> MatCB : register(b0);
          NUM_ROOT_PARAMETERS
        };
      } // namespace root_parameters

      namespace tags
      {
        static const std::string unlit = "unlit";
      }

      struct ShaderCompilationDesc
      {
        ShaderType shader_type;

        std::string shader_entry_point;
        std::string shader_name;
        std::string shader_feature_target;

        memory::Blob shader_code;
      };

      wrl::com_ptr<ID3DBlob> compile_shader(const ShaderCompilationDesc& desc);

      void load(d3d12_device* device);
      void unload();

      ShaderInfo find_shader_info(std::string_view name);
    } // namespace shader_library
  }   // namespace renderer
} // namespace cera