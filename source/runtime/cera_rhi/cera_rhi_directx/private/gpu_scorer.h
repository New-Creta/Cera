#pragma once

#include "dxgi/objects/adapter.h"

#include <functional>
#include <memory>
#include <vector>

namespace cera
{
  namespace renderer
  {
    using adapter_scorer_fn = std::function<size_t(const std::vector<dxgi::adapter_description>&)>;
  }
} // namespace cera