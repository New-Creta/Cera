#pragma once

#include <wrl/client.h>
#include <wrl.h>

namespace cera
{
  namespace wrl
  {
    template <typename T>
    using com_ptr = Microsoft::WRL::ComPtr<T>;
  } // namespace wrl
} // namespace cera