#include "head.hpp"
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <vector>

namespace {

    std::vector<char> trim_leading_zeros(const std::vector<char> &bit_vector_)
    {
        size_t idx = 0;
        while (idx < bit_vector_.size() - 1 && !bit_vector_[idx])
        {
            idx++;
        }
        return std::vector<char>(bit_vector_.begin() + idx, bit_vector_.end());
    }

    std::vector<char> right_shift(const std::vector<char> &bit_vector_, int shift)
    {
        if (shift >= static_cast<int>(bit_vector_.size()))
        {
            return std::vector<char>{false};
        }
        return std::vector<char>(bit_vector_.begin(), bit_vector_.end() - shift);
    }

    int compare_bit_vectors(const std::vector<char> &a, const std::vector<char> &b)
    {
        std::vector<char> A = trim_leading_zeros(a);
        std::vector<char> B = trim_leading_zeros(b);
        if (A.size() < B.size())
            return -1;
        if (A.size() > B.size())
            return 1;
        for (size_t i = 0; i < A.size(); ++i)
        {
            if (A[i] != B[i])
                return (A[i] ? 1 : -1);
        }
        return 0;
    }

    std::vector<char> subtract_bit_vectors(const std::vector<char> &a, const std::vector<char> &b)
    {
        size_t n = std::max(a.size(), b.size());
        std::vector<char> A(n, false), B(n, false);
        int diffA = n - a.size();
        for (size_t i = 0; i < a.size(); i++)
        {
            A[diffA + i] = a[i];
        }
        int diffB = n - b.size();
        for (size_t i = 0; i < b.size(); i++)
        {
            B[diffB + i] = b[i];
        }
        std::vector<char> result(n, false);
        bool borrow = false;
        for (int i = static_cast<int>(n) - 1; i >= 0; i--)
        {
            int ai = A[i] ? 1 : 0;
            int bi = B[i] ? 1 : 0;
            int sub = ai - bi - (borrow ? 1 : 0);
            if (sub < 0)
            {
                sub += 2;
                borrow = true;
            }
            else
            {
                borrow = false;
            }
            result[i] = (sub != 0);
        }
        return trim_leading_zeros(result);
    }

    std::vector<char> divide_bit_vectors(const std::vector<char> &dividend, const std::vector<char> &divisor, int prec)
    {
        std::vector<char> Q;
        std::vector<char> R;
        for (size_t i = 0; i < dividend.size(); i++)
        {
            R.push_back(dividend[i]);
            R = trim_leading_zeros(R);
            if (compare_bit_vectors(R, divisor) >= 0)
            {
                R = subtract_bit_vectors(R, divisor);
                Q.push_back(true);
            }
            else
            {
                Q.push_back(false);
            }
        }
        while (Q.size() > prec + 1 && Q[0] == 0)
            Q.erase(Q.begin());
        if (Q.empty())
            Q.push_back(false);
        return Q;
    }

    std::vector<char> multiply_bit_vectors(const std::vector<char> &a, const std::vector<char> &b)
    {
        size_t n = a.size();
        size_t m = b.size();
        std::vector<char> result(n + m, false);
        for (int i = static_cast<int>(m) - 1; i >= 0; i--)
        {
            if (b[i])
            {
                int shift = m - 1 - i;
                std::vector<char> shifted(a.size() + shift, false);
                for (size_t j = 0; j < a.size(); j++)
                {
                    shifted[j] = a[j];
                }
                int offset = result.size() - shifted.size();
                bool carry = false;
                for (int k = static_cast<int>(shifted.size()) - 1; k >= 0; k--)
                {
                    int sum = (result[offset + k] ? 1 : 0) + (shifted[k] ? 1 : 0) + (carry ? 1 : 0);
                    result[offset + k] = (sum % 2) != 0;
                    carry = (sum / 2) != 0;
                }
                int k = offset - 1;
                while (carry && k >= 0)
                {
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

LongNumber::LongNumber(long double number, int precision_, bool is_negative)
    : precision_(precision_), is_negative_(is_negative)
{
    bit_vector_ = convert_to_binary(number, precision_, is_negative);
}

LongNumber::LongNumber(const std::string &str, int precision_)
    : precision_(precision_)
{
    size_t start = 0;
    if (str[start] == '-')
    {
        is_negative_ = true;
        start++;
    }
    else
    {
        is_negative_ = false;
    }
    size_t dot_pos = str.find('.');
    std::string integer_part = str.substr(start, dot_pos - start);
    std::string fractional_part = (dot_pos != std::string::npos) ? str.substr(dot_pos + 1) : "";

    std::vector<char> integer_binary;
    if (!integer_part.empty())
    {
        std::string temp = integer_part;
        while (temp != "0")
        {
            int remainder = 0;
            std::string next_temp;
            for (char ch : temp)
            {
                int digit = ch - '0';
                int value = remainder * 10 + digit;
                next_temp.push_back((value / 2) + '0');
                remainder = value % 2;
            }
            integer_binary.push_back(remainder);
            temp = next_temp;
            temp.erase(0, temp.find_first_not_of('0'));
            if (temp.empty())
                temp = "0";
        }
        std::reverse(integer_binary.begin(), integer_binary.end());
    }
    else
    {
        integer_binary.push_back(false);
    }

    std::vector<char> fractional_binary;
    if (!fractional_part.empty())
    {
        long double frac = std::stold("0." + fractional_part);
        for (int i = 0; i < precision_; ++i)
        {
            frac *= 2;
            bool bit = (frac >= 1.0);
            fractional_binary.push_back(bit);
            if (bit)
            {
                frac -= 1.0;
            }
        }
    }
    else
    {
        for (int i = 0; i < precision_; ++i)
        {
            fractional_binary.push_back(false);
        }
    }

    bit_vector_ = integer_binary;
    bit_vector_.insert(bit_vector_.end(), fractional_binary.begin(), fractional_binary.end());
}

LongNumber::LongNumber(const LongNumber &other)
    : bit_vector_(other.bit_vector_), precision_(other.precision_), is_negative_(other.is_negative_) {}

LongNumber &LongNumber::operator=(const LongNumber &other)
{
    if (this != &other)
    {
        bit_vector_ = other.bit_vector_;
        precision_ = other.precision_;
        is_negative_ = other.is_negative_;
    }
    return *this;
}

LongNumber::~LongNumber() {}

std::vector<char> LongNumber::convert_to_binary(long double number, int precision_, bool)
{
    std::vector<char> binary;
    long double abs_number = std::abs(number);
    long double integer_part = std::floor(abs_number);
    long long IntegerPart = static_cast<long long>(integer_part);
    long double fractional_part = abs_number - integer_part;
    while (IntegerPart > 0) {
        binary.insert(binary.begin(), IntegerPart % 2);
        IntegerPart /= 2;
    }
    if (binary.empty())
    {
        binary.push_back(false);
    }
    for (int i = 0; i < precision_; ++i)
    {
        fractional_part *= 2;
        binary.push_back(static_cast<int>(fractional_part));
        fractional_part -= static_cast<int>(fractional_part);
    }
    return binary;
}

static void align_bit_vectors(const LongNumber &num1, const LongNumber &num2,
                              std::vector<char> &aligned_bits1, std::vector<char> &aligned_bits2,
                              int &aligned_int_length, int &aligned_frac_length)
{
    int num1_int_length = num1.get_bit_vector().size() - num1.get_precision();
    int num2_int_length = num2.get_bit_vector().size() - num2.get_precision();
    aligned_int_length = std::max(num1_int_length, num2_int_length);
    aligned_frac_length = std::max(num1.get_precision(), num2.get_precision());

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

LongNumber LongNumber::operator+(const LongNumber &other) const
{
    std::vector<char> a, b;
    int new_int_len, new_frac_len;
    align_bit_vectors(*this, other, a, b, new_int_len, new_frac_len);
    if (is_negative_ == other.is_negative_)
    {
        int n = a.size();
        int carry = 0;
        std::vector<char> sum(n, false);
        for (int i = n - 1; i >= 0; i--)
        {
            int bit_a = a[i] ? 1 : 0;
            int bit_b = b[i] ? 1 : 0;
            int s = bit_a + bit_b + carry;
            sum[i] = (s % 2 != 0);
            carry = s / 2;
        }
        if (carry)
        {
            sum.insert(sum.begin(), true);
            new_int_len++;
        }
        LongNumber res(0, new_frac_len, is_negative_);
        res.bit_vector_ = sum;
        return res;
    }
    else
    {
        bool equal = true;
        bool a_is_larger = false;
        for (size_t i = 0; i < a.size(); i++)
        {
            if (a[i] != b[i])
            {
                equal = false;
                a_is_larger = a[i];
                break;
            }
        }
        if (equal)
        {
            std::vector<char> zero(a.size(), false);
            LongNumber res(0, new_frac_len, false);
            res.bit_vector_ = zero;
            return res;
        }
        const std::vector<char> &larger = a_is_larger ? a : b;
        const std::vector<char> &smaller = a_is_larger ? b : a;
        int n = larger.size();
        int borrow = 0;
        std::vector<char> diff(n, false);
        for (int i = n - 1; i >= 0; i--)
        {
            int bit_l = larger[i] ? 1 : 0;
            int bit_s = smaller[i] ? 1 : 0;
            int d = bit_l - bit_s - borrow;
            if (d < 0)
            {
                d += 2;
                borrow = 1;
            }
            else
            {
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

LongNumber LongNumber::operator-(const LongNumber &other) const
{
    LongNumber temp = other;
    temp.is_negative_ = !temp.is_negative_;
    return *this + temp;
}

LongNumber LongNumber::operator>>(int shift) const
{
    LongNumber ans(*this);
    ans.bit_vector_.insert(ans.bit_vector_.begin(), shift, 0);
    ans.bit_vector_.erase(ans.bit_vector_.end() - shift, ans.bit_vector_.end());
    while (ans.bit_vector_[0] == '0' && ans.bit_vector_.size() > ans.precision_ + 1)
    {
        ans.bit_vector_.erase(ans.bit_vector_.begin());
    }
    return ans;
}

LongNumber LongNumber::operator*(const LongNumber &other) const
{
    std::vector<char> a, b;
    int new_int_len, new_frac_len;
    align_bit_vectors(*this, other, a, b, new_int_len, new_frac_len);
    int total_len = a.size();

    std::vector<int> a_le(total_len), b_le(total_len);
    for (int i = 0; i < total_len; i++)
    {
        a_le[i] = a[total_len - 1 - i] ? 1 : 0;
        b_le[i] = b[total_len - 1 - i] ? 1 : 0;
    }

    int prod_size = total_len + total_len;
    std::vector<int> prod(prod_size, 0);
    for (int i = 0; i < total_len; i++)
    {
        for (int j = 0; j < total_len; j++)
        {
            prod[i + j] += a_le[i] * b_le[j];
        }
    }

    for (int i = 0; i < prod_size; i++)
    {
        if (prod[i] >= 2)
        {
            int carry = prod[i] / 2;
            prod[i] %= 2;
            if (i + 1 < prod_size)
                prod[i + 1] += carry;
            else
            {
                prod.push_back(carry);
                prod_size++;
            }
        }
    }

    if (prod_size <= new_frac_len)
    {
        std::vector<char> zero(1, false);
        LongNumber res(0, new_frac_len, false);
        res.bit_vector_ = zero;
        return res;
    }
    std::vector<int> shifted(prod.begin() + new_frac_len, prod.end());
    int res_size = shifted.size();
    std::vector<char> result(res_size, false);
    for (int i = 0; i < res_size; i++)
    {
        result[i] = (shifted[res_size - 1 - i] != 0);
    }

    bool res_sign = (is_negative_ != other.is_negative_);
    LongNumber res(0, new_frac_len, res_sign);
    res.bit_vector_ = result;
    return res;
}

LongNumber LongNumber::operator/(const LongNumber &other) const
{
    if (other.bit_vector_.empty() || (other.bit_vector_.size() == 1 && !other.bit_vector_[0]))
    {
        throw std::runtime_error("Division by zero.");
    }
    int prec = precision_;
    std::vector<char> dividend = bit_vector_;
    for (int i = 0; i < prec; i++)
    {
        dividend.push_back(false);
    }
    std::vector<char> q = divide_bit_vectors(dividend, other.bit_vector_, prec);
    LongNumber result(0.0, prec, is_negative_ != other.is_negative_);
    result.bit_vector_ = q;
    return result;
}

bool LongNumber::operator==(const LongNumber &other) const
{
    return bit_vector_ == other.bit_vector_ &&
           precision_ == other.precision_ &&
           is_negative_ == other.is_negative_;
}

bool LongNumber::operator!=(const LongNumber &other) const
{
    return !(*this == other);
}

bool LongNumber::operator<(const LongNumber &other) const
{
    if (is_negative_ != other.is_negative_)
    {
        return is_negative_;
    }
    if (bit_vector_.size() != other.bit_vector_.size())
    {
        return bit_vector_.size() < other.bit_vector_.size();
    }
    for (size_t i = 0; i < bit_vector_.size(); ++i)
    {
        if (bit_vector_[i] != other.bit_vector_[i])
        {
            return bit_vector_[i] < other.bit_vector_[i];
        }
    }
    return false;
}

bool LongNumber::operator>(const LongNumber &other) const
{
    return other < *this;
}

bool LongNumber::operator<=(const LongNumber &other) const
{
    return !(*this > other);
}

bool LongNumber::operator>=(const LongNumber &other) const
{
    return !(*this < other);
}

void LongNumber::new_precision(int new_precision)
{
    if (new_precision < 0)
    {
        throw std::invalid_argument("precision_ cannot be negative.");
    }
    int old_precision = precision_;
    if (new_precision > old_precision)
    {
        int shift = new_precision - old_precision;
        for (int i = 0; i < shift; i++)
        {
            bit_vector_.push_back(false);
        }
    }
    else if (new_precision < old_precision)
    {
        int shift = old_precision - new_precision;
        bit_vector_ = right_shift(bit_vector_, shift);
    }
    precision_ = new_precision;
}

LongNumber LongNumber::calculate_pi(int precision_)
{
    LongNumber pi(0.0, precision_, false);
    LongNumber n0(1.0, precision_, false);
    LongNumber n(16.0, precision_, false);

    LongNumber a0(4.0, precision_, false);
    LongNumber b0(2.0, precision_, false);
    LongNumber c0(1.0, precision_, false);
    LongNumber d0(1.0, precision_, false);

    LongNumber a(1.0, precision_, false);
    LongNumber b(4.0, precision_, false);
    LongNumber c(5.0, precision_, false);
    LongNumber d(6.0, precision_, false);

    LongNumber eight(8.0, precision_, false);

    if (precision_ == 0)
    {
        pi = pi + LongNumber(3.0, precision_, false);
    }

    for (int k = 0; k < precision_; ++k)
    {
        pi = pi + n0 * (a0 / a - b0 / b - c0 / c - d0 / d);
        n0 = n0 / n;
        a = a + eight;
        b = b + eight;
        c = c + eight;
        d = d + eight;
    }

    return pi;
}

LongNumber operator"" _longnum(long double number)
{
    return LongNumber(number, 10, false);
}

void LongNumber::printk_binary(const std::vector<char> &bit_vector_, int precision_, bool is_negative) const
{
    if (is_negative)
    {
        std::cout << "-";
    }
    size_t integer_part_size = bit_vector_.size() - precision_;
    for (size_t i = 0; i < integer_part_size; ++i)
    {
        std::cout << char(bit_vector_[i] + '0');
    }
    if (precision_ > 0)
    {
        std::cout << ".";
    }
    for (size_t i = integer_part_size; i < bit_vector_.size(); ++i)
    {
        std::cout << char(bit_vector_[i] + '0');
    }
    std::cout << std::endl;
}

std::string LongNumber::MultStringOnTwo(const std::string &s) const
{
    std::string result = "";
    int carry = 0;
    for (int i = (int)s.size() - 1; i >= 0; --i)
    {
        int digit = s[i] - '0';
        int value = digit * 2 + carry;
        carry = value / 10;
        result.push_back(char(value % 10 + '0'));
    }

    result.push_back(char('0' + carry));

    std::reverse(result.begin(), result.end());
    return result;
}

std::string LongNumber::DivStringOnTwo(const std::string &s) const
{
    std::string result = "";
    int carry = 0;
    for (char c : s)
    {
        int digit = c - '0';
        int value = carry * 10 + digit;
        carry = value % 2;
        result.push_back(char('0' + value / 2));
    }
    while (carry != 0)
    {
        carry *= 10;
        result.push_back(char('0' + carry / 2));
        carry %= 2;
    }
    return result;
}

std::string LongNumber::SumTwoString(const std::string &num1, const std::string &num2, int type) const
{
    int len = std::max(num1.size(), num2.size());
    std::string a(num1), b(num2);
    if (type == 0)
    {
        a.insert(a.begin(), len - num1.size(), '0');
        b.insert(b.begin(), len - num2.size(), '0');
    }
    else
    {
        a.insert(a.end(), len - num1.size(), '0');
        b.insert(b.end(), len - num2.size(), '0');
    }
    std::string result = "";
    int carry = 0;
    for (int i = len - 1; i >= 0; --i)
    {
        int digit1 = (i < a.size() ? a[i] : '0') - '0';
        int digit2 = (i < b.size() ? b[i] : '0') - '0';
        int value = digit1 + digit2 + carry;
        carry = value / 10;
        value %= 10;
        result.push_back(char('0' + value));
    }
    if (type == 0)
        result.push_back(char('0' + carry));
    std::reverse(result.begin(), result.end());
    return result;
}

std::string LongNumber::to_string() const
{
    std::string IntegerPart = "", FractionalPart = "";
    std::string temp = "1";

    for (int i = (int)bit_vector_.size() - precision_ - 1; i >= 0; --i)
    {
        if (bit_vector_[i])
            IntegerPart = SumTwoString(IntegerPart, temp, 0);
        
        temp = MultStringOnTwo(temp);

        while (temp[0] == '0')
            temp.erase(temp.begin());

        while (IntegerPart[0] == '0')
            IntegerPart.erase(IntegerPart.begin());
    }

    temp = "5";
    for (int i = (int)bit_vector_.size() - precision_; i < (int)bit_vector_.size(); ++i)
    {
        if (bit_vector_[i])
            FractionalPart = SumTwoString(FractionalPart, temp, 1);

        temp = DivStringOnTwo(temp);

        while (temp.back() == '0')
            temp.pop_back();

        while (FractionalPart.back() == '0')
            FractionalPart.pop_back();
    }

    while (IntegerPart[0] == '0')
        IntegerPart.erase(IntegerPart.begin());

    while (FractionalPart.back() == '0')
        FractionalPart.pop_back();

    if (IntegerPart.empty())
        IntegerPart = "0";
    if (FractionalPart.empty())
        FractionalPart = "0";

    if(is_negative_ == true)
        IntegerPart = "-" + IntegerPart;

    return IntegerPart + "." + FractionalPart;
}

std::ostream &operator<<(std::ostream &os, const LongNumber &num)
{
    os << num.to_string();
    return os;
}
