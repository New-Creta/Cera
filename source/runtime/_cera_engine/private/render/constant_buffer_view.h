#pragma once

#include "util/types.h"

#include "render/d3dx12_declarations.h"
#include "render/descriptor_allocation.h"

#include <memory>

namespace cera
{
    class constant_buffer;
    class device;

    class constant_buffer_view
    {
    public:
        std::shared_ptr<constant_buffer> get_constant_buffer() const;

        D3D12_CPU_DESCRIPTOR_HANDLE get_descriptor_handle();

    protected:
        constant_buffer_view(device& device, const std::shared_ptr<constant_buffer>& constantBuffer, size_t offset = 0);
        virtual ~constant_buffer_view();

    private:
        device& m_device;
        std::shared_ptr<constant_buffer> m_constant_buffer;
        descriptor_allocation m_descriptor;
    };
}