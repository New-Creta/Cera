#pragma once

#include "util/types.h"

namespace cera
{
  namespace renderer
  {
    struct Info
    {
      std::string api_version;
      std::string shader_version;
      std::string adaptor;
      std::string vendor;
    };
  } // namespace renderer
} // namespace cera