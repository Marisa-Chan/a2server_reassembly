#include "perf.h"

#include <windows.h>

// 599F90
Perf::Perf() {
    QueryPerformanceFrequency((LARGE_INTEGER*)&this->freq64);
    this->freq = (uint32_t)(this->freq64 / 1000);
    this->Begin();
}

// 599FD0
void Perf::Begin() {
    QueryPerformanceCounter((LARGE_INTEGER*)&this->begin);
}

// 5B6E50
void Perf::End() {
    QueryPerformanceCounter((LARGE_INTEGER*)&this->end);
    this->period = (uint32_t)(this->end - this->begin);
    this->cnt = this->period / this->freq;
    this->period64 = this->period;
    this->period64 *= 1000;
    this->period64 /= this->freq;
}
