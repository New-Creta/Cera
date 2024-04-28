#include "render/buffer.h"

namespace cera
{
    buffer::buffer(device& device, const D3D12_RESOURCE_DESC& resDesc)
        : resource(device, resDesc)
    {}

    buffer::buffer(device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource)
        : resource(device, resource)
    {}

    buffer::~buffer() = default;
}