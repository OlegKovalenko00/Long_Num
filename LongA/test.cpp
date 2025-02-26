// test.cpp
#include "head.hpp"
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <cmath>
#include <chrono>

// Функция преобразования LongNumber в long double для приблизительного сравнения.
long double longNumberToLongDouble(const LongNumber &num) {
    const std::vector<bool>& bits = num.get_bit_vector();
    int frac = num.get_precision();
    int int_len = bits.size() - frac;
    long double value = 0.0L;
    for (int i = 0; i < int_len; i++) {
        value = value * 2.0L + (bits[i] ? 1.0L : 0.0L);
    }
    long double factor = 0.5L;
    for (int i = int_len; i < (int)bits.size(); i++) {
        if (bits[i])
            value += factor;
        factor /= 2.0L;
    }
    return num.isNegative() ? -value : value;
}

// Функция сравнения с допуском.
bool approxEqual(const LongNumber &a, const LongNumber &b) {
    long double tol = std::pow(10.L, -a.get_precision());
    return std::fabs(longNumberToLongDouble(a) - longNumberToLongDouble(b)) < tol;
}

// Для наглядного вывода.
void print_long_number(const LongNumber &num) {
    LongNumber copy = num;
    copy.printk_binary(copy.get_bit_vector(), copy.get_precision(), copy.isNegative());
    std::cout << " (" << longNumberToLongDouble(num) << ")" << std::endl;
}

bool test_pi() {
    using namespace std::chrono;
    const int precision = 20*1024;
    auto start_time = high_resolution_clock::now();
    LongNumber pi_calculated = LongNumber::calculate_pi(precision);
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end_time - start_time);
    std::cout << "Time taken for Pi calculation: " 
              << duration.count() << " microseconds" << std::endl;
    LongNumber pi_expected("3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679", precision);
    if (!approxEqual(pi_calculated, pi_expected)) {
        std::cout << "Pi test failed." << std::endl;
        std::cout << "Expected: ";
        print_long_number(pi_expected);
        std::cout << "Result:   ";
        print_long_number(pi_calculated);
        return false;
    }
    return true;
}

bool test_addition() {
    const int precision = 30;
    LongNumber a("44.38777228889", precision);
    LongNumber b("98.232342323245", precision);
    LongNumber expected("142.620114612135", precision);
    LongNumber result = a + b;
    if (!approxEqual(result, expected)) {
        std::cout << "Addition test failed." << std::endl;
        std::cout << "Expected: ";
        print_long_number(expected);
        std::cout << "Result:   ";
        print_long_number(result);
        return false;
    }
    return true;
}

bool test_subtraction() {
    const int precision = 10;
    LongNumber a("123.456789", precision);
    LongNumber b("45.678901", precision);
    LongNumber expected("77.777888", precision);
    LongNumber result = a - b;
    if (!approxEqual(result, expected)) {
        std::cout << "Subtraction test failed." << std::endl;
        std::cout << "Expected: ";
        print_long_number(expected);
        std::cout << "Result:   ";
        print_long_number(result);
        return false;
    }
    return true;
}

bool test_multiplication() {
    const int precision = 100;
    LongNumber a("3.12", precision);
    LongNumber b("2.234", precision);
    LongNumber expected("6.97008", precision);
    LongNumber result = a * b;
    if (!approxEqual(result, expected)) {
        std::cout << "Multiplication test failed." << std::endl;
        std::cout << "Expected: ";
        print_long_number(expected);
        std::cout << "Result:   ";
        print_long_number(result);
        return false;
    }
    return true;
}

bool test_division() {
    const int precision = 15;
    LongNumber a("20.3", precision);
    LongNumber b("10.13", precision);
    LongNumber expected("2.003948667324778", precision);
    try {
        LongNumber result = a / b;
        if (!approxEqual(result, expected)) {
            std::cout << "Division test failed." << std::endl;
            std::cout << "Expected: ";
            print_long_number(expected);
            std::cout << "Result:   ";
            print_long_number(result);
            return false;
        }
    } catch (const std::runtime_error &e) {
        std::cout << "Division test threw exception: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool test_string_constructor() {
    const int precision = 10;
    LongNumber from_str("123.456789", precision);
    LongNumber from_double(123.456789, precision, false);
    if (!approxEqual(from_str, from_double)) {
        std::cout << "String constructor test failed." << std::endl;
        std::cout << "From String: ";
        print_long_number(from_str);
        std::cout << "From Double: ";
        print_long_number(from_double);
        return false;
    }
    return true;
}

bool test_comparisons() {
    const int precision = 10;
    LongNumber a("123.456789", precision);
    LongNumber b("123.456789", precision);
    LongNumber c("98.7654321", precision);
    LongNumber d("234.567891", precision);
    
    if (!(a == b)) {
        std::cout << "Comparison '==' failed" << std::endl;
        return false;
    }
    if (a == c) {
        std::cout << "Comparison '!=' failed" << std::endl;
        return false;
    }
    if (!(a > c)) {
        std::cout << "Comparison '>' failed" << std::endl;
        return false;
    }
    if (!(c < a)) {
        std::cout << "Comparison '<' failed" << std::endl;
        return false;
    }
    if (!(d >= a)) {
        std::cout << "Comparison '>=' failed" << std::endl;
        return false;
    }
    if (!(c <= a)) {
        std::cout << "Comparison '<=' failed" << std::endl;
        return false;
    }
    return true;
}

bool test_negative_addition() {
    const int precision = 10;
    LongNumber a("-123.456789", precision);
    LongNumber b("-876.543211", precision);
    LongNumber expected("-1000.000000", precision);
    LongNumber result = a + b;
    if (!approxEqual(result, expected)) {
        std::cout << "Negative Addition test failed." << std::endl;
        std::cout << "Expected: ";
        print_long_number(expected);
        std::cout << "Result:   ";
        print_long_number(result);
        return false;
    }
    return true;
}

bool test_mixed_subtraction() {
    const int precision = 30;
    LongNumber a("500.1234567890", precision);
    LongNumber b("-250.9876543210", precision);
    LongNumber expected("751.1111111100", precision);
    LongNumber result = a - b;
    if (!approxEqual(result, expected)) {
        std::cout << "Mixed Subtraction test failed." << std::endl;
        std::cout << "Expected: ";
        print_long_number(expected);
        std::cout << "Result:   ";
        print_long_number(result);
        return false;
    }
    return true;
}

bool test_negative_multiplication() {
    const int precision = 10;
    LongNumber a("-3.5", precision);
    LongNumber b("2", precision);
    LongNumber expected("-7.0000000000", precision);
    LongNumber result = a * b;
    if (!approxEqual(result, expected)) {
        std::cout << "Negative Multiplication test failed." << std::endl;
        std::cout << "Expected: ";
        print_long_number(expected);
        std::cout << "Result:   ";
        print_long_number(result);
        return false;
    }
    return true;
}

bool test_negative_division() {
    const int precision = 10;
    LongNumber a("-20", precision);
    LongNumber b("4", precision);
    LongNumber expected("-5", precision);
    LongNumber result = a / b;
    if (!approxEqual(result, expected)) {
        std::cout << "Negative Division test failed." << std::endl;
        std::cout << "Expected: ";
        print_long_number(expected);
        std::cout << "Result:   ";
        print_long_number(result);
        return false;
    }
    return true;
}


bool test_large_numbers() {
    const int precision = 10;
    LongNumber a("12345678901234567890.1234567890", precision);
    LongNumber b("98765432109876543210.9876543210", precision);
    LongNumber expected_add("111111111011111111101.1111111100", precision);
    LongNumber result = a + b;
    if (!approxEqual(result, expected_add)) {
        std::cout << "Large Numbers Addition test failed." << std::endl;
        return false;
    }
    LongNumber expected_sub("86419753208641975320.8641975320", precision);
    result = b - a;
    if (!approxEqual(result, expected_sub)) {
        std::cout << "Large Numbers Subtraction test failed." << std::endl;
        return false;
    }
    return true;
}

bool test_rounding_division() {
    const int precision = 20;
    LongNumber a("10", precision);
    LongNumber b("3", precision);
    LongNumber expected("3.3333333333", precision);
    LongNumber result = a / b;
    if (!approxEqual(result, expected)) {
        std::cout << "Rounding Division test failed." << std::endl;
        std::cout << "Expected: ";
        print_long_number(expected);
        std::cout << "Result:   ";
        print_long_number(result);
        return false;
    }
    return true;
}

int main() {
    int tests_passed = 0;
    int tests_total = 0;
    
    auto run_test = [&](const std::string &test_name, bool (*test_func)()){
        tests_total++;
        bool result = test_func();
        std::cout << test_name << ": " << (result ? "OK" : "FAIL") << std::endl;
        if (result)
            tests_passed++;
    };
    
    run_test("Test Addition", test_addition);
    run_test("Test Subtraction", test_subtraction);
    run_test("Test Multiplication", test_multiplication);
    run_test("Test Division", test_division);
    run_test("Test String Constructor", test_string_constructor);
    run_test("Test Comparisons", test_comparisons);
   // run_test("Test Pi", test_pi);
    
    // Дополнительные тесты:
    run_test("Test Negative Addition", test_negative_addition);
    run_test("Test Mixed Subtraction", test_mixed_subtraction);
    run_test("Test Negative Multiplication", test_negative_multiplication);
    run_test("Test Negative Division", test_negative_division);
    run_test("Test Large Numbers", test_large_numbers);
    run_test("Test Rounding Division", test_rounding_division);
    
    std::cout << "\nPassed " << tests_passed << " out of " << tests_total << " tests." << std::endl;
    
    return 0;
}
