// longnum.cpp
#include "head.hpp"
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

// Вспомогательная функция: удаляет ведущие нули в векторе битов (MSB-first)
static std::vector<bool> removeLeadingZeros(const std::vector<bool>& vec) {
    size_t i = 0;
    while(i < vec.size() && !vec[i]) {
        i++;
    }
    if(i == vec.size())
        return std::vector<bool>{false}; // если все нули, возвращаем один нулевой бит
    return std::vector<bool>(vec.begin() + i, vec.end());
}

// Функция выравнивания битовых векторов двух чисел для арифметических операций.
// После выравнивания оба числа имеют одинаковую длину целой и дробной части.
void align_bit_vectors(const LongNumber &num1, const LongNumber &num2,
                       std::vector<bool> &a, std::vector<bool> &b,
                       int &new_int_len, int &new_frac_len) {
    int int_len1 = num1.bit_vector_.size() - num1.precision_;
    int int_len2 = num2.bit_vector_.size() - num2.precision_;
    new_int_len = std::max(int_len1, int_len2);
    new_frac_len = std::max(num1.precision_, num2.precision_);
    int total_len = new_int_len + new_frac_len;
    // Для num1 – дополнение слева и справа нулями
    int pad_left1 = new_int_len - int_len1;
    int pad_right1 = new_frac_len - num1.precision_;
    a = std::vector<bool>(pad_left1, false);
    a.insert(a.end(), num1.bit_vector_.begin(), num1.bit_vector_.end());
    a.insert(a.end(), pad_right1, false);
    // Для num2
    int pad_left2 = new_int_len - int_len2;
    int pad_right2 = new_frac_len - num2.precision_;
    b = std::vector<bool>(pad_left2, false);
    b.insert(b.end(), num2.bit_vector_.begin(), num2.bit_vector_.end());
    b.insert(b.end(), pad_right2, false);
}

// Арифметическое сложение двух выровненных (одинаковой длины) чисел, представленных в MSB-first.
static std::vector<bool> add_binary(const std::vector<bool>& a, const std::vector<bool>& b) {
    int n = a.size();
    int carry = 0;
    std::vector<bool> result(n, false);
    for (int i = n - 1; i >= 0; i--) {
        int sum = (a[i] ? 1 : 0) + (b[i] ? 1 : 0) + carry;
        result[i] = (sum % 2) == 1;
        carry = sum / 2;
    }
    if(carry) {
        std::vector<bool> res;
        res.push_back(true);
        res.insert(res.end(), result.begin(), result.end());
        return res;
    }
    return result;
}

// Вычитание двух выровненных чисел (a - b) при условии, что a >= b по абсолютной величине.
static std::vector<bool> subtract_binary(const std::vector<bool>& a, const std::vector<bool>& b) {
    int n = a.size();
    int borrow = 0;
    std::vector<bool> result(n, false);
    for (int i = n - 1; i >= 0; i--) {
        int diff = (a[i] ? 1 : 0) - (b[i] ? 1 : 0) - borrow;
        if(diff < 0) { diff += 2; borrow = 1; }
        else { borrow = 0; }
        result[i] = (diff == 1);
    }
    return removeLeadingZeros(result);
}

// Сравнение двух чисел (без учёта знака). Возвращает 1, если a > b, -1 если a < b, и 0 если равны.
static int compare_binary(const std::vector<bool>& a, const std::vector<bool>& b) {
    std::vector<bool> A = removeLeadingZeros(a);
    std::vector<bool> B = removeLeadingZeros(b);
    if(A.size() < B.size()) return -1;
    if(A.size() > B.size()) return 1;
    for (size_t i = 0; i < A.size(); i++) {
        if(A[i] != B[i])
            return A[i] ? 1 : -1;
    }
    return 0;
}

// Для умножения удобно работать с представлением в порядке LSB-first.
static std::vector<bool> to_lsb(const std::vector<bool>& msb) {
    return std::vector<bool>(msb.rbegin(), msb.rend());
}
static std::vector<bool> to_msb(const std::vector<bool>& lsb) {
    return std::vector<bool>(lsb.rbegin(), lsb.rend());
}

// Локальная функция сложения для LSB-представления.
static std::vector<bool> add_lsb(const std::vector<bool>& a, const std::vector<bool>& b) {
    size_t n = std::max(a.size(), b.size());
    std::vector<bool> res;
    bool carry = false;
    for (size_t i = 0; i < n; i++) {
        int bit_a = (i < a.size() && a[i]) ? 1 : 0;
        int bit_b = (i < b.size() && b[i]) ? 1 : 0;
        int s = bit_a + bit_b + (carry ? 1 : 0);
        res.push_back((s % 2) == 1);
        carry = (s >= 2);
    }
    if(carry) res.push_back(true);
    return res;
}

// Функция умножения двух чисел в LSB-представлении.
static std::vector<bool> multiply_lsb(const std::vector<bool>& a, const std::vector<bool>& b) {
    std::vector<bool> res(a.size() + b.size(), false);
    for (size_t i = 0; i < a.size(); i++) {
        if(a[i]) {
            // Создаём сдвинутую копию b
            std::vector<bool> temp(i, false);
            temp.insert(temp.end(), b.begin(), b.end());
            res = add_lsb(res, temp);
        }
    }
    return res;
}

// Универсальная функция округления для LongNumber.
// Предполагается, что в bit_vector_ хранится дробная часть с precision_+1 битом.
// После округления в bit_vector_ останется ровно precision_ бит дробной части.
void LongNumber::round_to_precision() {
    int total_len = bit_vector_.size();
    // Число целых разрядов = total_len - (precision_ + 1)
    int int_len = total_len - (precision_ + 1);
    if (int_len < 0) return; // Некорректное состояние – ничего не делаем
    // Извлекаем дополнительный бит для округления (последний бит)
    bool round_bit = bit_vector_.back();
    // Удаляем дополнительный бит
    bit_vector_.resize(total_len - 1);
    // Если бит для округления равен 1, прибавляем единицу с переносом
    if (round_bit) {
        int i = bit_vector_.size() - 1;
        bool carry = true;
        while (i >= 0 && carry) {
            if (!bit_vector_[i]) {
                bit_vector_[i] = true;
                carry = false;
            } else {
                bit_vector_[i] = false;
                i--;
            }
        }
        if (carry) {
            bit_vector_.insert(bit_vector_.begin(), true);
        }
    }
}

// Операция сложения
LongNumber LongNumber::operator+(const LongNumber &other) const {
    std::vector<bool> a, b;
    int new_int_len, new_frac_len;
    // Выравниваем до максимальной длины целой и дробной частей
    align_bit_vectors(*this, other, a, b, new_int_len, new_frac_len);
    
    // Модификация: расширяем выравненные вектора на 1 доп. бит для округления
    a.push_back(false);  
    b.push_back(false);
    
    // Теперь новая длина дробной части становится new_frac_len + 1
    int extended_frac = new_frac_len + 1;
    
    // Производим сложение выровненных чисел
    std::vector<bool> sum = add_binary(a, b);
    
    // Создаем результат с исходной точностью new_frac_len (без доп. бита)
    LongNumber result(0, new_frac_len);
    result.bit_vector_ = removeLeadingZeros(sum);
    result.precision_ = new_frac_len;
    result.is_negative_ = this->is_negative_;
    
    // Округление результата с опорой на доп. бит
    result.round_to_precision();
    
    return result;
}

// Вычитание реализуем как сложение с противоположным знаком.
LongNumber LongNumber::operator-(const LongNumber &other) const {
    std::vector<bool> a, b;
    int new_int_len, new_frac_len;
    align_bit_vectors(*this, other, a, b, new_int_len, new_frac_len);
    
    // Если одно из чисел отрицательное, выполняем сложение
    if (this->is_negative_ != other.is_negative_) {
        std::vector<bool> sum;
        a.push_back(false);
        b.push_back(false);
        sum = add_binary(a, b);
        
        LongNumber result(0, new_frac_len);
        result.bit_vector_ = removeLeadingZeros(sum);
        result.precision_ = new_frac_len;
        result.is_negative_ = this->is_negative_; // знак результата совпадает с первым числом
        result.round_to_precision();
        return result;
    }
    
    int cmp = compare_binary(a, b);
    std::vector<bool> diff;
    bool result_sign = false;
    if (cmp == 0) {
        diff = std::vector<bool>{false};
    } else if (cmp > 0) {
        // Если |this| > |other|, отнимаем b от a.
        diff = subtract_binary(a, b);
        result_sign = this->is_negative_; // знак результата такой же, как у первого числа
    } else {
        // Если |this| < |other|, отнимаем a от b.
        diff = subtract_binary(b, a);
        result_sign = !this->is_negative_;
    }
    
    LongNumber result(0, new_frac_len);
    result.bit_vector_ = removeLeadingZeros(diff);
    result.precision_ = new_frac_len;
    result.is_negative_ = result_sign;
    return result;
}


// Умножение: результат вычисляется как (|A| * |B|) >> precision с округлением.
LongNumber LongNumber::operator*(const LongNumber &other) const {
    int result_precision = this->precision_;
    std::vector<bool> a_abs = this->bit_vector_;
    std::vector<bool> b_abs = other.bit_vector_;
    std::vector<bool> a_lsb = to_lsb(a_abs);
    std::vector<bool> b_lsb = to_lsb(b_abs);
    std::vector<bool> product_lsb = multiply_lsb(a_lsb, b_lsb);
    std::vector<bool> product = to_msb(product_lsb);
    if ((int)product.size() <= result_precision) {
        product = std::vector<bool>{false};
    } else {
        // Используем дополнительный бит для округления: сдвигаем на precision+1 бит
        bool round_bit = product[product.size() - result_precision - 1];
        product.resize(product.size() - result_precision);
        if (round_bit) {
            int i = product.size() - 1;
            bool carry = true;
            while (i >= 0 && carry) {
                if (!product[i]) {
                    product[i] = true;
                    carry = false;
                } else {
                    product[i] = false;
                    i--;
                }
            }
            if (carry)
                product.insert(product.begin(), true);
        }
    }
    product = removeLeadingZeros(product);
    LongNumber result(0, result_precision);
    result.bit_vector_ = product;
    result.precision_ = result_precision;
    result.is_negative_ = (this->is_negative_ ^ other.is_negative_);
    return result;
}

// Деление: результат вычисляется как (|делимое| << precision) / |делитель| с округлением.
LongNumber LongNumber::operator/(const LongNumber &other) const {
    std::vector<bool> other_abs = removeLeadingZeros(other.bit_vector_);
    if (other_abs.size() == 1 && !other_abs[0])
        throw std::runtime_error("Division by zero");
    int result_precision = this->precision_;
    std::vector<bool> dividend = this->bit_vector_;
    std::vector<bool> divisor = other.bit_vector_;
    // Расширяем делимое на precision+1 бит для округления
    std::vector<bool> dividend_ext = dividend;
    for (int i = 0; i < result_precision + 1; i++) {
        dividend_ext.push_back(false);
    }
    std::vector<bool> quotient;
    std::vector<bool> remainder = {false};
    for (bool bit : dividend_ext) {
        remainder.push_back(bit);
        remainder = removeLeadingZeros(remainder);
        if (compare_binary(remainder, divisor) >= 0) {
            remainder = subtract_binary(remainder, divisor);
            quotient.push_back(true);
        } else {
            quotient.push_back(false);
        }
    }
    // Округление: последний бит используется для округления результата
    bool round_bit = quotient.back();
    quotient.pop_back();
    if (round_bit) {
        int i = quotient.size() - 1;
        bool carry = true;
        while (i >= 0 && carry) {
            if (!quotient[i]) {
                quotient[i] = true;
                carry = false;
            } else {
                quotient[i] = false;
                i--;
            }
        }
        if (carry)
            quotient.insert(quotient.begin(), true);
    }
    quotient = removeLeadingZeros(quotient);
    LongNumber result(0, result_precision);
    result.bit_vector_ = quotient;
    result.precision_ = result_precision;
    result.is_negative_ = (this->is_negative_ ^ other.is_negative_);
    return result;
}

// Операторы сравнения
bool LongNumber::operator==(const LongNumber &other) const {
    if(this->is_negative_ != other.is_negative_)
        return false;
    std::vector<bool> a, b;
    int new_int, new_frac;
    align_bit_vectors(*this, other, a, b, new_int, new_frac);
    return a == b;
}
bool LongNumber::operator!=(const LongNumber &other) const {
    return !(*this == other);
}
bool LongNumber::operator<(const LongNumber &other) const {
    if(*this == other) return false;
    if(this->is_negative_ && !other.is_negative_) return true;
    if(!this->is_negative_ && other.is_negative_) return false;
    std::vector<bool> a, b;
    int new_int, new_frac;
    align_bit_vectors(*this, other, a, b, new_int, new_frac);
    int cmp = compare_binary(a, b);
    return this->is_negative_ ? (cmp > 0) : (cmp < 0);
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

// Функция изменения точности.
void LongNumber::new_precision(int new_precision) {
    // Реализация может быть добавлена по необходимости.
}

// Конструктор для целых чисел.
LongNumber::LongNumber(int value, int precision) : precision_(precision) {
    is_negative_ = (value < 0);
    unsigned int abs_val = (value < 0) ? -value : value;
    std::vector<bool> bits;
    if(abs_val == 0) {
        bits.push_back(false);
    } else {
        while(abs_val > 0) {
            bits.push_back((abs_val & 1) != 0);
            abs_val >>= 1;
        }
        std::reverse(bits.begin(), bits.end());
    }
    bit_vector_ = bits;
    // Дополняем дробную часть нулями
    for (int i = 0; i < precision; i++)
        bit_vector_.push_back(false);
}

// Конструктор для long double.
LongNumber::LongNumber(long double number, int precision, bool /*is_negative*/) : precision_(precision) {
    if(number < 0) {
        is_negative_ = true;
        number = -number;
    } else {
        is_negative_ = false;
    }
    unsigned long long int_part = static_cast<unsigned long long>(number);
    long double frac_part = number - int_part;
    std::vector<bool> int_bits;
    if(int_part == 0) {
        int_bits.push_back(false);
    } else {
        while(int_part > 0) {
            int_bits.push_back((int_part & 1ULL) != 0);
            int_part /= 2;
        }
        std::reverse(int_bits.begin(), int_bits.end());
    }
    std::vector<bool> frac_bits;
    // Вычисляем precision+1 бит дробной части для округления.
    for (int i = 0; i < precision + 1; i++) {
        frac_part *= 2;
        if(frac_part >= 1.0L) {
            frac_bits.push_back(true);
            frac_part -= 1.0L;
        } else {
            frac_bits.push_back(false);
        }
    }
    bit_vector_ = int_bits;
    bit_vector_.insert(bit_vector_.end(), frac_bits.begin(), frac_bits.end());
    // Округляем до заданной точности.
    round_to_precision();
}

// Конструктор из строки.
// Реализован с вычислением precision+1 бит дробной части и округлением,
// а также с конвертацией целой части через умножение на 10.
LongNumber::LongNumber(const std::string &str, int precision) : precision_(precision) {
    bool neg = false;
    std::string s = str;
    if (!s.empty() && s[0] == '-') {
        neg = true;
        s = s.substr(1);
    }
    size_t pos = s.find('.');
    std::string int_part_str = (pos == std::string::npos) ? s : s.substr(0, pos);
    std::string frac_part_str = (pos == std::string::npos) ? "" : s.substr(pos + 1);

    // Обработка целой части
    LongNumber int_value(0, 0);  // precision 0 – без лишних нулей
    for (char ch : int_part_str) {
        if (ch >= '0' && ch <= '9') {
            int_value = int_value * LongNumber(10, 0) + LongNumber(ch - '0', 0);
        }
    }
    std::vector<bool> int_bits = int_value.bit_vector_;

    // Обработка дробной части через целочисленное представление
    std::vector<bool> frac_bits;
    if (!frac_part_str.empty()) {
        unsigned long long frac_int = 0;
        for (char ch : frac_part_str) {
            if (ch >= '0' && ch <= '9') {
                frac_int = frac_int * 10 + (ch - '0');
            }
        }
        unsigned long long denom = 1;
        for (size_t i = 0; i < frac_part_str.size(); i++) {
            denom *= 10;
        }
        // Вычисляем precision+1 бит дробной части
        for (int i = 0; i < precision + 1; i++) {
            frac_int *= 2;
            bool bit = (frac_int >= denom);
            frac_bits.push_back(bit);
            if (bit)
                frac_int -= denom;
        }
    } else {
        // Если дробной части нет, создаём precision+1 нулей
        frac_bits = std::vector<bool>(precision + 1, false);
    }
    // Объединяем целую и дробную части
    bit_vector_ = int_bits;
    bit_vector_.insert(bit_vector_.end(), frac_bits.begin(), frac_bits.end());
    is_negative_ = neg;
    // Округляем до заданной точности.
    round_to_precision();
}

// Конструктор копирования
LongNumber::LongNumber(const LongNumber &other)
    : bit_vector_(other.bit_vector_), precision_(other.precision_), is_negative_(other.is_negative_) { }

// Оператор присваивания
LongNumber& LongNumber::operator=(const LongNumber &other) {
    if(this != &other) {
        bit_vector_ = other.bit_vector_;
        precision_ = other.precision_;
        is_negative_ = other.is_negative_;
    }
    return *this;
}

// Деструктор
LongNumber::~LongNumber() { }

// Функция печати битового представления числа.
void LongNumber::printk_binary(const std::vector<bool> &bit_vec, int precision, bool is_negative) {
    if(is_negative)
        std::cout << "-";
    int int_len = bit_vec.size() - precision;
    if(int_len <= 0)
        std::cout << "0";
    else {
        for (int i = 0; i < int_len; i++) {
            std::cout << (bit_vec[i] ? "1" : "0");
        }
    }
    std::cout << ".";
    for (int i = int_len; i < (int)bit_vec.size(); i++) {
        std::cout << (bit_vec[i] ? "1" : "0");
    }
}

// Пользовательский литерал для создания LongNumber.
LongNumber operator"" _longnum(long double number) {
    int default_precision = 10;
    return LongNumber(number, default_precision, (number < 0));
}

// Функция преобразования LongNumber в long double.
static long double longNumberToLongDouble(const LongNumber &num) {
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

// Функция вычисления квадратного корня методом Ньютона для LongNumber.
static LongNumber sqrt_LongNumber(const LongNumber &num, int newton_iters = 20) {
    long double approx = std::sqrt(longNumberToLongDouble(num));
    LongNumber x(approx, num.get_precision(), false);
    LongNumber two(2, num.get_precision());
    for (int i = 0; i < newton_iters; i++) {
        x = (x + num / x) / two;
    }
    return x;
}

LongNumber LongNumber::calculate_pi(int precision) {
    LongNumber sqrt10005 = sqrt_LongNumber(LongNumber(10005, precision), 20);
    LongNumber C = LongNumber(426880, precision) * sqrt10005;
    LongNumber C_val(640320, precision);
    LongNumber C3 = C_val * C_val * C_val;
    LongNumber M(1, precision);
    LongNumber L(13591409, precision);
    LongNumber X(1, precision);
    LongNumber S = L;
    int max_iter = 10;
    for (int k = 1; k < max_iter; k++) {
        LongNumber k_ln(k, precision);
        LongNumber numerator = LongNumber(6 * k - 5, precision) *
                               LongNumber(2 * k - 1, precision) *
                               LongNumber(6 * k - 1, precision);
        LongNumber denominator = (k_ln * k_ln * k_ln) * C3;
        M = M * (numerator / denominator);
        L = L + LongNumber(545140134, precision);
        X = X * LongNumber(-1, precision) * C3;
        LongNumber T = M * L / X;
        S = S + T;
    }
    LongNumber pi = C / S;
    return pi;
}
