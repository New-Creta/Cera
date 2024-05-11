#pragma once

#include "util/memory_size.h"
#include "util/types.h"

#include <memory>

namespace cera
{
  namespace memory
  {
    class Blob
    {
    public:
      static void copy(const Blob& src, Blob& dst);
      static void copy(const Blob& src, void* dst);
      static void copy(void* src, const memory_size& size, Blob& dst);

      Blob();
      Blob(const Blob& other) = delete;
      Blob(Blob&& other) noexcept;
      explicit Blob(std::unique_ptr<std::byte[]> data, const memory_size& size);
      ~Blob();

      Blob& operator=(const Blob& other) = delete;
      Blob& operator=(Blob&& other) noexcept;

      explicit operator bool() const;

      std::byte& operator[](s32 index);
      const std::byte& operator[](s32 index) const;

      void allocate(const memory_size& inSize);
      void release();
      void zero_initialize();

      std::byte* data();
      const std::byte* data() const;

      memory_size size() const;

      template <typename T>
      T* data_as();
      template <typename T>
      const T* data_as() const;

      template <typename T>
      T& read(const memory_size& offset = 0_bytes);
      template <typename T>
      const T& read(const memory_size& offset = 0_bytes) const;

    private:
      friend class BlobWriter;
      friend class BlobReader;

      std::byte* read_bytes(std::byte* dst, const memory_size& inSize, const memory_size& inOffset);
      const std::byte* read_bytes(std::byte* dst, const memory_size& inSize, const memory_size& inOffset) const;

      void write(const void* inData, const memory_size& inSize, const memory_size& inOffset = 0_bytes);

      std::unique_ptr<std::byte[]> m_data;
      memory_size m_data_size;
    };

    //-------------------------------------------------------------------------
    Blob make_blob(const std::byte* inData, const memory_size& inSize);
    //-------------------------------------------------------------------------
    template <typename T>
    Blob make_blob(const T* data, s32 num)
    {
      return make_blob(reinterpret_cast<const std::byte*>(data), memory_size(sizeof(T) * num));
    }

    //-------------------------------------------------------------------------
    template <typename T>
    T& Blob::read(const memory_size& offset /*= 0*/)
    {
      T* data = reinterpret_cast<T*>(m_data.get() + offset);
      return *data;
    }

    //-------------------------------------------------------------------------
    template <typename T>
    const T& Blob::read(const memory_size& offset) const
    {
      return *(T*)(m_data.get() + offset);
    }

    //-------------------------------------------------------------------------
    template <typename T>
    T* Blob::data_as()
    {
      return reinterpret_cast<T*>(m_data.get());
    }
    //-------------------------------------------------------------------------
    template <typename T>
    const T* Blob::data_as() const
    {
      return reinterpret_cast<const T*>(m_data.get());
    }
  } // namespace memory
} // namespace cera