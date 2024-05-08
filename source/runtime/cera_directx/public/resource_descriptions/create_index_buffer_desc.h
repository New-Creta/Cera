#pragma once

#include "rex_renderer_core/format.h"

namespace cera
{
  namespace renderer
  {
    struct CreateIndexBufferDesc
    {
      size_t num_indices;
      Format index_format;
    };
  }   // namespace renderer
} // namespace cera