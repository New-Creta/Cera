#pragma once

#include <wrl/client.h>
#include <wrl.h>

namespace cera
{
  namespace wrl
  {
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;
  } // namespace wrl
} // namespace cera