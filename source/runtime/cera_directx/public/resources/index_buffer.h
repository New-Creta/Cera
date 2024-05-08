#pragma once

#include "util/types.h"

#include "directx_util.h"
#include "resources/buffer.h"

namespace cera
{
  namespace renderer
  {
    class IndexBuffer : public Buffer
    {
    public:
      RESOURCE_CLASS_TYPE(IndexBuffer);

      s64 num_indices() const;

      DXGI_FORMAT index_format() const;

      /**
       * Get the index buffer view for biding to the Input Assembler stage.
       */
      D3D12_INDEX_BUFFER_VIEW index_buffer_view() const;

    protected:
      IndexBuffer(Device& device, s64 numIndices, DXGI_FORMAT indexFormat);
      IndexBuffer(Device& device, wrl::ComPtr<ID3D12Resource> resource, s64 numIndices, DXGI_FORMAT indexFormat);
      ~IndexBuffer() override;

      void create_index_buffer_view();

    private:
      s64 m_num_indicies;
      DXGI_FORMAT m_index_format;

      D3D12_INDEX_BUFFER_VIEW m_index_buffer_view;
    };
  } // namespace renderer
}