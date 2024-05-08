#pragma once

#include "gpu_description.h"

#include <functional>
#include <memory>
#include <vector>

namespace cera
{
  namespace renderer
  {
    using GpuScorerFn = std::function<size_t(const std::vector<GpuDescription>&)>;
  }
} // namespace cera