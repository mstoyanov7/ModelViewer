#pragma once
#include <chrono>

class Timer {
public:
    Timer() 
    { 
        Reset(); 
    }

    double Tick() 
    {
        using namespace std::chrono;
        auto now = clock::now();
        double dt = duration<double>(now - last_).count();
        last_ = now;
        return dt;
    }

    // Reset baseline to now
    void Reset() 
    { 
        last_ = clock::now(); 
    }

    double Elapsed() const 
    {
        using namespace std::chrono;
        return duration<double>(clock::now() - last_).count();
    }

private:
    using clock = std::chrono::high_resolution_clock;
    clock::time_point last_;
};