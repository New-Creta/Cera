#pragma once

#include "util/types.h"

#include "wrl/comobject.h"
#include "gpu_description.h"

struct IDXGIAdapter4;

namespace cera
{
  namespace dxgi
  {
    class Adapter : public wrl::ComObject<IDXGIAdapter4> // NOLINT(fuchsia-multiple-inheritance)
    {
    public:
      Adapter(wrl::ComPtr<IDXGIAdapter4>&& adapter);

      const renderer::GpuDescription& description() const;

    private:
      renderer::GpuDescription m_description;
    };
  } // namespace dxgi
} // namespace cera