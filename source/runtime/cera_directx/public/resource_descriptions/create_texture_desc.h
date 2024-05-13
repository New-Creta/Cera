#pragma once

#include "resource_descriptions/create_clear_state_desc.h"
#include "resource_descriptions/create_resource_desc.h"

#include "rhi_format.h"

namespace cera
{
  namespace renderer
  {
    struct CreateTextureDesc
    {
      CreateResourceDesc resource_desc       = {};
      CreateClearStateDesc* clear_state_desc = nullptr;
      Format clear_state_format              = Format::UNKNOWN;
    };
  } // namespace renderer
} // namespace cera