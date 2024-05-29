#include "resources/buffer.h"

namespace cera
{
    namespace renderer
    {
        //-------------------------------------------------------------------------
        Buffer::Buffer(d3d12_device& device, const D3D12_RESOURCE_DESC& res_desc) : resource(device, res_desc)
        {}

        //-------------------------------------------------------------------------
        Buffer::Buffer(d3d12_device& device, wrl::com_ptr<ID3D12Resource> resource)
            : resource(device, resource)
        {}

        //-------------------------------------------------------------------------
        Buffer::~Buffer() = default;
    }
}