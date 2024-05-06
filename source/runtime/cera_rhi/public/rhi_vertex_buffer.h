#pragma once

#include "rhi_buffer.h"

namespace cera
{
    class rhi_vertex_buffer : public rhi_buffer
    {
    public:
        ~rhi_vertex_buffer() override = default;

        virtual size_t get_num_vertices() const = 0;
        virtual size_t get_vertex_stride() const = 0;
    };
}