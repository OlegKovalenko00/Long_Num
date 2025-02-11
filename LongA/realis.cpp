#include "head.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// Функция для удаления ведущих нулей из строки
static std::string remove_leading_zeros(const std::string &str) {
    size_t first_non_zero = str.find_first_not_of('0');
    return (first_non_zero == std::string::npos) ? "0" : str.substr(first_non_zero);
}

// Деление неотрицательного числа, заданного строкой, на 2
static std::pair<std::string, int> divide_string_by_2(const std::string &str) {
    std::string result_str;
    int carry = 0;
    for (char ch : str) {
        int digit = ch - '0';
        int cur = carry * 10 + digit;
        int quotient = cur / 2;
        carry = cur % 2;
        result_str.push_back(char('0' + quotient));
    }
    return {remove_leading_zeros(result_str), carry};
}

// Умножение неотрицательного числа, заданного строкой, на 2
static std::string multiply_string_by_2(const std::string &str) {
    int carry = 0;
    std::string result_str = str;
    for (int i = str.size() - 1; i >= 0; i--) {
        int digit = str[i] - '0';
        int prod = digit * 2 + carry;
        result_str[i] = char('0' + (prod % 10));
        carry = prod / 10;
    }
    if (carry > 0)
        result_str.insert(result_str.begin(), char('0' + carry));
    return remove_leading_zeros(result_str);
}

// Вычитание двух неотрицательных чисел, представленных строками
static std::string subtract_strings(const std::string &a_str, const std::string &b_str) {
    std::string s1 = remove_leading_zeros(a_str);
    std::string s2 = remove_leading_zeros(b_str);
    while (s2.size() < s1.size())
        s2 = "0" + s2;
    int carry = 0;
    std::string result(s1.size(), '0');
    for (int i = s1.size() - 1; i >= 0; i--) {
        int digit_a = s1[i] - '0';
        int digit_b = s2[i] - '0';
        int diff = digit_a - digit_b - carry;
        if (diff < 0) {
            diff += 10;
            carry = 1;
        } else {
            carry = 0;
        }
        result[i] = char('0' + diff);
    }
    return remove_leading_zeros(result);
}

// Предполагается, что в head.hpp определён класс LongNumber с полями:
//   std::vector<bool> bit_vector_;
//   int precision_;
//   bool is_negative_;

// Конструктор из строки
LongNumber::LongNumber(const std::string &str, int precision) {
    std::string number_str = str;
    if (!number_str.empty() && number_str[0] == '-') {
        is_negative_ = true;
        number_str = number_str.substr(1);
    } else {
        is_negative_ = false;
    }

    std::string integer_part, fractional_part;
    size_t pos = number_str.find('.');
    if (pos != std::string::npos) {
        integer_part = number_str.substr(0, pos);
        fractional_part = number_str.substr(pos + 1);
    } else {
        integer_part = number_str;
        fractional_part = "";
    }
    integer_part = remove_leading_zeros(integer_part);
    if (integer_part == "")
        integer_part = "0";

    std::vector<bool> int_binary;
    if (integer_part == "0") {
        int_binary.push_back(false);
    } else {
        std::string temp = integer_part;
        while (temp != "0") {
            auto div_res = divide_string_by_2(temp);
            temp = div_res.first;
            int_binary.push_back(div_res.second == 1);
        }
        std::reverse(int_binary.begin(), int_binary.end());
    }

    std::vector<bool> frac_binary;
    if (fractional_part == "") {
        for (int i = 0; i < precision; i++) {
            frac_binary.push_back(false);
        }
    } else {
        std::string numerator = fractional_part;
        std::string denominator = "1" + std::string(fractional_part.size(), '0');
        for (int i = 0; i < precision; i++) {
            numerator = multiply_string_by_2(numerator);
            if (numerator.size() > denominator.size() ||
                (numerator.size() == denominator.size() && numerator >= denominator)) {
                frac_binary.push_back(true);
                numerator = subtract_strings(numerator, denominator);
            } else {
                frac_binary.push_back(false);
            }
        }
    }

    bit_vector_.clear();
    bit_vector_.insert(bit_vector_.end(), int_binary.begin(), int_binary.end());
    bit_vector_.insert(bit_vector_.end(), frac_binary.begin(), frac_binary.end());
    precision_ = precision;
}

// Конструктор из long double
LongNumber::LongNumber(long double number, int precision, bool is_negative)
    : precision_(precision), is_negative_(is_negative) {
    bit_vector_ = convert_to_binary(number, precision, is_negative);
}

// Деструктор
LongNumber::~LongNumber() {
}

// Функция перевода числа из long double в вектор битов
std::vector<bool> LongNumber::convert_to_binary(long double number, int precision, bool is_negative) {
    std::vector<bool> result;
    if (is_negative)
        number = -number;

    unsigned long long int_part = static_cast<unsigned long long>(number);
    long double frac_part = number - int_part;
    std::vector<bool> int_bits;
    if (int_part == 0) {
        int_bits.push_back(false);
    } else {
        while (int_part > 0) {
            int_bits.push_back(int_part % 2);
            int_part /= 2;
        }
        std::reverse(int_bits.begin(), int_bits.end());
    }

    result.insert(result.end(), int_bits.begin(), int_bits.end());

    for (int i = 0; i < precision; i++) {
        frac_part *= 2;
        int bit = static_cast<int>(frac_part);
        result.push_back(bit);
        frac_part -= bit;
    }
    return result;
}

// Функция перевода бинарного массива в десятичное число
long double convert_binary_to_decimal(const std::vector<bool> &binary, int fraction_length) {
    long double result = 0.0;
    int integer_length = binary.size() - fraction_length;

    // Обрабатываем целую часть
    for (int i = 0; i < integer_length; i++) {
        if (binary[i]) {
            result += std::pow(2, integer_length - 1 - i);
        }
    }

    // Обрабатываем дробную часть
    long double power = 0.5;
    for (size_t i = integer_length; i < binary.size(); i++) {
        if (binary[i])
            result += power;
        power /= 2;
    }

    return result;
}

// Конструктор копирования
LongNumber::LongNumber(const LongNumber &other)
    : bit_vector_(other.bit_vector_), precision_(other.precision_), is_negative_(other.is_negative_) {
}

LongNumber &LongNumber::operator=(const LongNumber &other) {
    if (this != &other) {
        bit_vector_ = other.bit_vector_;
        precision_ = other.precision_;
        is_negative_ = other.is_negative_;
    }
    return *this;
}

// Вспомогательная функция для выравнивания векторов битов до максимальной длины
static void align_bit_vectors(const LongNumber &num1, const LongNumber &num2,
                              std::vector<bool> &a, std::vector<bool> &b,
                              int &new_int_len, int &new_frac_len) {
    int a_int_len = num1.bit_vector_.size() - num1.precision_;
    int b_int_len = num2.bit_vector_.size() - num2.precision_;
    new_int_len = std::max(a_int_len, b_int_len);
    new_frac_len = std::max(num1.precision_, num2.precision_);

    // Для num1
    {
        int zeros_to_prepend = new_int_len - a_int_len;
        for (int i = 0; i < zeros_to_prepend; i++)
            a.push_back(false);
        a.insert(a.end(), num1.bit_vector_.begin(), num1.bit_vector_.end());
        int zeros_to_append = new_frac_len - num1.precision_;
        for (int i = 0; i < zeros_to_append; i++)
            a.push_back(false);
    }
    // Для num2
    {
        int zeros_to_prepend = new_int_len - b_int_len;
        for (int i = 0; i < zeros_to_prepend; i++)
            b.push_back(false);
        b.insert(b.end(), num2.bit_vector_.begin(), num2.bit_vector_.end());
        int zeros_to_append = new_frac_len - num2.precision_;
        for (int i = 0; i < zeros_to_append; i++)
            b.push_back(false);
    }
}

LongNumber LongNumber::operator+(const LongNumber &other) const {
    std::vector<bool> a, b;
    int new_int_len, new_frac_len;
    align_bit_vectors(*this, other, a, b, new_int_len, new_frac_len);
    if (is_negative_ == other.is_negative_) {
        int n = a.size();
        int carry = 0;
        std::vector<bool> sum(n, false);
        for (int i = n - 1; i >= 0; i--) {
            int bit_a = a[i] ? 1 : 0;
            int bit_b = b[i] ? 1 : 0;
            int s = bit_a + bit_b + carry;
            sum[i] = (s % 2 != 0);
            carry = s / 2;
        }
        if (carry) {
            sum.insert(sum.begin(), true);
            new_int_len++;
        }
        LongNumber res(0, new_frac_len, is_negative_);
        res.bit_vector_ = sum;
        return res;
    } else {
        bool equal = true;
        bool a_is_larger = false;
        for (size_t i = 0; i < a.size(); i++) {
            if (a[i] != b[i]) {
                equal = false;
                a_is_larger = a[i];
                break;
            }
        }
        if (equal) {
            std::vector<bool> zero(a.size(), false);
            LongNumber res(0, new_frac_len, false);
            res.bit_vector_ = zero;
            return res;
        }
        const std::vector<bool> &larger = a_is_larger ? a : b;
        const std::vector<bool> &smaller = a_is_larger ? b : a;
        int n = larger.size();
        int borrow = 0;
        std::vector<bool> diff(n, false);
        for (int i = n - 1; i >= 0; i--) {
            int bit_l = larger[i] ? 1 : 0;
            int bit_s = smaller[i] ? 1 : 0;
            int d = bit_l - bit_s - borrow;
            if (d < 0) {
                d += 2;
                borrow = 1;
            } else {
                borrow = 0;
            }
            diff[i] = (d != 0);
        }
        bool res_sign = a_is_larger ? is_negative_ : other.is_negative_;
        LongNumber res(0, new_frac_len, res_sign);
        res.bit_vector_ = diff;
        return res;
    }
}

LongNumber LongNumber::operator-(const LongNumber &other) const {
    LongNumber temp = other;
    temp.is_negative_ = !temp.is_negative_;
    return *this + temp;
}

LongNumber LongNumber::operator*(const LongNumber &other) const {
    std::vector<bool> a, b;
    int new_int_len, new_frac_len;
    align_bit_vectors(*this, other, a, b, new_int_len, new_frac_len);
    int total_len = a.size();

    std::vector<int> a_le(total_len), b_le(total_len);
    for (int i = 0; i < total_len; i++) {
        a_le[i] = a[total_len - 1 - i] ? 1 : 0;
        b_le[i] = b[total_len - 1 - i] ? 1 : 0;
    }

    int prod_size = total_len + total_len;
    std::vector<int> prod(prod_size, 0);
    for (int i = 0; i < total_len; i++) {
        for (int j = 0; j < total_len; j++) {
            prod[i + j] += a_le[i] * b_le[j];
        }
    }

    for (int i = 0; i < prod_size; i++) {
        if (prod[i] >= 2) {
            int carry = prod[i] / 2;
            prod[i] %= 2;
            if (i + 1 < prod_size)
                prod[i + 1] += carry;
            else {
                prod.push_back(carry);
                prod_size++;
            }
        }
    }

    if (prod_size <= new_frac_len) {
        std::vector<bool> zero(1, false);
        LongNumber res(0, new_frac_len, false);
        res.bit_vector_ = zero;
        return res;
    }
    std::vector<int> shifted(prod.begin() + new_frac_len, prod.end());
    int res_size = shifted.size();
    std::vector<bool> result(res_size, false);
    for (int i = 0; i < res_size; i++) {
        result[i] = (shifted[res_size - 1 - i] != 0);
    }

    bool res_sign = (is_negative_ != other.is_negative_);
    LongNumber res(0, new_frac_len, res_sign);
    res.bit_vector_ = result;
    return res;
}

LongNumber LongNumber::operator/(const LongNumber &other) const {
    {
        bool is_zero = true;
        for (bool b : other.bit_vector_)
            if (b) {
                is_zero = false;
                break;
            }
        if (is_zero)
            throw std::runtime_error("Делим на ноль");
    }

    std::vector<bool> a, b;
    int new_int_len, new_frac_len;
    align_bit_vectors(*this, other, a, b, new_int_len, new_frac_len);

    std::vector<bool> dividend = a;
    for (int i = 0; i < new_frac_len; i++)
        dividend.push_back(false);

    unsigned long long divd = 0, divs = 0;
    for (bool bit : dividend) {
        divd = (divd << 1) | (bit ? 1ULL : 0ULL);
    }
    for (bool bit : b) {
        divs = (divs << 1) | (bit ? 1ULL : 0ULL);
    }

    unsigned long long quot_val = divd / divs;

    int total_len = new_int_len + new_frac_len;
    std::vector<bool> quotient(total_len, false);
    for (int i = total_len - 1; i >= 0; i--) {
        quotient[i] = (quot_val & 1ULL);
        quot_val >>= 1;
    }

    bool res_sign = (is_negative_ != other.is_negative_);
    LongNumber res(0, new_frac_len, res_sign);
    res.bit_vector_ = quotient;
    return res;
}

// Операторы сравнения
bool LongNumber::operator==(const LongNumber &other) const {
    if (is_negative_ != other.is_negative_)
        return false;
    std::vector<bool> a, b;
    int new_int_len, new_frac_len;
    align_bit_vectors(*this, other, a, b, new_int_len, new_frac_len);
    return (a == b);
}

bool LongNumber::operator!=(const LongNumber &other) const {
    return !(*this == other);
}

bool LongNumber::operator<(const LongNumber &other) const {
    if (is_negative_ != other.is_negative_)
        return is_negative_;
    std::vector<bool> a, b;
    int new_int_len, new_frac_len;
    align_bit_vectors(*this, other, a, b, new_int_len, new_frac_len);
    if (a == b)
        return false;
    if (!is_negative_)
        return (a < b);
    else
        return (b < a);
}

bool LongNumber::operator>(const LongNumber &other) const {
    return other < *this;
}

bool LongNumber::operator<=(const LongNumber &other) const {
    return !(*this > other);
}

bool LongNumber::operator>=(const LongNumber &other) const {
    return !(*this < other);
}

// Функция для смены точности представления числа
void LongNumber::new_precision(int new_precision) {
    long double value = 0.0;
    int total = bit_vector_.size();
    int int_len = total - precision_;
    for (int i = 0; i < int_len; i++) {
        if (bit_vector_[i])
            value += std::pow(2, int_len - 1 - i);
    }
    long double factor = 0.5;
    for (int i = int_len; i < total; i++) {
        if (bit_vector_[i])
            value += factor;
        factor /= 2;
    }
    if (is_negative_)
        value = -value;
    precision_ = new_precision;
    bit_vector_ = convert_to_binary(value, new_precision, (value < 0));
}

// Вспомогательная функция для вывода бинарного представления числа
void print_binary(const std::vector<bool> &bit_vec, int precision, bool is_negative = false) {
    if (is_negative)
        std::cout << "-";
    int int_bits_count = bit_vec.size() - precision;
    if (int_bits_count <= 0)
        std::cout << "0";
    else {
        for (int i = 0; i < int_bits_count; ++i)
            std::cout << bit_vec[i];
    }
    if (precision > 0) {
        std::cout << ".";
        for (size_t i = int_bits_count; i < bit_vec.size(); ++i)
            std::cout << bit_vec[i];
    }
}

int main() {
    return 0;
}
