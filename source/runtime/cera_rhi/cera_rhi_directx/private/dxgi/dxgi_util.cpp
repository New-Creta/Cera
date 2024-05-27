#include "dxgi/dxgi_util.h"

namespace cera
{
    namespace dxgi
    {
        namespace helpers
        {
            /**
             * Since CreateDXGIFactory is a delay loaded import from the DXGI DLL, if the user
             * doesn't have Vista/DX10, calling CreateDXGIFactory will throw an exception.
             * We could use SEH to detect that case and fail gracefully.
             * 
             * However, we don't support anything older than Windows 10 anyway so it is fine to assume this succeeds.
             */
            void safe_create_dxgi_factory(IDXGIFactory4** in_factory)
            {
                CreateDXGIFactory(__uuidof(IDXGIFactory4), (void**)in_factory);
            }
        }
    }
}