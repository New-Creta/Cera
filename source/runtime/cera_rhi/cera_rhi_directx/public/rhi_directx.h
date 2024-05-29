#pragma once

#include "rhi.h"

namespace cera
{
    namespace renderer
    {
        class adapter;
        class d3d12_device;

        class rhi_directx : public rhi
        {
          public:
            static rhi_directx* instance();

          public:
            rhi_directx(const std::shared_ptr<adapter>& adapter);

            void initialize() override;
            void shutdown() override;

            adapter* get_adapter();
            const adapter* get_adapter() const;
            d3d12_device* get_device();
            const d3d12_device* get_device() const;

          private:
            // There should only be one!
            static rhi_directx* s_instance;

            std::shared_ptr<adapter> m_adapter;
            std::shared_ptr<d3d12_device> m_root_device;
        };
    } // namespace renderer
} // namespace cera