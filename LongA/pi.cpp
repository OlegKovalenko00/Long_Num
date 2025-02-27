#include "head.hpp"
#include <iostream>
#include <string>
#include <iomanip>
#include <cmath>

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

    // Если задана точность 0, возвращаем приближение π = 3.
    if (precision == 0) {
        pi = pi + LongNumber(3.0, precision, false);
    }

    // Вычисляем сумму ряда по формуле BBP:
    // π = Σ (от k = 0 до precision-1) { 1/16^k * ( 4/(8*k+1) - 2/(8*k+4) - 1/(8*k+5) - 1/(8*k+6) ) }
    for (int k = 0; k < precision/4; ++k) {
        std::cout << k << std::endl;
        pi = pi + n0 * (a0 / a - b0 / b - c0 / c - d0 / d);
        n0 = n0 / n;
        a = a + eight;
        b = b + eight;
        c = c + eight;
        d = d + eight;
    }

    return pi;
}

int main(int argc, char *argv[])
{   
    int precision = ;
    LongNumber pi = calculate_pi(precision);
    std::cout << pi << std::endl;
    return 0;
}