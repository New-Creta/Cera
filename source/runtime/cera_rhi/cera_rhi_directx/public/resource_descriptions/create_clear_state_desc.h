#pragma once

#include "util/state_controller.h"
#include "util/types.h"
#include "util/color.h"

#include "common/rhi_clear_bits.h"

namespace cera
{
  namespace renderer
  {
    struct CreateClearStateDesc
    {
      CreateClearStateDesc()
          : rgba(1.0f, 1.0f, 1.0f, 1.0f)
          , depth(1.0f)
          , stencil(0x00)
          , flags()
      {
        flags.add_state(renderer::ClearBits::ClearColorBuffer);
        flags.add_state(renderer::ClearBits::ClearDepthBuffer);
      }

      Color4f rgba;
      f32 depth;
      u8 stencil;
      StateController<ClearBits> flags {};
    };
  } // namespace renderer
} // namespace cera