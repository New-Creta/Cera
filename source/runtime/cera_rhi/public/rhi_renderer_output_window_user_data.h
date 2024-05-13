#pragma once

#include "util/types.h"

namespace cera
{
  namespace renderer
  {
    struct OutputWindowUserData
    {
      void* primary_display_handle;

      s32 window_width;
      s32 window_height;

      s32 refresh_rate;

      bool windowed;
    };
  } // namespace renderer
} // namespace cera