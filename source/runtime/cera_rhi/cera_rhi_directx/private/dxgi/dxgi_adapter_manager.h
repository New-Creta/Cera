#pragma once

#include "dxgi/dxgi_adapter.h"

#include <vector>

namespace cera
{
  namespace renderer
  {
    using adapter_ptr = std::shared_ptr<adapter>;
    using adapter_vec = std::vector<adapter_ptr>;

    using adapter_scorer_fn = std::function<size_t(const std::vector<adapter_description>&)>;

    class adapter_manager
    {
    public:
      adapter_manager(const renderer::adapter_scorer_fn& scorer, s32 gpu_preference = -1);
      adapter_manager(const adapter_manager&) = delete;
      adapter_manager(adapter_manager&&)      = delete;
      ~adapter_manager() = default;

      adapter_manager& operator=(const adapter_manager&) = delete;
      adapter_manager& operator=(adapter_manager&&)      = delete;

      adapter_ptr selected() const;
      adapter_ptr first() const;
      
      const adapter_vec& all() const;

    private:
      bool load_adapters(s32 gpu_preference);

      adapter_ptr m_selected_adapter;
      adapter_vec m_adapters;
    };
  } // namespace renderer
} // namespace cera