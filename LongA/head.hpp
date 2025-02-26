#ifndef LONGNUM_HPP
#define LONGNUM_HPP
#pragma once

#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>

class LongNumber {
private:
    std::vector<bool> bit_vector_;  // Битовое представление числа
    int precision_;                 // Количество битов после запятой
    bool is_negative_;              // Знак числа

    // Функция выравнивания битовых векторов двух чисел (для арифметических операций)
    friend void align_bit_vectors(const LongNumber &num1, const LongNumber &num2,
                                    std::vector<bool> &a, std::vector<bool> &b,
                                    int &new_int_len, int &new_frac_len);

public:
        
    // Новый конструктор для целых чисел
    LongNumber(int value, int precision);

    // Статический метод для вычисления π
    static LongNumber calculate_pi(int precision);
    void printk_binary(const std::vector<bool> &bit_vec, int precision, bool is_negative);
    // Геттеры для отладки (возвращают приватные члены)
    std::vector<bool> get_bit_vector() const { return bit_vector_; }
    int get_precision() const { return precision_; }
    bool isNegative() const { return is_negative_; }
    static long double longNumberToLongDouble(const LongNumber &num);
    void round_to_precision();
    // Функция перевода числа из long double в вектор битов
    std::vector<bool> convert_to_binary(long double number, int precision, bool is_negative);

    // Конструктор (число типа long double)
    LongNumber(long double number, int precision, bool is_negative);

    // Конструктор (число, заданное строкой)
    LongNumber(const std::string &str, int precision);

    // Конструктор копирования
    LongNumber(const LongNumber &other);

    // Оператор присваивания
    LongNumber& operator=(const LongNumber &other);

    // Деструктор
    ~LongNumber();

    // Арифметические операции
    LongNumber operator+(const LongNumber &other) const;
    LongNumber operator-(const LongNumber &other) const;
    LongNumber operator*(const LongNumber &other) const;
    LongNumber operator/(const LongNumber &other) const;

    // Операторы сравнения
    bool operator==(const LongNumber &other) const;
    bool operator!=(const LongNumber &other) const;
    bool operator<(const LongNumber &other) const;
    bool operator>(const LongNumber &other) const;
    bool operator<=(const LongNumber &other) const;
    bool operator>=(const LongNumber &other) const;

    // Функция для изменения точности представления числа
    void new_precision(int new_precision);
};

// Пользовательский литерал для создания LongNumber (должен быть не-членом класса)
LongNumber operator"" _longnum(long double number);

#endif
