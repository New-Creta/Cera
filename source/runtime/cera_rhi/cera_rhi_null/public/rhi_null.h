#pragma once

#include "rhi.h"

namespace cera
{
    namespace renderer
    {
        class rhi_null : public rhi
        {
            void initialize() override;

            void shutdown() override;
        };
    } // namespace renderer
} // namespace cera