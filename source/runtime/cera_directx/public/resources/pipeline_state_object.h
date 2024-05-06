#pragma once

#include "cera_engine/engine/types.h"

#include "directx_util.h"

#include "cera_renderer_core/iresource.h"

namespace cera
{
  namespace renderer
  {
    class Device;

    class PipelineStateObject : public IResource
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