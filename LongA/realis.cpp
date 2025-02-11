#include "head.hpp"

static std::string removeLeadingZeros(const std::string &s) {
    size_t pos = s.find_first_not_of('0');
    return (pos == std::string::npos) ? "0" : s.substr(pos);
}

// Деление неотрицательного числа на 2
static std::pair<std::string, int> divideStringBy2(const std::string &s) {
    std::string result;
    int carry = 0;
    for (char c : s) {
        int digit = c - '0';
        int cur = carry * 10 + digit;
        int q = cur / 2;
        carry = cur % 2;
        result.push_back(char('0' + q));
    }
    return {removeLeadingZeros(result), carry};
}

// Умножение неотрицательного числа на 2
static std::string multiplyStringBy2(const std::string &s) {
    int carry = 0;
    std::string result = s;
    for (int i = s.size() - 1; i >= 0; i--) {
        int digit = s[i] - '0';
        int prod = digit * 2 + carry;
        result[i] = char('0' + (prod % 10));
        carry = prod / 10;
    }
    if (carry > 0)
        result.insert(result.begin(), char('0' + carry));
    return removeLeadingZeros(result);
}

// Вычитание двух неотрицательных чисел, представленных строками 
static std::string subtractStrings(const std::string &a, const std::string &b) {
    std::string s1 = removeLeadingZeros(a);
    std::string s2 = removeLeadingZeros(b);
    while (s2.size() < s1.size())
        s2 = "0" + s2;
    int carry = 0;
    std::string result(s1.size(), '0');
    for (int i = s1.size() - 1; i >= 0; i--) {
        int digitA = s1[i] - '0';
        int digitB = s2[i] - '0';
        int diff = digitA - digitB - carry;
        if (diff < 0) {
            diff += 10;
            carry = 1;
        } else {
            carry = 0;
        }
        result[i] = char('0' + diff);
    }
    return removeLeadingZeros(result);
}

LongNumber::LongNumber(const std::string &str, int precision) {
    std::string s = str;
    if (!s.empty() && s[0] == '-') {
        neg = true;
        s = s.substr(1);
    } else {
        neg = false;
    }

    std::string integerPart, fractionalPart;
    size_t pos = s.find('.');
    if (pos != std::string::npos) {
        integerPart = s.substr(0, pos);
        fractionalPart = s.substr(pos + 1);
    } else {
        integerPart = s;
        fractionalPart = "";
    }
    integerPart = removeLeadingZeros(integerPart);
    if (integerPart == "") integerPart = "0";

    std::vector<bool> intBinary;
    if (integerPart == "0") {
        intBinary.push_back(false);
    } else {
        std::string temp = integerPart;
        while (temp != "0") {
            auto divRes = divideStringBy2(temp);
            temp = divRes.first;
            intBinary.push_back(divRes.second == 1);
        }
        std::reverse(intBinary.begin(), intBinary.end());
    }

    std::vector<bool> fracBinary;
    if (fractionalPart == "") {
        for (int i = 0; i < precision; i++) {
            fracBinary.push_back(false);
        }
    } else {
        std::string numerator = fractionalPart;
        std::string denominator = "1" + std::string(fractionalPart.size(), '0');
        for (int i = 0; i < precision; i++) {
            numerator = multiplyStringBy2(numerator);
            if (numerator.size() > denominator.size() ||
                (numerator.size() == denominator.size() && numerator >= denominator)) {
                fracBinary.push_back(true);
                numerator = subtractStrings(numerator, denominator);
            } else {
                fracBinary.push_back(false);
            }
        }
    }


    bits.clear();
    bits.insert(bits.end(), intBinary.begin(), intBinary.end());
    bits.insert(bits.end(), fracBinary.begin(), fracBinary.end());
    precis = precision;
}
// Реализация конструктора для long double
LongNumber::LongNumber(long double number, int precision, bool neg)
    : precis(precision), neg(neg) {
    bits = convert_to_binary(number, precision, neg);
}
// Реализация деконструктора
LongNumber::~LongNumber() {
}
// Функция перевода числа из double в vector
std::vector <bool> LongNumber::convert_to_binary(long double number,int precision, bool neg){
    std::vector <bool> result;
    if(neg == 1){
        number = -number;
    }

    unsigned long long int_part = static_cast<unsigned long long>(number);
    long double frac_part = number - int_part;
    std::vector<bool> int_bits;
    if (int_part == 0) {
        // Если целая часть равна нулю, добавляем 0
        int_bits.push_back(false);
    } else {
        while (int_part > 0) {
            int_bits.push_back(int_part % 2);
            int_part /= 2;
        }
        std::reverse(int_bits.begin(), int_bits.end());
    }

    // Добавляю биты целой части в общий результат
    result.insert(result.end(), int_bits.begin(), int_bits.end());

    for(int i = 0; i < precision; i++){
        frac_part *= 2;
        int bit = static_cast<int>(frac_part);
        result.push_back(bit);
        frac_part -= bit;
    }
    return result;
}


// Функция для перевода бинарного массива в десятичное число
long double convertBinaryToDecimal(const std::vector<bool>& binary, int fractionLength) {
    long double result = 0.0;
    int integerLength = binary.size() - fractionLength;

    // Обрабатываем целую часть
    for (int i = 0; i < integerLength; i++) {
        if (binary[i]) {
            result += std::pow(2, integerLength - 1 - i);
        }
    }

    // Обрабатываем дробную часть
    long double power = 0.5; // 2^(-1)
    for (size_t i = integerLength; i < binary.size(); i++) {
        if (binary[i]) {
            result += power;
        }
        power /= 2; 
    }

    return result;
}

// Конструктор копирования
LongNumber::LongNumber(const LongNumber &other)
    : bits(other.bits), precis(other.precis), neg(other.neg)
{
}

LongNumber& LongNumber::operator=(const LongNumber &other) {
    if (this != &other) {
        bits = other.bits;
        precis = other.precis;
        neg = other.neg;
    }
    return *this;
}
// Функция выравнивания векторов до максимальной длинны
static void alignBitVectors(const LongNumber &num1, const LongNumber &num2,
                              std::vector<bool> &A, std::vector<bool> &B,
                              int &new_int_len, int &new_frac_len)
{
    int aIntLen = num1.bits.size() - num1.precis;
    int bIntLen = num2.bits.size() - num2.precis;
    new_int_len = std::max(aIntLen, bIntLen);
    new_frac_len = std::max(num1.precis, num2.precis);

    // Для num1
    {
        int zerosToPrepend = new_int_len - aIntLen;
        for (int i = 0; i < zerosToPrepend; i++)
            A.push_back(false);
        A.insert(A.end(), num1.bits.begin(), num1.bits.end());
        int zerosToAppend = new_frac_len - num1.precis;
        for (int i = 0; i < zerosToAppend; i++)
            A.push_back(false);
    }
    // Для num2
    {
        int zerosToPrepend = new_int_len - bIntLen;
        for (int i = 0; i < zerosToPrepend; i++)
            B.push_back(false);
        B.insert(B.end(), num2.bits.begin(), num2.bits.end());
        int zerosToAppend = new_frac_len - num2.precis;
        for (int i = 0; i < zerosToAppend; i++)
            B.push_back(false);
    }
}

LongNumber LongNumber::operator+(const LongNumber &other) const {
    std::vector<bool> A, B;
    int new_int_len, new_frac_len;
    alignBitVectors(*this, other, A, B, new_int_len, new_frac_len);
    if (neg == other.neg) {
        int n = A.size();
        int carry = 0;
        std::vector<bool> sum(n, false);
        for (int i = n - 1; i >= 0; i--) {
            int bitA = A[i] ? 1 : 0;
            int bitB = B[i] ? 1 : 0;
            int s = bitA + bitB + carry;
            sum[i] = (s % 2 != 0);
            carry = s / 2;
        }
        if (carry) {
            sum.insert(sum.begin(), true);
            new_int_len++;
        }
        LongNumber res(0, new_frac_len, neg);
        res.bits = sum;
        return res;
    }
    else {
        bool equal = true;
        bool a_is_larger = false;
        for (size_t i = 0; i < A.size(); i++) {
            if (A[i] != B[i]) {
                equal = false;
                a_is_larger = A[i];
                break;
            }
        }
        if (equal) {
            std::vector<bool> zero(A.size(), false);
            LongNumber res(0, new_frac_len, false);
            res.bits = zero;
            return res;
        }
        const std::vector<bool> &larger = a_is_larger ? A : B;
        const std::vector<bool> &smaller = a_is_larger ? B : A;
        int n = larger.size();
        int borrow = 0;
        std::vector<bool> diff(n, false);
        for (int i = n - 1; i >= 0; i--) {
            int bitL = larger[i] ? 1 : 0;
            int bitS = smaller[i] ? 1 : 0;
            int d = bitL - bitS - borrow;
            if (d < 0) { d += 2; borrow = 1; }
            else { borrow = 0; }
            diff[i] = (d != 0);
        }
        bool resSign = a_is_larger ? neg : other.neg;
        LongNumber res(0, new_frac_len, resSign);
        res.bits = diff;
        return res;
    }
}

LongNumber LongNumber::operator-(const LongNumber &other) const {
    LongNumber temp = other;
    temp.neg = !temp.neg;
    return *this + temp;
}

LongNumber LongNumber::operator*(const LongNumber &other) const {
    std::vector<bool> A, B;
    int new_int_len, new_frac_len;
    alignBitVectors(*this, other, A, B, new_int_len, new_frac_len);
    int totalLen = A.size();

    std::vector<int> aLE(totalLen), blea(totalLen);
    for (int i = 0; i < totalLen; i++) {
        aLE[i] = A[totalLen - 1 - i] ? 1 : 0;
        blea[i] = B[totalLen - 1 - i] ? 1 : 0;
    }

    int prodSize = totalLen + totalLen;
    std::vector<int> prod(prodSize, 0);
    for (int i = 0; i < totalLen; i++) {
        for (int j = 0; j < totalLen; j++) {
            prod[i+j] += aLE[i] * blea[j];
        }
    }

    for (int i = 0; i < prodSize; i++) {
        if (prod[i] >= 2) {
            int carry = prod[i] / 2;
            prod[i] %= 2;
            if (i+1 < prodSize)
                prod[i+1] += carry;
            else {
                prod.push_back(carry);
                prodSize++;
            }
        }
    }

    if (prodSize <= new_frac_len) {
        std::vector<bool> zero(1, false);
        LongNumber res(0, new_frac_len, false);
        res.bits = zero;
        return res;
    }
    std::vector<int> shifted(prod.begin() + new_frac_len, prod.end());
    int resSize = shifted.size();
    std::vector<bool> result(resSize, false);
    for (int i = 0; i < resSize; i++) {
        result[i] = (shifted[resSize - 1 - i] != 0);
    }

    bool resSign = (neg != other.neg);
    LongNumber res(0, new_frac_len, resSign);
    res.bits = result;
    return res;
}

LongNumber LongNumber::operator/(const LongNumber &other) const {
    {
        bool isZero = true;
        for (bool b : other.bits)
            if (b) { isZero = false; break; }
        if (isZero)
            throw std::runtime_error("Делим на ноль");
    }

    std::vector<bool> A, B;
    int newIntLen, newFracLen;
    alignBitVectors(*this, other, A, B, newIntLen, newFracLen);

    std::vector<bool> dividend = A;
    for (int i = 0; i < newFracLen; i++)
        dividend.push_back(false);

    unsigned long long divd = 0, divs = 0;
    for (bool bit : dividend) {
        divd = (divd << 1) | (bit ? 1ULL : 0ULL);
    }
    for (bool bit : B) {
        divs = (divs << 1) | (bit ? 1ULL : 0ULL);
    }

    unsigned long long quotVal = divd / divs;

    int totalLen = newIntLen + newFracLen;
    std::vector<bool> quotient(totalLen, false);
    for (int i = totalLen - 1; i >= 0; i--) {
        quotient[i] = (quotVal & 1ULL);
        quotVal >>= 1;
    }

    bool resSign = (neg != other.neg);
    LongNumber res(0, newFracLen, resSign);
    res.bits = quotient;
    return res;
}


// Операторы сравнения
bool LongNumber::operator==(const LongNumber &other) const {
    if (neg != other.neg)
        return false;
    std::vector<bool> A, B;
    int newIntLen, newFracLen;
    alignBitVectors(*this, other, A, B, newIntLen, newFracLen);
    return (A == B);
}

bool LongNumber::operator!=(const LongNumber &other) const {
    return !(*this == other);
}

bool LongNumber::operator<(const LongNumber &other) const {
    if (neg != other.neg)
        return neg;
    std::vector<bool> A, B;
    int newIntLen, newFracLen;
    alignBitVectors(*this, other, A, B, newIntLen, newFracLen);
    if (A == B)
        return false;
    if (!neg)
        return (A < B);
    else
        return (B < A);
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


void LongNumber::new_precis(int newPrecis) {
    long double value = 0.0;
    int total = bits.size();
    int intLen = total - precis;
    for (int i = 0; i < intLen; i++) {
        if (bits[i])
            value += std::pow(2, intLen - 1 - i);
    }
    long double factor = 0.5;
    for (int i = intLen; i < total; i++) {
        if (bits[i])
            value += factor;
        factor /= 2;
    }
    if (neg)
        value = -value;
    precis = newPrecis;
    bits = convert_to_binary(value, newPrecis, (value < 0));
}

// Вспомогательная функция для вывода бинарного представления числа
void printBinary(const std::vector<bool>& bits, int precis, bool neg = false) {
    if (neg)
        std::cout << "-";
    int intBitsCount = bits.size() - precis;
    if (intBitsCount <= 0)
        std::cout << "0";
    else {
        for (int i = 0; i < intBitsCount; ++i)
            std::cout << bits[i];
    }
    if (precis > 0) {
        std::cout << ".";
        for (size_t i = intBitsCount; i < bits.size(); ++i)
            std::cout << bits[i];
    }
}

int main() {
    std::cout << "=== Тест конвертации из long double в двоичный вид ===" << std::endl;
    {
        long double number = 10.625;
        int precision = 3;
        bool isNegative = (number < 0);
        LongNumber ln(number, precision, isNegative);
        std::vector<bool> binary = ln.convert_to_binary(number, precision, isNegative);
        
        std::cout << "Десятичное число: " << number << "\nДвоичное представление: ";
        printBinary(binary, precision, isNegative);
        std::cout << std::endl;
        
        long double decimalValue = convertBinaryToDecimal(binary, precision);
        std::cout << "Обратно в десятичном виде: " << decimalValue << std::endl;
    }

    std::cout << "\n=== Тест арифметических операций (через long double) ===" << std::endl;
    {
        long double num1 = 15.75, num2 = 3.125;
        int precision = 4;
        LongNumber ln1(num1, precision, num1 < 0);
        LongNumber ln2(num2, precision, num2 < 0);
        
        // Сложение
        LongNumber sum = ln1 + ln2;
        std::cout << num1 << " + " << num2 << " = ";
        printBinary(sum.bits, sum.precis, sum.neg);
        std::cout << "  (в двоичном виде)" << std::endl;
        std::cout << "В десятичном виде: " << convertBinaryToDecimal(sum.bits, sum.precis) << std::endl;
        
        // Вычитание
        LongNumber diff = ln1 - ln2;
        std::cout << num1 << " - " << num2 << " = ";
        printBinary(diff.bits, diff.precis, diff.neg);
        std::cout << "  (в двоичном виде)" << std::endl;
        std::cout << "В десятичном виде: " << convertBinaryToDecimal(diff.bits, diff.precis) << std::endl;
        
        // Умножение
        LongNumber prod = ln1 * ln2;
        std::cout << num1 << " * " << num2 << " = ";
        printBinary(prod.bits, prod.precis, prod.neg);
        std::cout << "  (в двоичном виде)" << std::endl;
        std::cout << "В десятичном виде: " << convertBinaryToDecimal(prod.bits, prod.precis) << std::endl;
        
        // Деление
        try {
            LongNumber quot = ln1 / ln2;
            std::cout << num1 << " / " << num2 << " = ";
            printBinary(quot.bits, quot.precis, quot.neg);
            std::cout << "  (в двоичном виде)" << std::endl;
            std::cout << "В десятичном виде: " << convertBinaryToDecimal(quot.bits, quot.precis) << std::endl;
        }
        catch (const std::runtime_error &e) {
            std::cout << "Ошибка при делении: " << e.what() << std::endl;
        }
    }

    std::cout << "\n=== Тест конструктора из строки ===" << std::endl;
    {
        // Пример числа, которое может превышать возможности long double
        std::string testStr = "-12345678901234567890.987654321";
        int precision = 20;
        LongNumber lnFromStr(testStr, precision);
        
        std::cout << "Строка: " << testStr << std::endl;
        std::cout << "Бинарное представление: ";
        printBinary(lnFromStr.bits, lnFromStr.precis, lnFromStr.neg);
        std::cout << std::endl;
        std::cout << "Обратно в десятичном виде (с ограниченной точностью): "
                  << convertBinaryToDecimal(lnFromStr.bits, lnFromStr.precis)
                  << std::endl;
    }

    std::cout << "\n=== Тест арифметических операций для чисел, созданных из строк ===" << std::endl;
    {
        // Создадим два числа из строкового представления
        std::string s1 = "12345678901234567890.123456789";
        std::string s2 = "98765432109876543210.987654321";
        int precision = 30;
        LongNumber lnStr1(s1, precision);
        LongNumber lnStr2(s2, precision);
        
        // Сложение
        LongNumber sumStr = lnStr1 + lnStr2;
        std::cout << s1 << " + " << s2 << " = ";
        printBinary(sumStr.bits, sumStr.precis, sumStr.neg);
        std::cout << "  (в двоичном виде)" << std::endl;
        std::cout << "В десятичном виде: " << convertBinaryToDecimal(sumStr.bits, sumStr.precis) << std::endl;
        
        // Вычитание
        LongNumber diffStr = lnStr2 - lnStr1;
        std::cout << s2 << " - " << s1 << " = ";
        printBinary(diffStr.bits, diffStr.precis, diffStr.neg);
        std::cout << "  (в двоичном виде)" << std::endl;
        std::cout << "В десятичном виде: " << convertBinaryToDecimal(diffStr.bits, diffStr.precis) << std::endl;
        
        // Умножение
        LongNumber prodStr = lnStr1 * lnStr2;
        std::cout << s1 << " * " << s2 << " = ";
        printBinary(prodStr.bits, prodStr.precis, prodStr.neg);
        std::cout << "  (в двоичном виде)" << std::endl;
        std::cout << "В десятичном виде: " << convertBinaryToDecimal(prodStr.bits, prodStr.precis) << std::endl;
        
        // Деление (если возможно)
        try {
            LongNumber quotStr = lnStr2 / lnStr1;
            std::cout << s2 << " / " << s1 << " = ";
            printBinary(quotStr.bits, quotStr.precis, quotStr.neg);
            std::cout << "  (в двоичном виде)" << std::endl;
            std::cout << "В десятичном виде: " << convertBinaryToDecimal(quotStr.bits, quotStr.precis) << std::endl;
        }
        catch (const std::runtime_error &e) {
            std::cout << "Ошибка при делении: " << e.what() << std::endl;
        }
    }

    std::cout << "\n=== Тест изменения точности (new_precis) ===" << std::endl;
    {
        long double number = 3.1415;
        int oldPrecision = 5;
        LongNumber ln(number, oldPrecision, number < 0);
        
        std::cout << "Исходное число с точностью " << oldPrecision << ": ";
        printBinary(ln.bits, ln.precis, ln.neg);
        std::cout << std::endl;
        std::cout << "В десятичном виде: " << convertBinaryToDecimal(ln.bits, ln.precis) << std::endl;
        
        int newPrecision = 8;
        ln.new_precis(newPrecision);
        std::cout << "После изменения точности до " << newPrecision << ": ";
        printBinary(ln.bits, ln.precis, ln.neg);
        std::cout << std::endl;
        std::cout << "В десятичном виде: " << convertBinaryToDecimal(ln.bits, ln.precis) << std::endl;
    }
    
    return 0;
}
