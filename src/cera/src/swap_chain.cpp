#include "cera/swap_chain.h"

#include <algorithm>

namespace cera
{
    swap_chain::swap_chain(s32 width, s32 height)
        :m_front(std::make_unique<buffer>(width, height))
        ,m_back(std::make_unique<buffer>(width, height))
        ,m_width(width)
        ,m_height(height)
    {

    }

    buffer& swap_chain::curr_buffer()
    {
        return *m_front.get();
    }

    const buffer& swap_chain::curr_buffer() const
    {
        return *m_front.get();
    }

    void swap_chain::clear()
    {
        std::fill(m_back->data(), m_back->data() + size(), 0);
    }

    void swap_chain::set_pixel(s32 x, s32 y, u32 pix) 
    {
        m_back->data()[y * m_width + x] = pix;
    }

    void swap_chain::swap()
    {
        std::swap(m_front, m_back);
    }

    void swap_chain::resize(s32 width, s32 height)
    {
        m_front = std::make_unique<buffer>(width, height);
        m_back = std::make_unique<buffer>(width, height);
        m_width = width;
        m_height = height;
    }

    s32 swap_chain::size() const
    {
        return m_width * m_height;
    }

    s32 swap_chain::width() const
    {
        return m_width;
    }

    s32 swap_chain::height() const
    {
        return m_height;
    }
}