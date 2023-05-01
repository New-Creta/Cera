#include "cera/buffer.h"



namespace cera
{
    buffer::buffer(s32 width, s32 height)
    {
        HDC hdc = GetDC(NULL);

        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height; // top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        m_hbm = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, reinterpret_cast<void**>(&m_data), NULL, NULL);

        ReleaseDC(NULL, hdc);
    }

    buffer::~buffer()
    {
        DeleteObject(m_hbm);
    }

    HBITMAP& buffer::get()
    {
        return m_hbm;
    }
    const HBITMAP& buffer::get() const
    {
        return m_hbm;
    }

    u32* buffer::data()
    {
        return m_data;
    }

    const u32* buffer::data() const
    {
        return m_data;
    }
}