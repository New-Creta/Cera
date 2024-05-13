#include "dxgi/objects/factory.h"
#include "dxgi/dxgi_util.h"

#include "util/assert.h"

#include <memory>

namespace
{
    //-------------------------------------------------------------------------
    template <typename DXGIFactoryInterface>
    cera::wrl::ComPtr<DXGIFactoryInterface> create_dxgi_factory(s32 flags)
    {
        cera::wrl::ComPtr<DXGIFactoryInterface> dxgi_factory = nullptr;

        const HRESULT hr = CreateDXGIFactory2(flags, IID_PPV_ARGS(dxgi_factory.GetAddressOf()));
        if (hr != S_OK)
            return nullptr;

        return dxgi_factory;
    }

    //-------------------------------------------------------------------------
    template <typename DXGIFactoryInterface>
    cera::wrl::ComPtr<DXGIFactoryInterface> create_dxgi_factory()
    {
        cera::wrl::ComPtr<DXGIFactoryInterface> dxgi_factory = nullptr;

        const HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(dxgi_factory.GetAddressOf()));
        if (hr != S_OK)
            return nullptr;

        return dxgi_factory;
    }

    //-------------------------------------------------------------------------
    cera::wrl::ComPtr<IDXGIFactory> create_dxgi_factory()
    {
        cera::wrl::ComPtr<IDXGIFactory> dxgi_factory;

        const HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(dxgi_factory.GetAddressOf()));
        if (hr != S_OK)
            return nullptr;

        return dxgi_factory;
    }
} // namespace

namespace cera
{
    namespace dxgi
    {
        namespace adaptors
        {
            class MakeFactory : public Factory
            {
            public:
              MakeFactory()
                  : Factory()
              {
              }
              MakeFactory(wrl::ComPtr<IDXGIFactory>&& factory)
                  : Factory(std::move(factory))
              {
              }
            };
        }

        //-------------------------------------------------------------------------
        std::unique_ptr<Factory> Factory::create(u32 flags)
        {
            if (wrl::ComPtr<IDXGIFactory> factory = create_dxgi_factory<IDXGIFactory7>(flags))
                return std::make_unique<adaptors::MakeFactory>(std::move(factory));
            if (wrl::ComPtr<IDXGIFactory> factory = create_dxgi_factory<IDXGIFactory6>(flags))
                return std::make_unique<adaptors::MakeFactory>(std::move(factory));
            if (wrl::ComPtr<IDXGIFactory> factory = create_dxgi_factory<IDXGIFactory5>(flags))
                return std::make_unique<adaptors::MakeFactory>(std::move(factory));
            if (wrl::ComPtr<IDXGIFactory> factory = create_dxgi_factory<IDXGIFactory4>(flags))
                return std::make_unique<adaptors::MakeFactory>(std::move(factory));
            if (wrl::ComPtr<IDXGIFactory> factory = create_dxgi_factory<IDXGIFactory3>(flags))
                return std::make_unique<adaptors::MakeFactory>(std::move(factory));
            if (wrl::ComPtr<IDXGIFactory> factory = create_dxgi_factory<IDXGIFactory2>(flags))
                return std::make_unique<adaptors::MakeFactory>(std::move(factory));
            if (wrl::ComPtr<IDXGIFactory> factory = create_dxgi_factory<IDXGIFactory1>())
                return std::make_unique<adaptors::MakeFactory>(std::move(factory));
            if (wrl::ComPtr<IDXGIFactory> factory = create_dxgi_factory())
                return std::make_unique<adaptors::MakeFactory>(std::move(factory));

            CERA_ASSERT("Couldn't create dxgi factory!");
            return nullptr;
        }

        Factory::Factory()
          : ComObject(nullptr)
        {}

        //-------------------------------------------------------------------------
        Factory::Factory(wrl::ComPtr<IDXGIFactory>&& object)
            : ComObject(std::move(object))
        {}
    } // namespace dxgi
} // namespace cera