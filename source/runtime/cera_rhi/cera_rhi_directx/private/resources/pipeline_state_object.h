#pragma once

#include "util/types.h"
#include "util/windows_types.h"

#include "directx_util.h"

#include "resources/rhi_resource.h"

namespace cera
{
  namespace renderer
  {
    class d3d12_device;

    class PipelineStateObject : public rhi_resource
    {
    public:
      RESOURCE_CLASS_TYPE(PipelineStateObject);

      wrl::com_ptr<ID3D12PipelineState> d3d_pipeline_state_object() const;

    protected:
      PipelineStateObject(d3d12_device& device, const D3D12_PIPELINE_STATE_STREAM_DESC& desc);
      virtual ~PipelineStateObject();

    private:
      d3d12_device& m_device;
      wrl::com_ptr<ID3D12PipelineState> m_d3d_pipeline_state_object;
    };
  } // namespace renderer
}