#pragma once

#include "resource_descriptions/create_clear_state_desc.h"
#include "rhi_resource.h"

namespace cera
{
    namespace renderer
    {
        class ClearState : public rhi_resource
        {
        public:
          RESOURCE_CLASS_TYPE(ClearState);

          virtual ~ClearState();

          Color4f       rgba()  const;
          f32           depth() const;
          u8            stencil() const;
          ClearBits     flags() const;

        protected:
          ClearState();
          ClearState(CreateClearStateDesc&& desc);

        private:
          CreateClearStateDesc m_desc;
        };
    }
}