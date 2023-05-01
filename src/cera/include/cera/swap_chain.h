#pragma once

#include "cera/buffer.h"

#include <memory>

namespace cera
{
    class swap_chain
    {
    public:
        swap_chain(s32 width, s32 height);

        buffer& curr_buffer();
        const buffer& curr_buffer() const;

        void clear();
        void set_pixel(s32 x, s32 y, u32 pix);
        void swap();
        void resize(s32 width, s32 height);

        s32 size() const;
        s32 width() const;
        s32 height() const;

    private:
        std::unique_ptr<buffer> m_front;
        std::unique_ptr<buffer> m_back;

        s32 m_width;
        s32 m_height;
    };
}