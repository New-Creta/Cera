/***************************************************************************
* These functions were taken from the MiniEngine.
* Source code available here:
* https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Math/Common.h
* Retrieved: January 13, 2016
**************************************************************************/

namespace cera
{
    namespace memory
    {
        template <typename T>
        inline T align_up_with_mask(T value, size_t mask)
        {
            return (T)(((size_t)value + mask) & ~mask);
        }

        template <typename T>
        inline T align_down_with_mask(T value, size_t mask)
        {
            return (T)((size_t)value & ~mask);
        }

        template <typename T>
        inline T align_up(T value, size_t alignment)
        {
            return align_up_with_mask(value, alignment - 1);
        }

        template <typename T>
        inline T align_down(T value, size_t alignment)
        {
            return align_down_with_mask(value, alignment - 1);
        }

        template <typename T>
        inline bool isaligned(T value, size_t alignment)
        {
            return 0 == ((size_t)value & (alignment - 1));
        }

        template <typename T>
        inline T divide_by_multiple(T value, size_t alignment)
        {
            return (T)((value + alignment - 1) / alignment);
        }
    }
}