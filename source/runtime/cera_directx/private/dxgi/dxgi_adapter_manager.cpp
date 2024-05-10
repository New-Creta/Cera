#include "dxgi/dxgi_adapter_manager.h"
#include "dxgi/objects/adapter.h" // IWYU pragma: keep
#include "dxgi/objects/factory.h"
#include "dxgi/dxgi_util.h"

#include "util/assert.h"

#include "gpu_description.h"

#include <functional>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace
{
  //-------------------------------------------------------------------------
  std::function<HRESULT(UINT, cera::wrl::ComPtr<IDXGIAdapter4>*)> get_enumaration_function(cera::dxgi::Factory* factory)
  {
    cera::wrl::ComPtr<IDXGIFactory6> factory_6 = factory->as<IDXGIFactory6>(); // NOLINT(misc-const-correctness)

    REX_ASSERT_X(factory_6, "IDXGIFactory6 does not exist!");

    return [factory = factory_6.Get()](UINT index, cera::wrl::ComPtr<IDXGIAdapter4>* adapter) { return factory->EnumAdapterByGpuPreference(index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS((*adapter).GetAddressOf())); };
  }

  //-------------------------------------------------------------------------
  std::vector<std::shared_ptr<cera::dxgi::Adapter>> get_adapters(const std::function<HRESULT(UINT, cera::wrl::ComPtr<IDXGIAdapter4>*)>& enumarationFnc)
  {
    uint32 i                                = 0;
    cera::wrl::ComPtr<IDXGIAdapter4> adapter = nullptr;

    std::vector<std::shared_ptr<cera::dxgi::Adapter>> adapters;
    while(enumarationFnc(i, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
      if(adapter)
      {
        adapters.emplace_back(std::make_shared<cera::dxgi::Adapter>(std::move(adapter)));
      }

      ++i;
    }

    return adapters;
  }
} // namespace

namespace cera
{
  namespace dxgi
  {
    //-------------------------------------------------------------------------
    AdapterManager::AdapterManager(Factory* factory, const renderer::GpuScorerFn& scorerFn)
        : m_selected_adapter(nullptr)
    {
      load_adapters(factory);

      REX_ASSERT_X(!m_adapters.empty(), "No adapters found");

      // this can be fixed once we have vector views/ranges
      std::vector<renderer::GpuDescription> gpus;
      gpus.reserve(m_adapters.size());
      for(const auto& adapter: m_adapters)
      {
        gpus.push_back(adapter->description());
      }
      const size_t selected_adapter_idx = scorerFn(gpus);
      if(selected_adapter_idx != -1)
      {
        m_selected_adapter = m_adapters[selected_adapter_idx];
      }
    }

    //-------------------------------------------------------------------------
    bool AdapterManager::load_adapters(Factory* factory)
    {
      m_adapters = get_adapters(get_enumaration_function(factory));

      return m_adapters.empty() == false; // NOLINT(readability-simplify-boolean-expr)
    }

    //-------------------------------------------------------------------------
    std::shared_ptr<Adapter> AdapterManager::selected() const
    {
      REX_ASSERT_X(m_selected_adapter, "No adapter selected. Call \" select(uint32 adapterID) \" first");

      return m_selected_adapter;
    }
    //-------------------------------------------------------------------------
    std::shared_ptr<Adapter> AdapterManager::first() const
    {
      REX_ASSERT_X(!m_adapters.empty(), "No adapters found");

      return m_adapters.front();
    }
    //-------------------------------------------------------------------------
    const std::vector<std::shared_ptr<Adapter>>& AdapterManager::all() const
    {
      REX_ASSERT_X(!m_adapters.empty(), "No adapters found");

      return m_adapters;
    }
  } // namespace dxgi
} // namespace cera