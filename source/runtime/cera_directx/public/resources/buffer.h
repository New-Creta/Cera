#pragma once

#include "resources/resource.h"

/**
 * @brief Abstract base class for buffer resources.
 */

namespace cera
{
    namespace renderer
    {
        class Device;

        class Buffer : public Resource
        {
        protected:
            Buffer(Device& device, const D3D12_RESOURCE_DESC& resDesc);
            Buffer(Device& device, wrl::ComPtr<ID3D12Resource> resource);
            ~Buffer() override;
        };
    }
}