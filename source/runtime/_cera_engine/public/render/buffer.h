#pragma once

#include "render/resource.h"

namespace cera
{
    class device;

    class buffer : public resource
    {
    protected:
        buffer(device& device, const D3D12_RESOURCE_DESC& resDesc);
        buffer(device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource);
        ~buffer() override;
    };
}