#pragma once

#include "util/types.h"

#include "directx_util.h"
#include "resources/buffer.h"

namespace cera
{
  namespace renderer
  {
    class Device;

    /**
     *  @brief ByteAddressBuffer is a type of buffer resource used for storing structured data in GPU memory.
     *  It is specifically designed to store data in a linear byte-addressable format, which means you can access
     *  individual bytes within the buffer. ByteAddressBuffer is particularly useful for storing raw data,
     *  such as constant buffers, structured buffers, or data that does not have a specific format like textures
     *  or vertex buffers. It provides a flexible way to store and access data in GPU shaders.
     */
    class ByteAddressBuffer : public Buffer
    {
    public:
      RESOURCE_CLASS_TYPE(ByteAddressBuffer);

      size_t buffer_size() const;

    protected:
      ByteAddressBuffer(Device& device, const D3D12_RESOURCE_DESC& resDesc);
      ByteAddressBuffer(Device& device, wrl::ComPtr<ID3D12Resource> resource);
      ~ByteAddressBuffer() override;

    private:
      size_t m_buffer_size;
    };
  } // namespace renderer
}