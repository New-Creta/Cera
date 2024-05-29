#pragma once

#include "resources/resource.h"

/**
 * @brief Abstract base class for buffer resources.
 */

namespace cera
{
    namespace renderer
    {
        class d3d12_device;

        class Buffer : public resource
        {
        protected:
            Buffer(d3d12_device& device, const D3D12_RESOURCE_DESC& resDesc);
            Buffer(d3d12_device& device, wrl::com_ptr<ID3D12Resource> resource);
            ~Buffer() override;
        };
    }
}