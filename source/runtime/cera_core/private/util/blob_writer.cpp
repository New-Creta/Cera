#include "util/blob_writer.h"

namespace cera
{
  namespace memory
  {
    //-------------------------------------------------------------------------
    BlobWriter::BlobWriter(memory::Blob& b)
        : BlobWriter(b, 0_bytes)
    {
    }
    //-------------------------------------------------------------------------
    BlobWriter::BlobWriter(memory::Blob& b, s64 offset)
        : m_blob(&b)
        , m_write_offset(offset)
    {
    }

    //-------------------------------------------------------------------------
    void BlobWriter::write(const void* inData, const memory_size& inSize)
    {
      m_blob->write(inData, inSize, m_write_offset);
      m_write_offset += inSize;
    }

    //-------------------------------------------------------------------------
    s64 BlobWriter::write_offset() const
    {
      return m_write_offset;
    }

    namespace writer
    {
      //-------------------------------------------------------------------------
      void write(memory::Blob& b, const void* inData, const memory_size& inSize)
      {
        BlobWriter writer(b);
        writer.write(inData, inSize);
      }

      //-------------------------------------------------------------------------
      void write(memory::Blob& b, const void* inData, const memory_size& inSize, const memory_size& inOffset)
      {
        BlobWriter writer(b, inOffset);
        writer.write(inData, inSize);
      }
    } // namespace writer
  }   // namespace memory
} // namespace cera