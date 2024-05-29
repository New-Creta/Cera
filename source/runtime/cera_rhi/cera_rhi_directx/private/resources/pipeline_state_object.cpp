#include "resources/pipeline_state_object.h"

#include "rhi_directx_device.h"

#include "util/assert.h"

namespace cera
{
  namespace renderer
  {
    PipelineStateObject::PipelineStateObject(d3d12_device& device, const D3D12_PIPELINE_STATE_STREAM_DESC& desc)
        : m_device(device)
    {
      auto d3d_device = device.d3d_device();

      auto hr = d3d_device->CreatePipelineState(&desc, IID_PPV_ARGS(&m_d3d_pipeline_state_object));

      CERA_ASSERT_X(SUCCEEDED(hr), "Unable to CreatePipelineState");
    }

    PipelineStateObject::~PipelineStateObject() = default;

    wrl::com_ptr<ID3D12PipelineState> PipelineStateObject::d3d_pipeline_state_object() const
    {
      return m_d3d_pipeline_state_object;
    }
  } // namespace renderer
} // namespace cera