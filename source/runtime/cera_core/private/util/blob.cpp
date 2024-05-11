#include "util/blob.h"
#include "util/blob_writer.h"
#include "util/assert.h"
#include "util/log.h"

#include <memory>

namespace cera
{
  namespace memory
  {
    //-------------------------------------------------------------------------
    void Blob::copy(const Blob& src, Blob& dst)
    {
      if(src.size() == 0)
      {
        log::warn("size of src == 0, nothing to copy ...");

        return;
      }

      CERA_ASSERT_X(dst.size() != 0, "dst blob size is 0");
      CERA_ASSERT_X(dst.size() >= src.size(), "dst size is smaller than src size");

      std::memcpy(dst.m_data.get(), src.m_data.get(), src.size());
      std::memcpy(&dst.m_data_size, &src.m_data_size, sizeof(memory_size));
    }
    //-------------------------------------------------------------------------
    void Blob::copy(const Blob& src, void* dst)
    {
      if(src.size() == 0)
      {
        log::warn("size of src == 0, nothing to copy ...");

        return;
      }

      CERA_ASSERT_X(dst != nullptr, "dst is nullptr");

      std::memcpy(dst, src.m_data.get(), src.size());
    }
    //-------------------------------------------------------------------------
    void Blob::copy(void* src, const memory_size& size, Blob& dst)
    {
      CERA_ASSERT_X(src != nullptr, "src is nullptr");
      CERA_ASSERT_X(size != 0, "size is 0");

      CERA_ASSERT_X(dst.size() != 0, "dst size is 0");
      CERA_ASSERT_X(dst.size() >= size, "dst size is not big enough to receive copy");

      std::memcpy(dst.m_data.get(), src, size);
      std::memcpy(&dst.m_data_size, &size, sizeof(memory_size));
    }

    //-------------------------------------------------------------------------
    Blob::Blob()
        : m_data(nullptr)
        , m_data_size(0_bytes)
    {
    }

    //-------------------------------------------------------------------------
    Blob::Blob(Blob&& other) noexcept 
        : m_data(std::exchange(other.m_data, nullptr))
        , m_data_size(std::exchange(other.m_data_size, 0_bytes))
    {
    }
    //-------------------------------------------------------------------------
    Blob::Blob(std::unique_ptr<std::byte[]> data, const memory_size& size)
        : m_data(std::exchange(data, nullptr))
        , m_data_size(size)
    {
    }

    //-------------------------------------------------------------------------
    Blob::~Blob() = default;

    //-------------------------------------------------------------------------
    Blob& Blob::operator=(Blob&& other) noexcept
    {
      // Guard self assignment
      CERA_ASSERT_X(this != &other, "can't move to yourself");

      m_data = std::exchange(other.m_data, nullptr);
      m_data_size = std::exchange(other.m_data_size, 0_bytes);

      return *this;
    }

    //-------------------------------------------------------------------------
    Blob::operator bool() const
    {
      return m_data.get() != nullptr;
    }

    //-------------------------------------------------------------------------
    std::byte& Blob::operator[](s32 index)
    {
      return m_data.get()[index];
    }
    //-------------------------------------------------------------------------
    const std::byte& Blob::operator[](s32 index) const
    {
      return (static_cast<const std::byte*>(m_data.get()))[index];
    }

    //-------------------------------------------------------------------------
    void Blob::allocate(const memory_size& inSize)
    {
      release();

      if(inSize == 0)
      {
        log::warn("allocation of size equal to 0");

        return;
      }

      m_data = std::make_unique<std::byte[]>(static_cast<s32>(inSize)); // NOLINT(modernize-avoid-c-arrays)
      m_data_size = inSize;
    }
    //-------------------------------------------------------------------------
    void Blob::release()
    {
      m_data.reset();
      m_data_size = 0_bytes;
    }
    //-------------------------------------------------------------------------
    void Blob::zero_initialize()
    {
      if(m_data)
      {
        std::memset(m_data.get(), 0, size());
      }
    }

    //-------------------------------------------------------------------------
    std::byte* Blob::read_bytes(std::byte* dst, const memory_size& inSize, const memory_size& inOffset)
    {
      CERA_ASSERT_X(inOffset + inSize <= size(), "amount to read out of bounds");

      std::memcpy(dst, static_cast<std::byte*>(m_data.get()) + inOffset, inSize);
      return dst;
    }

    //-------------------------------------------------------------------------
    const std::byte* Blob::read_bytes(std::byte* dst, const memory_size& inSize, const memory_size& inOffset) const
    {
      CERA_ASSERT_X(inOffset + inSize <= size(), "amount of read out of bounds");

      std::memcpy(dst, static_cast<const std::byte*>(m_data.get()) + inOffset, inSize);
      return dst;
    }

    //-------------------------------------------------------------------------
    void Blob::write(const void* inData, const memory_size& inSize, const memory_size& inOffset)
    {
      CERA_ASSERT_X(inOffset + inSize <= size(), "amount for write out of bounds");

      std::memcpy(static_cast<std::byte*>(m_data.get()) + inOffset, inData, inSize);
    }

    //-------------------------------------------------------------------------
    std::byte* Blob::data()
    {
      return m_data.get();
    }
    //-------------------------------------------------------------------------
    const std::byte* Blob::data() const
    {
      return m_data.get();
    }

    //-------------------------------------------------------------------------
    memory_size Blob::size() const
    {
      return m_data_size;
    }

    //-------------------------------------------------------------------------
    Blob make_blob(const std::byte* inData, const memory_size& inSize)
    {
      Blob blob;

      blob.allocate(inSize);
      blob.zero_initialize();

      if(inData != nullptr)
      {
        writer::write(blob, inData, inSize);
      }

      return blob;
    }
  } // namespace memory
} // namespace cera