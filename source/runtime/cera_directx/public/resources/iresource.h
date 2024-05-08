#pragma once

#define RESOURCE_CLASS_TYPE(resourceType)                                                                                                                                                                                                                \
  static size_t static_type()                                                                                                                                                                                                                            \
  {                                                                                                                                                                                                                                                      \
    return std::type_id<resourceType>().hash_code();                                                                                                                                                                                                     \
  }                                                                                                                                                                                                                                                      \
  size_t type() const override                                                                                                                                                                                                                           \
  {                                                                                                                                                                                                                                                      \
    return static_type();                                                                                                                                                                                                                                \
  }

namespace cera
{
    namespace renderer
    {
      class IResource
      {
      public:
        IResource()          = default;
        virtual ~IResource() = default;

        virtual size_t type() const = 0;
      };
    }
}