#include "render/pipeline_state_object.h"
#include "render/device.h"

namespace cera
{
    pipeline_state_object::pipeline_state_object(device& device, const D3D12_PIPELINE_STATE_STREAM_DESC& desc)
        : m_device(device)
    {
        auto d3d_device = device.get_d3d_device();

        auto hr = d3d_device->CreatePipelineState(&desc, IID_PPV_ARGS(&m_d3d_pipeline_state_object));

        assert(SUCCEEDED(hr));
    }

    pipeline_state_object::~pipeline_state_object() = default;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_state_object::get_d3d_pipeline_state_object() const
    {
        return m_d3d_pipeline_state_object;
    }
}