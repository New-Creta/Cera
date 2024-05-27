#pragma once

#include "dxgi/objects/adapter.h"
#include "gpu_scorer.h"

#include <vector>

namespace cera
{
  namespace dxgi
  {
    using adapter_vec = std::vector<std::shared_ptr<adapter>>;

    class adapter_manager
    {
    public:
      adapter_manager(const renderer::adapter_scorer_fn& scorer, s32 gpu_preference = -1);
      adapter_manager(const adapter_manager&) = delete;
      adapter_manager(adapter_manager&&)      = delete;
      ~adapter_manager() = default;

      adapter_manager& operator=(const adapter_manager&) = delete;
      adapter_manager& operator=(adapter_manager&&)      = delete;

      std::shared_ptr<adapter> selected() const;
      std::shared_ptr<adapter> first() const;
      
      const adapter_vec& all() const;

    private:
      bool load_adapters(s32 gpu_preference);

      std::shared_ptr<adapter> m_selected_adapter;
      adapter_vec m_adapters;
    };
  } // namespace dxgi
} // namespace cera