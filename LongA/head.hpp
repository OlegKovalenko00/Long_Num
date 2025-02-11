#ifndef LongNum_HPP
#define LongNum_HPP
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
        std::vector <bool> bits; // Число
        int precis; // Кол-во битов после точки
        bool neg; // Знак

    public:

        // Функция перевода числа из double в vector
        std::vector <bool> convert_to_binary(long double number, int precision, bool neg);

        // Конструктор (число типа long double)
        LongNumber(long double number, int precision, bool neg);

        // Новый перегруженный конструктор (число передаётся в виде строки)
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

        // Операции сравнения
        bool operator==(const LongNumber &other) const;
        bool operator!=(const LongNumber &other) const;
        bool operator<(const LongNumber &other) const;
        bool operator>(const LongNumber &other) const;
        bool operator<=(const LongNumber &other) const;
        bool operator>=(const LongNumber &other) const;

        // Изменение кол-ва разрядов после запятой
        void new_precis(int newPrecis);
};

#endif
