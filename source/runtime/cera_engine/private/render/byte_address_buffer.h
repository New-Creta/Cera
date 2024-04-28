#pragma once

#include "render/d3dx12_declarations.h"
#include "render/buffer.h"

namespace cera
{
    class device;

    /**
     *  @brief byte_address_buffer is a type of buffer resource used for storing structured data in GPU memory.
     *  It is specifically designed to store data in a linear byte-addressable format, which means you can access 
     *  individual bytes within the buffer. byte_address_buffer is particularly useful for storing raw data, 
     *  such as constant buffers, structured buffers, or data that does not have a specific format like textures
     *  or vertex buffers. It provides a flexible way to store and access data in GPU shaders.
     */
    class byte_address_buffer : public buffer
    {
    public:
        size_t get_buffer_size() const;

    protected:
        byte_address_buffer(device& device, const D3D12_RESOURCE_DESC& resDesc);
        byte_address_buffer(device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource);
        ~byte_address_buffer() override;

    private:
        size_t m_buffer_size;
    };
}