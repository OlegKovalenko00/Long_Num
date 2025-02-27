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
static void align_bit_vectors(const LongNumber &num1, const LongNumber &num2,
    std::vector<bool> &aligned_bits1, std::vector<bool> &aligned_bits2,
    int &aligned_int_length, int &aligned_frac_length) {
    
    int num1_int_length = num1.get_bit_vector().size() - num1.get_precision();
    int num2_int_length = num2.get_bit_vector().size() - num2.get_precision();
    aligned_int_length = std::max(num1_int_length, num2_int_length);
    aligned_frac_length = std::max(num1.get_precision(), num2.get_precision());

    // Выравнивание для num1
    {
        int num1_leading_zeros = aligned_int_length - num1_int_length;
        for (int i = 0; i < num1_leading_zeros; i++)
            aligned_bits1.push_back(false);
        const auto &num1_bits = num1.get_bit_vector();
        aligned_bits1.insert(aligned_bits1.end(), num1_bits.begin(), num1_bits.end());
        int num1_trailing_zeros = aligned_frac_length - num1.get_precision();
        for (int i = 0; i < num1_trailing_zeros; i++)
            aligned_bits1.push_back(false);
    }
    
    // Выравнивание для num2
    {
        int num2_leading_zeros = aligned_int_length - num2_int_length;
        for (int i = 0; i < num2_leading_zeros; i++)
            aligned_bits2.push_back(false);
        const auto &num2_bits = num2.get_bit_vector();
        aligned_bits2.insert(aligned_bits2.end(), num2_bits.begin(), num2_bits.end());
        int num2_trailing_zeros = aligned_frac_length - num2.get_precision();
        for (int i = 0; i < num2_trailing_zeros; i++)
            aligned_bits2.push_back(false);
    }
}



// Оператор сложения (выравнивание по двоичной точке)
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