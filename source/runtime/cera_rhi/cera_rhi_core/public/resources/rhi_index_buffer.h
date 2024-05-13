
#pragma once

#include "rhi_buffer.h"
#include "rhi_format.h"

namespace cera
{
    class rhi_index_buffer : public rhi_buffer
    {
    public:
        ~rhi_index_buffer() override = default;

        virtual size_t get_num_indices() const = 0;
    };
}