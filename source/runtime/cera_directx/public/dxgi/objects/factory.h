#pragma once

#include "util/types.h"

#include "wrl/comobject.h"

#include <memory>

struct IDXGIFactory;

namespace cera
{
  namespace dxgi
  {
    class Factory : public wrl::ComObject<IDXGIFactory>
    {
    public:
      static std::unique_ptr<Factory> create(u32 flags);

    protected:
      Factory();
      Factory(wrl::ComPtr<IDXGIFactory>&& object);
    };
  } // namespace dxgi
} // namespace cera