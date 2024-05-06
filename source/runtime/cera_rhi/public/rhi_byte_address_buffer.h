#pragma once

#include "rhi_buffer.h"

namespace cera
{
    class rhi_byte_address_buffer : public rhi_buffer
    {
    public:
        ~rhi_byte_address_buffer() override = default;

        virtual size_t get_buffer_size() const = 0;
    }
}