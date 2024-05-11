#pragma once

#include "util/blob.h" // IWYU pragma: keep
#include "util/memory_size.h"
#include "util/types.h"

namespace cera
{
  namespace memory
  {
    class BlobWriter
    {
    public:
      explicit BlobWriter(memory::Blob& b);
      BlobWriter(memory::Blob& b, s64 offset);

      template <typename T>
      void write(const T& data);
      void write(const void* inData, const memory_size& inSize);

      s64 write_offset() const;

    private:
      memory::Blob* m_blob;
      memory_size m_write_offset;
    };

    //-------------------------------------------------------------------------
    template <typename T>
    void rex::memory::BlobWriter::write(const T& data)
    {
      m_blob->write(&data, sizeof(T), m_write_offset);
      m_write_offset += sizeof(T);
    }

    namespace writer
    {
      //-------------------------------------------------------------------------
      template <typename T>
      void write(memory::Blob& b, const T& data)
      {
        BlobWriter writer(b);

        writer.write<T>(data);
      }

      void write(memory::Blob& b, const void* inData, const memory_size& inSize);
      void write(memory::Blob& b, const void* inData, const memory_size& inSize, const memory_size& inOffset);
    } // namespace writer
  }   // namespace memory
} // namespace cera