#pragma once

#include "util/assert.h"
#include "util/types.h"
#include <atomic>
#include <functional>

namespace cera
{
  namespace renderer
  {
    class ResourceSlot
    {
      public:
        static const s32 invalid_slot_id = -1;
    public:
      static ResourceSlot make_invalid();

    public:
      ResourceSlot();
      ResourceSlot(const ResourceSlot& other);
      ResourceSlot(ResourceSlot&& other) noexcept;
      ~ResourceSlot();

      explicit ResourceSlot(s32 slotId);

    public:
      ResourceSlot& operator=(const ResourceSlot& other);
      ResourceSlot& operator=(ResourceSlot&& other) noexcept;

      bool operator==(const ResourceSlot& other) const;
      bool operator!=(const ResourceSlot& other) const;
      bool operator==(s32 other) const;
      bool operator!=(s32 other) const;

    public:
      bool is_valid() const;

      s32 release();
      s32 slot_id() const;

    private:
      s32 m_slot_id;
      s32* m_ref_count;
    };

  } // namespace renderer
} // namespace cera

namespace std
{
    template <>
    struct hash<cera::renderer::ResourceSlot>
    {
      size_t operator()(const cera::renderer::ResourceSlot& resourceSlot) const
      {
        return resourceSlot.slot_id();
      }
    };
} // namespace rsl