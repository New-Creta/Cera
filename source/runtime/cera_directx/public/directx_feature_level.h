#pragma once

#include "dxgi/dxgi_util.h"

#include "directx_util.h"

#include <string>

namespace cera
{
  std::string_view feature_level_name(D3D_FEATURE_LEVEL level);

  bool is_correct_feature_level(D3D_FEATURE_LEVEL level);

  D3D_FEATURE_LEVEL query_feature_level(IDXGIAdapter* adapter);
} // namespace cera