#include "head.hpp"
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <vector>

// Вспомогательные функции для работы с vector<bool> как с двоичными числами
namespace {

    // Удаление ведущих нулей
    std::vector<bool> trim_leading_zeros(const std::vector<bool>& bits) {
        size_t idx = 0;
        while (idx < bits.size() - 1 && !bits[idx]) {
            idx++;
        }
        return std::vector<bool>(bits.begin() + idx, bits.end());
    }

    // Функция правого сдвига на shift битов (удаляются shift младших битов)
    std::vector<bool> right_shift(const std::vector<bool>& bits, int shift) {
        if (shift >= static_cast<int>(bits.size())) {
            return std::vector<bool>{false};
        }
        return std::vector<bool>(bits.begin(), bits.end() - shift);
    }

    // Сравнение двух векторов битов (представляют неотрицательные числа в MSB-порядке)
    // Возвращает -1, если a < b, 0 если a == b, 1 если a > b.
    int compare_bit_vectors(const std::vector<bool>& a, const std::vector<bool>& b) {
        std::vector<bool> A = trim_leading_zeros(a);
        std::vector<bool> B = trim_leading_zeros(b);
        if (A.size() < B.size()) return -1;
        if (A.size() > B.size()) return 1;
        for (size_t i = 0; i < A.size(); ++i) {
            if (A[i] != B[i])
                return (A[i] ? 1 : -1);
        }
        return 0;
    }

    // Вычитание двух векторов битов (a >= b). Результат – вектор битов в MSB-порядке.
    std::vector<bool> subtract_bit_vectors(const std::vector<bool>& a, const std::vector<bool>& b) {
        size_t n = std::max(a.size(), b.size());
        std::vector<bool> A(n, false), B(n, false);
        // Выравнивание по правому краю (LSB)
        int diffA = n - a.size();
        for (size_t i = 0; i < a.size(); i++) {
            A[diffA + i] = a[i];
        }
        int diffB = n - b.size();
        for (size_t i = 0; i < b.size(); i++) {
            B[diffB + i] = b[i];
        }
        std::vector<bool> result(n, false);
        bool borrow = false;
        for (int i = static_cast<int>(n) - 1; i >= 0; i--) {
            int ai = A[i] ? 1 : 0;
            int bi = B[i] ? 1 : 0;
            int sub = ai - bi - (borrow ? 1 : 0);
            if (sub < 0) {
                sub += 2;
                borrow = true;
            } else {
                borrow = false;
            }
            result[i] = (sub != 0);
        }
        return trim_leading_zeros(result);
    }

    // Двоичное длинное деление. dividend и divisor – неотрицательные числа в виде vector<bool> (MSB-первым).
    // Функция возвращает вектор битов частного (MSB-первым).
    std::vector<bool> divide_bit_vectors(const std::vector<bool>& dividend, const std::vector<bool>& divisor, int prec) {
        std::vector<bool> Q; // частное
        std::vector<bool> R; // остаток (начинаем с пустого)
        for (size_t i = 0; i < dividend.size(); i++) {
            // Сдвигаем остаток влево и добавляем очередной бит из делимого
            R.push_back(dividend[i]);
            R = trim_leading_zeros(R);
            if (compare_bit_vectors(R, divisor) >= 0) {
                R = subtract_bit_vectors(R, divisor);
                Q.push_back(true);
            } else {
                Q.push_back(false);
            }
        }
        while(Q.size() > prec + 1 && Q[0] == 0)
            Q.erase(Q.begin());
        if (Q.empty())
            Q.push_back(false);
        return Q;
    }


    // Функция умножения двух двоичных чисел, представленных vector<bool> в MSB-порядке.
    std::vector<bool> multiply_bit_vectors(const std::vector<bool>& a, const std::vector<bool>& b) {
        size_t n = a.size();
        size_t m = b.size();
        // Результат может иметь до n + m битов
        std::vector<bool> result(n + m, false);
        // Для умножения обход от LSB к MSB у множителя b (LSB находится в конце).
        for (int i = static_cast<int>(m) - 1; i >= 0; i--) {
            if (b[i]) {
                int shift = m - 1 - i;
                // Сдвигаем a влево на shift битов.
                std::vector<bool> shifted(a.size() + shift, false);
                for (size_t j = 0; j < a.size(); j++) {
                    shifted[j] = a[j];
                }
                // Выравниваем shifted с результатом так, чтобы их LSB совпадали.
                int offset = result.size() - shifted.size();
                bool carry = false;
                for (int k = static_cast<int>(shifted.size()) - 1; k >= 0; k--) {
                    int sum = (result[offset + k] ? 1 : 0) + (shifted[k] ? 1 : 0) + (carry ? 1 : 0);
                    result[offset + k] = (sum % 2) != 0;
                    carry = (sum / 2) != 0;
                }
                int k = offset - 1;
                while (carry && k >= 0) {
                    int sum = (result[k] ? 1 : 0) + 1;
                    result[k] = (sum % 2) != 0;
                    carry = (sum / 2) != 0;
                    k--;
                }
            }
        }
        return trim_leading_zeros(result);
    }

} // end anonymous namespace

// --- Реализация класса LongNumber ---

// Конструктор (число типа long double)
LongNumber::LongNumber(long double number, int precision, bool is_negative) 
    : precision_(precision), is_negative_(is_negative) {
    bit_vector_ = convert_to_binary(number, precision, is_negative);
}

// Конструктор (число, заданное строкой)
LongNumber::LongNumber(const std::string &str, int precision) 
    : precision_(precision) {
    size_t start = 0;
    if (str[start] == '-') {
        is_negative_ = true;
        start++;
    } else {
        is_negative_ = false;
    }
    size_t dot_pos = str.find('.');
    std::string integer_part = str.substr(start, dot_pos - start);
    std::string fractional_part = (dot_pos != std::string::npos) ? str.substr(dot_pos + 1) : "";

    std::vector<bool> integer_binary;
    if (!integer_part.empty()) {
        std::string temp = integer_part;
        while (temp != "0") {
            int remainder = 0;
            std::string next_temp;
            for (char ch : temp) {
                int digit = ch - '0';
                int value = remainder * 10 + digit;
                next_temp.push_back((value / 2) + '0');
                remainder = value % 2;
            }
            integer_binary.push_back(remainder);
            temp = next_temp;
            temp.erase(0, temp.find_first_not_of('0'));
            if (temp.empty()) temp = "0";
        }
        std::reverse(integer_binary.begin(), integer_binary.end());
    } else {
        integer_binary.push_back(false);
    }

    std::vector<bool> fractional_binary;
    if (!fractional_part.empty()) {
        long double frac = std::stold("0." + fractional_part);
        for (int i = 0; i < precision; ++i) {
            frac *= 2;
            bool bit = (frac >= 1.0);
            fractional_binary.push_back(bit);
            if (bit) {
                frac -= 1.0;
            }
        }
    } else {
        for (int i = 0; i < precision; ++i) {
            fractional_binary.push_back(false);
        }
    }

    bit_vector_ = integer_binary;
    bit_vector_.insert(bit_vector_.end(), fractional_binary.begin(), fractional_binary.end());
}

// Конструктор копирования
LongNumber::LongNumber(const LongNumber &other) 
    : bit_vector_(other.bit_vector_), precision_(other.precision_), is_negative_(other.is_negative_) {}

// Оператор присваивания
LongNumber& LongNumber::operator=(const LongNumber &other) {
    if (this != &other) {
        bit_vector_ = other.bit_vector_;
        precision_ = other.precision_;
        is_negative_ = other.is_negative_;
    }
    return *this;
}

// Деструктор
LongNumber::~LongNumber() {}

// Функция перевода числа из long double в вектор битов
std::vector<bool> LongNumber::convert_to_binary(long double number, int precision, bool /*is_negative*/) {
    std::vector<bool> binary;
    long double abs_number = std::abs(number);
    long double integer_part = std::floor(abs_number);
    long double fractional_part = abs_number - integer_part;
    while (integer_part > 0) {
        binary.insert(binary.begin(), static_cast<bool>(std::fmod(integer_part, 2)));
        integer_part = std::floor(integer_part / 2);
    }
    if (binary.empty()) {
        binary.push_back(false);
    }
    for (int i = 0; i < precision; ++i) {
        fractional_part *= 2;
        binary.push_back(static_cast<bool>(std::floor(fractional_part)));
        fractional_part -= std::floor(fractional_part);
    }
    return binary;
}

// Арифметические операции

// Оператор сложения (выравнивание по двоичной точке)
LongNumber LongNumber::operator+(const LongNumber &other) const {
    // Если знаки разные, сводим к вычитанию
    if (is_negative_ != other.is_negative_) {
        LongNumber temp = other;
        temp.is_negative_ = !temp.is_negative_;
        return *this - temp;
    }
    int prec = precision_;
    // Вычисляем длину целой части для каждого числа
    size_t int_A = bit_vector_.size() - prec;
    size_t int_B = other.bit_vector_.size() - prec;
    size_t max_int = std::max(int_A, int_B);
    size_t total_length = max_int + prec;

    // Дополняем числа нулями слева до одинаковой длины
    std::vector<bool> A(total_length, false);
    std::vector<bool> B(total_length, false);
    size_t offset_A = total_length - bit_vector_.size();
    size_t offset_B = total_length - other.bit_vector_.size();
    std::copy(bit_vector_.begin(), bit_vector_.end(), A.begin() + offset_A);
    std::copy(other.bit_vector_.begin(), other.bit_vector_.end(), B.begin() + offset_B);

    std::vector<bool> result(total_length, false);
    bool carry = false;
    for (int i = static_cast<int>(total_length) - 1; i >= 0; i--) {
        int sum = (A[i] ? 1 : 0) + (B[i] ? 1 : 0) + (carry ? 1 : 0);
        result[i] = (sum % 2) != 0;
        carry = (sum / 2) != 0;
    }
    if (carry) {
        result.insert(result.begin(), true);
    }
    LongNumber sum_result(0.0, prec, is_negative_);
    sum_result.bit_vector_ = trim_leading_zeros(result);
    return sum_result;
}

// Оператор вычитания с корректным вычислением модуля и знака
LongNumber LongNumber::operator-(const LongNumber &other) const {
    // Если знаки разные, сводим к сложению
    if (is_negative_ != other.is_negative_) {
        LongNumber temp = other;
        temp.is_negative_ = !temp.is_negative_;
        return *this + temp;
    }
    int prec = precision_;
    size_t int_A = bit_vector_.size() - prec;
    size_t int_B = other.bit_vector_.size() - prec;
    size_t max_int = std::max(int_A, int_B);
    size_t total_length = max_int + prec;

    std::vector<bool> A(total_length, false);
    std::vector<bool> B(total_length, false);
    size_t offset_A = total_length - bit_vector_.size();
    size_t offset_B = total_length - other.bit_vector_.size();
    std::copy(bit_vector_.begin(), bit_vector_.end(), A.begin() + offset_A);
    std::copy(other.bit_vector_.begin(), other.bit_vector_.end(), B.begin() + offset_B);

    int cmp = compare_bit_vectors(A, B);
    // Если числа равны, результат 0
    if (cmp == 0) {
        std::vector<bool> zero(total_length, false);
        LongNumber result(0.0, prec, false);
        result.bit_vector_ = zero;
        return result;
    }
    bool result_negative;
    std::vector<bool> result_bits;
    if (cmp > 0) {
        result_negative = is_negative_;
        result_bits = subtract_bit_vectors(A, B);
    } else {
        result_negative = !is_negative_;
        result_bits = subtract_bit_vectors(B, A);
    }
    LongNumber diff_result(0.0, prec, result_negative);
    diff_result.bit_vector_ = trim_leading_zeros(result_bits);
    return diff_result;
}

// Оператор умножения с корректным масштабированием
LongNumber LongNumber::operator*(const LongNumber &other) const {
    int prec = precision_;
    std::vector<bool> prod = multiply_bit_vectors(bit_vector_, other.bit_vector_);
    // Корректный правый сдвиг на precision битов (деление на 2^(precision))
    prod = right_shift(prod, prec);
    LongNumber result(0.0, prec, is_negative_ != other.is_negative_);
    result.bit_vector_ = trim_leading_zeros(prod);
    return result;
}

// Оператор деления с использованием двоичного длинного деления
// Оператор деления с использованием двоичного длинного деления
LongNumber LongNumber::operator/(const LongNumber &other) const {
    if (other.bit_vector_.empty() || (other.bit_vector_.size() == 1 && !other.bit_vector_[0])) {
        throw std::runtime_error("Division by zero.");
    }
    int prec = precision_;
    // Сдвигаем делимое влево на prec битов (умножая на 2^(precision))
    std::vector<bool> dividend = bit_vector_;
    for (int i = 0; i < prec; i++) {
        dividend.push_back(false);
    }
    std::vector<bool> q = divide_bit_vectors(dividend, other.bit_vector_, prec);
    LongNumber result(0.0, prec, is_negative_ != other.is_negative_);
    result.bit_vector_ = q;
    return result;
}


// Операторы сравнения (остались без изменений)
bool LongNumber::operator==(const LongNumber &other) const {
    return bit_vector_ == other.bit_vector_ &&
           precision_ == other.precision_ &&
           is_negative_ == other.is_negative_;
}

bool LongNumber::operator!=(const LongNumber &other) const {
    return !(*this == other);
}

bool LongNumber::operator<(const LongNumber &other) const {
    if (is_negative_ != other.is_negative_) {
        return is_negative_;
    }
    if (bit_vector_.size() != other.bit_vector_.size()) {
        return bit_vector_.size() < other.bit_vector_.size();
    }
    for (size_t i = 0; i < bit_vector_.size(); ++i) {
        if (bit_vector_[i] != other.bit_vector_[i]) {
            return bit_vector_[i] < other.bit_vector_[i];
        }
    }
    return false;
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

// Функция изменения точности представления числа с корректировкой значения
void LongNumber::new_precision(int new_precision) {
    if (new_precision < 0) {
        throw std::invalid_argument("Precision cannot be negative.");
    }
    int old_precision = precision_;
    if (new_precision > old_precision) {
        // Увеличение точности: умножаем на 2^(new_precision - old_precision)
        int shift = new_precision - old_precision;
        for (int i = 0; i < shift; i++) {
            bit_vector_.push_back(false);
        }
    } else if (new_precision < old_precision) {
        // Уменьшение точности: делим на 2^(old_precision - new_precision)
        int shift = old_precision - new_precision;
        bit_vector_ = right_shift(bit_vector_, shift);
    }
    precision_ = new_precision;
}

// Статическая функция вычисления числа π (формула BBP)
LongNumber LongNumber::calculate_pi(int precision) {
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
    for (int k = 0; k < precision; ++k) {
        pi = pi + n0 * (a0 / a - b0 / b - c0 / c - d0 / d);
        n0 = n0 / n;
        a = a + eight;
        b = b + eight;
        c = c + eight;
        d = d + eight;
    }

    return pi;
}

// Пользовательский литерал для создания LongNumber
LongNumber operator"" _longnum(long double number) {
    return LongNumber(number, 10, false);
}

// Функция вывода числа в двоичном виде
void LongNumber::printk_binary(const std::vector<bool>& bits, int precision, bool is_negative) const {
    if (is_negative) {
        std::cout << "-";
    }
    size_t integer_part_size = bits.size() - precision;
    for (size_t i = 0; i < integer_part_size; ++i) {
        std::cout << bits[i];
    }
    if (precision > 0) {
        std::cout << ".";
    }
    for (size_t i = integer_part_size; i < bits.size(); ++i) {
        std::cout << bits[i];
    }
    std::cout << std::endl;
}

std::ostream& operator<<(std::ostream& os, const LongNumber& num) {
    num.printk_binary(num.bit_vector_, num.precision_, num.is_negative_);
    return os;
}
