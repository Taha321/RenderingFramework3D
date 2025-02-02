#pragma once

#include <string>
#include <chrono>
#include <iostream>


namespace RenderingFramework3D {
class TimeProfiler {
public:
    void Start() {
        _last_check = 0;
        _start = std::chrono::steady_clock::now();
    }

    float Check(double div=1) {
        Check("", div, false);
        return _last_check;
    }

    void Check(const std::string& prompt, double div = 1, bool print=true) {
        const auto elapsed = std::chrono::steady_clock::now() - _start;
        const auto elapsed_seconds = std::chrono::duration<double>(elapsed);
        _last_check = elapsed_seconds.count();
        if(print) std::cout << prompt << ": " << (elapsed_seconds.count() / div) << " s\n";
    }

private:
    std::chrono::steady_clock::time_point _start;
    float _last_check;
};
}