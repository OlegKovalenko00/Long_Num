#include "head.hpp"
#include "time.hpp"
#include <iostream>
#include <string>
#include <iomanip>
#include <cmath>


int main(int argc, char *argv[])
{
    LongNumber pi = calculate_pi(precision * 4);
    std::cout << pi << std::endl;
    return 0;
}