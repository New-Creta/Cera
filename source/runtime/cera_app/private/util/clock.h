#pragma once

#include "util/types.h"

#include <chrono>

namespace cera
{
    class clock
    {
    public:
        clock();

        // Tick the high resolution clock.
        // Tick the clock before reading the delta time for the first time.
        // Only tick the clock once per frame.
        // Use the Get* functions to return the elapsed time between ticks.
        void update();

        // Reset the clock.
        void reset();

        double get_delta_nano_seconds() const;
        double get_delta_micro_seconds() const;
        double get_delta_milli_seconds() const;
        double get_delta_seconds() const;

        double get_total_nano_seconds() const;
        double get_total_micro_seconds() const;
        double get_total_milli_seconds() const;
        double get_total_seconds() const;

    private:
        // Initial time point.
        std::chrono::high_resolution_clock::time_point m_T0;

        // Time since last tick.
        std::chrono::high_resolution_clock::duration m_delta_time;
        std::chrono::high_resolution_clock::duration m_total_time;
    };
}