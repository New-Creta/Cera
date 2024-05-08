#pragma once

#include "util/types.h"
#include "util/memory_size.h"

#include <ostream>
#include <string>

namespace cera
{
  namespace renderer
  {
    struct GpuDescription
    {
      GpuDescription();

      std::string name;
      std::string vendor_name;

      u32 vendor_id;
      u32 device_id;

      memory_size dedicated_video_memory;
      memory_size dedicated_system_memory;
      memory_size shared_system_memory;
    };

    std::ostream& operator<<(std::ostream& os, const GpuDescription& desc);
  } // namespace renderer
} // namespace cera