#include "util/blob_reader.h"

namespace cera
{
  namespace memory
  {
    //-------------------------------------------------------------------------
    BlobReader::BlobReader(const memory::Blob& b)
        : BlobReader(b, 0_bytes)
    {
    }
    //-------------------------------------------------------------------------
    BlobReader::BlobReader(const memory::Blob& b, const memory_size& offset)
        : m_blob(&b)
        , m_read_offset(offset)
    {
    }

    //-------------------------------------------------------------------------
    std::byte* BlobReader::read(const memory_size& bytesToRead)
    {
      std::byte* dst = new std::byte[bytesToRead];

      m_blob->read_bytes(dst, bytesToRead, m_read_offset);
      m_read_offset += bytesToRead;

      return dst;
    }

    namespace reader
    {
      //-------------------------------------------------------------------------
      std::byte* read(const memory::Blob& b, const memory_size& bytesToRead, const memory_size& offset)
      {
        BlobReader reader(b, offset);
        return reader.read(bytesToRead);
      }
    } // namespace reader
  }   // namespace memory
} // namespace cera