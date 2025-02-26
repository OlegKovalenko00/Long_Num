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

public:
    static LongNumber calculate_pi(int precision);
    
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

    // Функция для округления до заданной точности
    void round_to_precision();

    // Функция для вывода битового представления
    void printk_binary(const std::vector<bool>& bits, int precision, bool is_negative) const;

    // Дружественная функция для перегрузки оператора <<
    friend std::ostream& operator<<(std::ostream& os, const LongNumber& num);
};

// Пользовательский литерал для создания LongNumber (должен быть не-членом класса)
LongNumber operator"" _longnum(long double number);

#endif