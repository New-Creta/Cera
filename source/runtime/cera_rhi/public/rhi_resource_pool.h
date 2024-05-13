#pragma once

#include "util/assert.h"
#include "util/types.h"

#include "rhi_resource.h"
#include "rhi_resource_slot.h"
#include "rhi_resource_slots.h"

#include <memory>
#include <mutex>
#include <unordered_map>

namespace cera
{
  namespace renderer
  {
    using ResourcePtr = std::shared_ptr<rhi_resource>;

    class ResourcePool
    {
    public:
      ResourcePool(s32 initialSlots = 16);

      void                      initialize(s32 reservedCapacity);
      void                      clear();

      ResourceSlot              allocate(const ResourcePtr& resource);

      void                      insert(const ResourceSlot& slot, const ResourcePtr& resource);
      void                      remove(const ResourceSlot& slot);

      bool                      has_slot(const ResourceSlot& slot) const;

      ResourcePtr&              at(const ResourceSlot& slot);
      const ResourcePtr&        at(const ResourceSlot& slot) const;

    public:
      template <typename U>
      bool                      is(const ResourceSlot& slot) const;

      template <typename U>
      std::shared_ptr<U>        as(const ResourceSlot& slot);
      template <typename U>
      const std::shared_ptr<U>  as(const ResourceSlot& slot) const;

    private:
      using ResourceMap = std::unordered_map<ResourceSlot, ResourcePtr>;

      void                      validate_and_grow_if_necessary(s32 minCapacity);
      
      ResourceSlots             m_resource_slots;
      ResourceMap               m_resource_map;

      std::recursive_mutex      m_lock;
    };

    //-----------------------------------------------------------------------
    template <typename U>
    bool ResourcePool::is(const ResourceSlot& slot) const
    {
      auto base = at(slot);

      return U::static_type() == base->type();
    }

    //-----------------------------------------------------------------------
    template <typename U>
    std::shared_ptr<U> ResourcePool::as(const ResourceSlot& slot)
    {
      CERA_ASSERT_X(has_slot(slot), "Slot was not registered within resource pool ({})", slot.slot_id());
      CERA_ASSERT_X(slot != ResourceSlot::invalid_slot_id, "Invalid index given to retrieve resource from resource pool");
      CERA_ASSERT_X(is<U>(slot), "Invalid type cast for given resource");

      std::shared_ptr<rhi_resource> base = at(slot);
      std::shared_ptr<U> derived      = std::static_pointer_cast<U>(base);

      return derived;
    }

    //-----------------------------------------------------------------------
    template <typename U>
    const std::shared_ptr<U> ResourcePool::as(const ResourceSlot& slot) const
    {
      CERA_ASSERT_X(has_slot(slot), "Slot was not registered within resource pool ({})", slot.slot_id());
      CERA_ASSERT_X(slot != ResourceSlot::invalid_slot_id, "Invalid index given to retrieve resource from resource pool");
      CERA_ASSERT_X(is<U>(slot), "Invalid type cast for given resource");

      const std::shared_ptr<rhi_resource>&    base    = at(slot);
      const std::shared_ptr<U>&            derived = std::static_pointer_cast<U>(base);

      return derived;
    }
  } // namespace renderer
} // namespace cera