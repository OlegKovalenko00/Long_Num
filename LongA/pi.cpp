#include "head.hpp"
#include <iostream>
#include <string>
#include <iomanip>
#include <cmath>
#include <chrono>

LongNumber calculate_pi(int precision) {
    LongNumber pi(0.0, precision, false);
    LongNumber n0(1.0, precision, false);
    LongNumber n(16.0, precision, false);

    LongNumber a0(4.0, precision, false);
    LongNumber b0(2.0, precision, false);
    LongNumber c0(1.0, precision, false);
    LongNumber d0(1.0, precision, false);

    LongNumber a(1.0, precision, false);
    LongNumber b(4.0, precision, false);
    LongNumber c(5.0, precision, false);
    LongNumber d(6.0, precision, false);

    LongNumber eight(8.0, precision, false);

    if (precision == 0) {
        pi = pi + LongNumber(3.0, precision, false);
    }

    for (int k = 0; k < precision/4; ++k) {
        pi = pi + ((a0 / a - b0 / b - c0 / c - d0 / d) >> (4 * k));
        a = a + eight;
        b = b + eight;
        c = c + eight;
        d = d + eight;
    }

    return pi;
}

int main(int argc, char *argv[])
{   
    auto start_time = std::chrono::steady_clock::now();
    
    int precision = std::stoi(argv[1]) * 4;
    LongNumber pi = calculate_pi(precision);
    std::cout << pi << std::endl;

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Pi calculate in " << duration.count() << " ms\n";

    return 0;
}