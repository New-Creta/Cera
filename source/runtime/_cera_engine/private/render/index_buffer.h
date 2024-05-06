#pragma once

#include "device/windows_types.h"

#include "render/buffer.h"
#include "render/d3dx12_declarations.h"

namespace cera
{
    class index_buffer : public buffer
    {
    public:
        size_t get_num_indices() const;

        DXGI_FORMAT get_index_format() const;

        /**
         * Get the index buffer view for biding to the Input Assembler stage.
         */
        D3D12_INDEX_BUFFER_VIEW get_index_buffer_view() const;

    protected:
        index_buffer(device& device, size_t numIndices, DXGI_FORMAT indexFormat);
        index_buffer(device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numIndices, DXGI_FORMAT indexFormat);
        ~index_buffer() override;

        void create_index_buffer_view();

    private:
        size_t m_num_indicies;
        DXGI_FORMAT m_index_format;

        D3D12_INDEX_BUFFER_VIEW m_index_buffer_view;
    };
}