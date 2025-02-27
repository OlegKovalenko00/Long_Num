#include "head.hpp"
#include <iostream>
#include <string>

int main() {
    std::cout << "=== Тесты арифметических операций с длинными числами ===" << std::endl;
    
    // Тест 1: Сложение двух положительных чисел с разной длиной дробной части
    LongNumber t1("123.456", 50);
    LongNumber t2("789.123", 50);
    LongNumber res1 = t1 + t2;
    std::cout << t1.to_string() << " + " << t2.to_string() << " = " 
              << res1.to_string() << " (ожидается примерно: 912.579)" << std::endl;
    
    // Тест 2: Вычитание: меньшее число из большего (результат отрицательный)
    LongNumber t3("50.25", 50);
    LongNumber t4("100.75", 50);
    LongNumber res2 = t3 - t4;
    std::cout << t3.to_string() << " - " << t4.to_string() << " = " 
              << res2.to_string() << " (ожидается примерно: -50.50)" << std::endl;
    
    // Тест 3: Сложение двух отрицательных чисел
    LongNumber t5("-200.200", 50);
    LongNumber t6("-300.300", 50);
    LongNumber res3 = t5 + t6;
    std::cout << t5.to_string() << " + " << t6.to_string() << " = " 
              << res3.to_string() << " (ожидается примерно: -500.500)" << std::endl;
    
    // Тест 4: Вычитание отрицательного числа из положительного (эквивалент сложения)
    LongNumber t7("400.400", 50);
    LongNumber t8("-100.50", 50);
    LongNumber res4 = t7 - t8;
    std::cout << t7.to_string() << " - (" << t8.to_string() << ") = " 
              << res4.to_string() << " (ожидается примерно: 500.900)" << std::endl;
    
    // Тест 5: Умножение двух положительных чисел
    LongNumber t9("12.34", 50);
    LongNumber t10("56.78", 50);
    LongNumber res5 = t9 * t10;
    std::cout << t9.to_string() << " * " << t10.to_string() << " = " 
              << res5.to_string() << " (ожидается примерно: 700.6652)" << std::endl;
    
    // Тест 6: Умножение положительного и отрицательного числа
    LongNumber t11("123.456", 50);
    LongNumber t12("-7.89", 50);
    LongNumber res6 = t11 * t12;
    std::cout << t11.to_string() << " * " << t12.to_string() << " = " 
              << res6.to_string() << " (ожидается примерно: -974.06784)" << std::endl;
    
    // Тест 7: Умножение двух отрицательных чисел (результат положительный)
    LongNumber t13("-45.67", 50);
    LongNumber t14("-89.01", 50);
    LongNumber res7 = t13 * t14;
    std::cout << t13.to_string() << " * " << t14.to_string() << " = " 
              << res7.to_string() << " (ожидается примерно: 4061.2867)" << std::endl;
    
    // Тест 8: Деление двух положительных чисел
    LongNumber t15("1000.00", 50);
    LongNumber t16("25.00", 50);
    LongNumber res8 = t15 / t16;
    std::cout << t15.to_string() << " / " << t16.to_string() << " = " 
              << res8.to_string() << " (ожидается примерно: 40.00)" << std::endl;
    
    // Тест 9: Деление положительного числа на отрицательное (результат отрицательный)
    LongNumber t17("500.50", 50);
    LongNumber t18("-2.50", 50);
    LongNumber res9 = t17 / t18;
    std::cout << t17.to_string() << " / " << t18.to_string() << " = " 
              << res9.to_string() << " (ожидается примерно: -200.20)" << std::endl;
    
    // Тест 50: Деление двух отрицательных чисел (результат положительный)
    LongNumber t19("-144.00", 50);
    LongNumber t20("-12.00", 50);
    LongNumber res10 = t19 / t20;
    std::cout << t19.to_string() << " / " << t20.to_string() << " = " 
              << res10.to_string() << " (ожидается примерно: 12.00)" << std::endl;
    
    // Тест 11: Сложение чисел с высокой точностью
    LongNumber t21("123456789.987654321", 20);
    LongNumber t22("987654321.123456789", 20);
    LongNumber res11 = t21 + t22;
    std::cout << t21.to_string() << " + " << t22.to_string() << " = " 
              << res11.to_string() << " (ожидается примерно: 1111111111.111111110)" << std::endl;
    
    // Тест 12: Вычитание чисел с высокой точностью
    LongNumber t23("1000000000.000000001", 20);
    LongNumber t24("0.000000001", 20);
    LongNumber res12 = t23 - t24;
    std::cout << t23.to_string() << " - " << t24.to_string() << " = "
              << res12.to_string() << " (ожидается примерно: 1000000000.000000000)" << std::endl;
    
    return 0;
}