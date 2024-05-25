#include "common/rhi_resource_pool.h"

namespace cera
{
  namespace renderer
  {
    ResourcePool::ResourcePool(s32 initialSlots)
        : m_resource_slots(initialSlots)
    {

    }

    //-----------------------------------------------------------------------
    void ResourcePool::initialize(s32 reservedCapacity)
    {
      m_resource_map.reserve(reservedCapacity);
    }

    //-----------------------------------------------------------------------
    ResourceSlot ResourcePool::allocate(const ResourcePtr& resource)
    {
      ResourceSlot new_slot = m_resource_slots.alloc_slot();

      insert(new_slot, resource);
      
      return new_slot;
    }

    //-----------------------------------------------------------------------
    void ResourcePool::clear()
    {
      m_resource_slots.free_slots();

      std::unique_lock const sl(m_lock);
      m_resource_map.clear();
    }

    //-----------------------------------------------------------------------
    void ResourcePool::insert(const ResourceSlot& slot, const ResourcePtr& resource)
    {
      // The slots are all in numerical order ( see ResourceSlots.cpp )
      // This means if there is a slot ID incoming with a larger value as the currently allocated buckets within the hashmap
      // We have to grow rehash and allocate a larger number of buckets
      validate_and_grow_if_necessary(slot.slot_id());

      std::unique_lock const sl(m_lock);
      m_resource_map[slot] = std::move(resource);
    }

    //-----------------------------------------------------------------------
    void ResourcePool::remove(const ResourceSlot& slot)
    {
      CERA_ASSERT_X(has_slot(slot), "Slot was not registered within resource pool ({})", slot.slot_id());

      m_resource_slots.free_slot(slot.slot_id());

      std::unique_lock const sl(m_lock);
      m_resource_map.erase(slot);
    }

    //-----------------------------------------------------------------------
    bool ResourcePool::has_slot(const ResourceSlot& slot) const
    {
      return m_resource_map.find(slot) != m_resource_map.cend();
    }

    //-----------------------------------------------------------------------
    ResourcePtr& ResourcePool::at(const ResourceSlot& slot)
    {
      CERA_ASSERT_X(has_slot(slot), "Slot was not registered within resource pool ({})", slot.slot_id());
      return m_resource_map.at(slot);
    }

    //-----------------------------------------------------------------------
    const ResourcePtr& ResourcePool::at(const ResourceSlot& slot) const
    {
      CERA_ASSERT_X(has_slot(slot), "Slot was not registered within resource pool ({})", slot.slot_id());
      return m_resource_map.at(slot);
    }

    //-----------------------------------------------------------------------
    void ResourcePool::validate_and_grow_if_necessary(s32 minCapacity)
    {
      if(minCapacity >= m_resource_map.size())
      {
        const s32 new_cap = (minCapacity * 2) + 1; // Grow to accommodate the slot

        std::unique_lock const sl(m_lock);
        m_resource_map.reserve(new_cap);
      }
    }
  } // namespace renderer
} // namespace cera