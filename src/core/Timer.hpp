#pragma once
#include <chrono>


class Timer 
{
public:
    using clock = std::chrono::high_resolution_clock;

    Timer() : m_prev(clock::now()) {}

    double Tick() 
    { 
        // returns delta seconds
        auto now = clock::now();
        std::chrono::duration<double> d = now - m_prev;
        m_prev = now;
        m_time += d.count();
        return d.count();
    }

    double Time() const 
    { 
        return m_time; 
    }

private:
    clock::time_point m_prev;
    double m_time = 0.0;
};