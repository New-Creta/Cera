#pragma once

#include "util/assert.h"

namespace cera
{
    namespace renderer
    {
        class d3d12_device;

        class d3d12_device_child
        {
          public:
            d3d12_device_child(d3d12_device* in_parent = nullptr) 
                : m_parent(in_parent)
            {
            }

            d3d12_device* get_parent_device() const
            {
                // If this fires an object was likely created with a default constructor i.e in an STL container
                // and is therefore an orphan
                CERA_ASSERT_X(m_parent != nullptr, "Device was not set");
                return m_parent;
            }

            d3d12_device* GetParentDevice_Unsafe() const
            {
                return m_parent;
            }

        private:
            d3d12_device* m_parent;
        };
    }
}