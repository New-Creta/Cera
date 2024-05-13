#include "rhi_resource_slots.h"

#include "util/assert.h"
#include "util/log.h"

#if defined(CERA_BUILD_DEBUG) || defined(CERA_BUILD_DEBUG_OPT)
#define CERA_ENABLE_RESOURCE_SLOT_DEBUGGING
#endif

namespace cera
{
    namespace renderer
    {
        //-------------------------------------------------------------------------
        ResourceSlots::ResourceSlots(s32 num) : m_flag_capacity(0)
        {
            resize(num);
        }

        //-------------------------------------------------------------------------
        ResourceSlots::~ResourceSlots()
        {
            free_slots();

            m_flags.reset();
        }

        //-------------------------------------------------------------------------
        ResourceSlot ResourceSlots::alloc_slot()
        {
            for (s32 i = 0; i < m_flag_capacity; ++i)
            {
                if (!m_flags[i].test_and_set(std::memory_order_acquire))
                {
                    return ResourceSlot(i);
                }
            }

            log::warn("No available resource slots, resizing resource slots: {} => {}", m_flag_capacity,
                      m_flag_capacity * 2);

            const s32 current_flag_capacity =
                m_flag_capacity; // The capacity of the array will be changed after the next call.

            resize(m_flag_capacity * 2);

#ifdef CERA_ENABLE_RESOURCE_SLOT_DEBUGGING
            const bool result = m_flags[current_flag_capacity].test_and_set(rsl::memory_order_acquire);
            CERA_ASSERT_X(result == false, "Unable to allocated ResourceSlot at location: {}", current_flag_capacity);
            return ResourceSlot(current_flag_capacity);
#else
            return ResourceSlot(
                static_cast<s32>(m_flags[current_flag_capacity].test_and_set(std::memory_order_acquire)));
#endif
        }

        //-------------------------------------------------------------------------
        bool ResourceSlots::free_slot(s32 slot)
        {
            if (slot == ResourceSlot::invalid_slot_id)
            {
                log::warn("Trying to release an invalid resource from ResourceSlots");
                return true; // Slot is invalid
            }

            if (slot >= m_flag_capacity)
            {
                log::error("Trying to release a slot that is out of range of the allocated ResourceSlots");
                return false; // Slot is not in use or out of range.
            }

            if (m_flags[slot].test_and_set(std::memory_order_release))
            {
                return true;
            }

            log::error("Unable to test and set ResourceSlot");
            return false; // Unable to remove slot
        }

        //-------------------------------------------------------------------------
        void ResourceSlots::free_slots()
        {
            // Reset all atomic flags to indicate all slots are free
            for (s32 i = 0; i < m_flag_capacity; ++i)
            {
                m_flags[i].clear();
            }
        }

        //-------------------------------------------------------------------------
        void ResourceSlots::resize(s32 num)
        {
            CERA_ASSERT_X(num > 0, "At least 1 slot should be allocated");

            if (num == m_flag_capacity)
            {
                return; // No op
            }

            CERA_ASSERT_X(m_flag_capacity < num, "Shrinking the resource slots capacity is not supported");

            const s32 cur_cap = m_flag_capacity;
            const s32 new_cap = num;

            m_flags = std::make_unique<std::atomic_flag[]>(num);

            for (s32 i = 0; i < cur_cap; ++i)
            {
                m_flags[i].test_and_set(std::memory_order_acquire);
            }
            for (s32 i = cur_cap; i < new_cap; ++i)
            {
                m_flags[i].clear(); // Initialize new m_flags as clear (FREE)
            }

            m_flag_capacity = num;
        }

        //-------------------------------------------------------------------------
        ResourceSlots::Iterator ResourceSlots::begin()
        {
            size_t start = 0;
            while (start < m_flag_capacity)
            {
                bool flag_value = m_flags[start].test_and_set(std::memory_order_acquire);
                if (!flag_value)
                {
                    break;
                }
                ++start;
            }

            return Iterator(m_flags.get(), m_flag_capacity, start);
        }

        //-------------------------------------------------------------------------
        ResourceSlots::Iterator ResourceSlots::end()
        {
            return Iterator(m_flags.get(), m_flag_capacity, m_flag_capacity);
        }

        //-------------------------------------------------------------------------
        ResourceSlots::ConstIterator ResourceSlots::cbegin() const
        {
            size_t start = 0;
            while (start < m_flag_capacity)
            {
                bool flag_value = m_flags[start].test_and_set(std::memory_order_acquire);
                if (!flag_value)
                {
                    break;
                }
                ++start;
            }
            return ConstIterator(m_flags.get(), m_flag_capacity, start);
        }

        //-------------------------------------------------------------------------
        ResourceSlots::ConstIterator ResourceSlots::cend() const
        {
            return ConstIterator(m_flags.get(), m_flag_capacity, m_flag_capacity);
        }
    } // namespace renderer
} // namespace cera