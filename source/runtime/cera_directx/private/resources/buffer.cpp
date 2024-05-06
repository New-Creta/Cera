#include "resources/buffer.h"

namespace cera
{
    namespace renderer
    {
        //-------------------------------------------------------------------------
        Buffer::Buffer(Device& device, const D3D12_RESOURCE_DESC& resDesc)
            : Resource(device, resDesc)
        {}

        //-------------------------------------------------------------------------
        Buffer::Buffer(Device& device, wrl::ComPtr<ID3D12Resource> resource)
            : Resource(device, resource)
        {}

        //-------------------------------------------------------------------------
        Buffer::~Buffer() = default;
    }
}