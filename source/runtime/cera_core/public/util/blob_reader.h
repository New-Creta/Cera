#pragma once

#include "util/blob.h"
#include "util/memory_size.h"

#include <memory>

namespace cera
{
  namespace memory
  {
    class BlobReader
    {
    public:
      explicit BlobReader(const memory::Blob& b);
      BlobReader(const memory::Blob& b, const memory_size& offset);

      template <typename T>
      T read();

      std::byte* read(const memory_size& bytesToRead);

    private:
      const memory::Blob* m_blob;
      memory_size m_read_offset;
    };

    //-------------------------------------------------------------------------
    template <typename T>
    T memory::BlobReader::read()
    {
      T value = m_blob->read<T>(m_read_offset);
      m_read_offset += sizeof(T);
      return value;
    }

    namespace reader
    {
      //-------------------------------------------------------------------------
      template <typename T>
      T read(const memory::Blob& b)
      {
        BlobReader reader(b);
        return reader.read<T>();
      }
      //-------------------------------------------------------------------------
      template <typename T>
      T read(const memory::Blob& b, const memory_size& offset)
      {
        BlobReader reader(b, offset);
        return reader.read<T>();
      }

      //-------------------------------------------------------------------------
      std::byte* read(const memory::Blob& b, const memory_size& bytesToRead, const memory_size& offset);
    } // namespace reader
  }   // namespace memory
} // namespace cera