#pragma once

#include "util/types.h"

#include "render/d3dx12_declarations.h"

namespace cera
{
    class device;

    class pipeline_state_object
    {
    public:
        Microsoft::WRL::ComPtr<ID3D12PipelineState> get_d3d_pipeline_state_object() const;

    protected:
        pipeline_state_object(device& device, const D3D12_PIPELINE_STATE_STREAM_DESC& desc);
        virtual ~pipeline_state_object();

    private:
        device& m_device;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_d3d_pipeline_state_object;
    };
}