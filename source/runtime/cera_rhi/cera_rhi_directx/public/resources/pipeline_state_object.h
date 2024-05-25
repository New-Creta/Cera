#pragma once

#include "util/types.h"
#include "util/windows_types.h"

#include "directx_util.h"

#include "resources/rhi_resource.h"

namespace cera
{
  namespace renderer
  {
    class Device;

    class PipelineStateObject : public rhi_resource
    {
    public:
      RESOURCE_CLASS_TYPE(PipelineStateObject);

      wrl::ComPtr<ID3D12PipelineState> d3d_pipeline_state_object() const;

    protected:
      PipelineStateObject(Device& device, const D3D12_PIPELINE_STATE_STREAM_DESC& desc);
      virtual ~PipelineStateObject();

    private:
      Device& m_device;
      wrl::ComPtr<ID3D12PipelineState> m_d3d_pipeline_state_object;
    };
  } // namespace renderer
}