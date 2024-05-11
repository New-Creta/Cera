#pragma once

#include "util/defines.h"
#include "util/types.h"

namespace cera
{
  namespace renderer
  {
    enum class ClearBits
    {
      None               = BIT(0),
      ClearColorBuffer   = BIT(1),
      ClearDepthBuffer   = BIT(2),
      ClearStencilBuffer = BIT(3),
    };

    bool operator&(ClearBits bits1, ClearBits bits2);
    bool operator|(ClearBits bits1, ClearBits bits2);
  } // namespace renderer
} // namespace cera