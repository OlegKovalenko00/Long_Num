#include "head.hpp"
#include <iostream>
#include <cassert>
#include <stdexcept>

int main() {
    // Создание чисел из строки с фиксированной точностью (например, 10 битов для дробной части)
    LongNumber num1("123.456", 100);
    LongNumber num2("654.321", 100);

    // Тест сложения
    LongNumber sum = num1 + num2;
    std::cout << "Sum:         " << sum << std::endl;
    // Ожидаемый результат (вычислено вручную):
    // num1 = 123.456 -> 1111011.0111010010
    // num2 = 654.321 -> 1010001110.0101001000
    // Сумма  = 777.777  -> 1100001001.1100011010
    std::cout << "Expected Sum:         1100001001.1100011010" << std::endl;
    std::cout << std::endl;

    // Тест вычитания
    LongNumber diff = num2 - num1;
    std::cout << "Difference:  " << diff << std::endl;
    // Ожидаемый результат:
    // Разность = 654.321 - 123.456 = 530.865
    // Ожидаемая битовая запись (после заимствования) = 1000010010.1101110110
    std::cout << "Expected Difference:  1000010010.1101110110" << std::endl;
    std::cout << std::endl;

    // Тест умножения
    LongNumber prod = num1 * num2;
    std::cout << "Product:     " << prod << std::endl;
    // Для произведения ожидаемый битовый массив можно вычислить отдельно.
    // Например, если 123.456 * 654.321 ≈ 80773.383, то ожидаемый результат (с округлением и учетом фиксированного масштаба)
    // может выглядеть так (пример): 10011101110110101.XXXXXX (здесь X – дробные биты)
    std::cout << "Expected Product:     <expected bit array>" << std::endl;
    std::cout << std::endl;

    // Тест деления
    try {
        LongNumber quot = num2 / num1;
        std::cout << "Quotient:    " << quot << std::endl;
        // Аналогично для частного – рассчитанное значение следует сравнить с ожидаемым битовым представлением.
        std::cout << "Expected Quotient:    <expected bit array>" << std::endl;
    } catch (const std::runtime_error &e) {
        std::cerr << "Division error: " << e.what() << std::endl;
    }
    std::cout << std::endl;

    // Тест копирующего конструктора и оператора присваивания
    LongNumber copy = num1;
    assert(copy == num1);
    LongNumber assigned(0.0, 100, false);
    assigned = num2;
    assert(assigned == num2);
    std::cout << "Copy of num1:        " << copy << std::endl;
    std::cout << "Expected Copy:       1111011.0111010010" << std::endl;
    std::cout << "Assigned num2:       " << assigned << std::endl;
    std::cout << "Expected Assigned:   1010001110.0101001000" << std::endl;
    std::cout << std::endl;

    // Тест операторов сравнения
    if (num1 < num2) {
        std::cout << "num1 is less than num2" << std::endl;
    } else {
        std::cout << "num1 is not less than num2" << std::endl;
    }
    std::cout << std::endl;

    // Тест изменения точности
    std::cout << "Original num1:       " << num1 << std::endl;
    num1.new_precision(15);
    std::cout << "num1 with new precision: " << num1 << std::endl;
    std::cout << "Expected new num1:   <expected bit array with 15 fractional bits>" << std::endl;
    std::cout << std::endl;

    // Тест пользовательского литерала
    LongNumber literalTest = 3.1415926535_longnum;
    std::cout << "User literal:        " << literalTest << std::endl;
    std::cout << "Expected literal:    <expected bit array>" << std::endl;
    return 0;
}
