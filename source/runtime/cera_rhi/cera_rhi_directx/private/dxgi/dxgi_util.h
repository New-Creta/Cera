#pragma once

// NOLINTBEGIN(llvm-include-order)
// clang-format off

#include <dxgi1_6.h> // IWYU pragma: keep

#include <dxgicommon.h> // IWYU pragma: keep
#include <dxgidebug.h> // IWYU pragma: keep

namespace cera
{
    namespace dxgi
    {
        namespace helpers
        {
            void safe_create_dxgi_factory(IDXGIFactory4** DXGIFactory);
        }
    }
}

// clang-format on
// NOLINTEND(llvm-include-order)
