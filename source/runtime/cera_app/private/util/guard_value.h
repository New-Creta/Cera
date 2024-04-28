#pragma once

namespace cera
{
    /**
     * exception-safe guard around saving/restoring a value.
     * Commonly used to make sure a value is restored
     * even if the code early outs in the future.
     * Usage:
     *  	guard_value<bool> GuardSomeBool(bSomeBool, false); // Sets bSomeBool to false, and restores it in dtor.
     */
    template <typename ref_type, typename assigned_type = ref_type>
    struct guard_value
    {
        guard_value(ref_type &reference_value, const assigned_type &new_value)
            : ref_value(reference_value), old_value(reference_value)
        {
            ref_value = new_value;
        }
        ~guard_value()
        {
            ref_value = old_value;
        }

        /**
         * Overloaded dereference operator.
         * Provides read-only access to the original value of the data being tracked by this struct
         *
         * @return	a const reference to the original data value
         */
        FORCEINLINE const assigned_type &operator*() const
        {
            return old_value;
        }

    protected:
        // ensure the class cannot be constructed directly
        guard_value() {}

    private:
        guard_value(const guard_value&) = default;
        guard_value& operator=(const guard_value&) = default;

        ref_type &ref_value;
        assigned_type old_value;
    };
}