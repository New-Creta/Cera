#pragma once

#include "cera/types.h"

#include <Windows.h>

namespace cera
{
    class buffer
    {
    public:
        buffer(s32 width, s32 height);
        ~buffer();

        HBITMAP& get();
        const HBITMAP& get() const;

        u32* data();
        const u32* data() const;

    private:
        HBITMAP m_hbm;
        u32* m_data;
    };
}