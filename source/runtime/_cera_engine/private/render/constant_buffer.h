#pragma once

#include "device/windows_types.h"

#include "render/buffer.h"
#include "render/d3dx12_declarations.h"

namespace cera
{
    class constant_buffer : public buffer
    {
    public:
        size_t get_size_in_bytes() const;

    protected:
        constant_buffer(device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource);
        ~constant_buffer() override;

    private:
        size_t m_size_in_bytes;
    };
}