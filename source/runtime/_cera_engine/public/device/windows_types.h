#pragma once

#include "util/types.h"

#include <wrl/client.h>

namespace cera
{
  namespace win
  {
    using HInstance = void*;
    using Hwnd      = void*;
    using Hdc       = void*;

    using WParam  = size_t;
    using LParam  = long long;
    using LResult = size_t;

#ifndef UNICODE
    using LPtStr = char*;
#else
    using LPtStr = wchar_t*;
#endif

    using DWord               = unsigned long;
    using WindowProcedureFunc = LResult(__stdcall*)(Hwnd, s32, WParam, LParam);

    struct Rect
    {
        s32 left;
        s32 top;
        s32 right;
        s32 bottom;
    };
  } // namespace win

  namespace wrl
  {
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;
  } // namespace wrl
} // namespace cera