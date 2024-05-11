#include "clock.h"

namespace cera
{
    clock::clock()
        : m_delta_time(0)
        , m_total_time(0)
    {
        m_T0 = std::chrono::high_resolution_clock::now();
    }

    void clock::update()
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        m_delta_time = t1 - m_T0;
        m_total_time += m_delta_time;
        m_T0 = t1;
    }

    void clock::reset()
    {
        m_T0 = std::chrono::high_resolution_clock::now();
        m_delta_time = std::chrono::high_resolution_clock::duration();
        m_total_time = std::chrono::high_resolution_clock::duration();
    }

    u64 clock::get_delta_nano_seconds() const
    {
        return m_delta_time.count() * 1.0;
    }
    u64 clock::get_delta_micro_seconds() const
    {
        return m_delta_time.count() * 1e-3;
    }

    u64 clock::get_delta_milli_seconds() const
    {
        return m_delta_time.count() * 1e-6;
    }

    u64 clock::get_delta_seconds() const
    {
        return m_delta_time.count() * 1e-9;
    }

    u64 clock::get_total_nano_seconds() const
    {
        return m_total_time.count() * 1.0;
    }

    u64 clock::get_total_micro_seconds() const
    {
        return m_total_time.count() * 1e-3;
    }

    u64 clock::get_total_milli_seconds() const
    {
        return m_total_time.count() * 1e-6;
    }

    u64 clock::get_total_seconds() const
    {
        return m_total_time.count() * 1e-9;
    }
}