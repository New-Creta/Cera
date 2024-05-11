#include "util/blob_view.h"

#include "util/blob.h"

namespace cera
{
  namespace memory
  {
    BlobView::BlobView()
        : m_data(nullptr)
        , m_size(0)
    {
    }

    BlobView::BlobView(const Blob& blob)
        : m_data(blob.data())
        , m_size(blob.size())
    {
    }

    const std::byte* BlobView::data() const
    {
      return m_data;
    }

    memory_size BlobView::size() const
    {
      return m_size;
    }
  } // namespace memory
} // namespace cera