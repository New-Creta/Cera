#pragma once

#include "dxgi/objects/adapter.h"
#include "gpu_scorer.h"

#include <vector>

namespace cera
{
  namespace dxgi
  {
    class Factory;

    using AdapterVec = std::vector<std::shared_ptr<Adapter>>;

    class AdapterManager
    {
    public:
      AdapterManager(Factory* factory, const renderer::GpuScorerFn& scorer);
      AdapterManager(const AdapterManager&) = delete;
      AdapterManager(AdapterManager&&)      = delete;
      ~AdapterManager() = default;

      AdapterManager& operator=(const AdapterManager&) = delete;
      AdapterManager& operator=(AdapterManager&&)      = delete;

      bool load_adapters(Factory* factory);

      std::shared_ptr<Adapter> selected() const;
      std::shared_ptr<Adapter> first() const;
      const AdapterVec& all() const;

    private:
      std::shared_ptr<Adapter> m_selected_adapter;
      AdapterVec m_adapters;
    };
  } // namespace dxgi
} // namespace cera