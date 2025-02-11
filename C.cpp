#include <iostream>
#include "C.h"

int main() {
    std::string original = "Hello World";
    std::string encoded = Hex::encode(original);
    std::string decoded = Hex::decode(encoded);
    std::cout << "Original: " << original << std::endl;
    std::cout << "Encoded:  " << encoded << std::endl;
    std::cout << "Decoded:  " << decoded << std::endl;
    return 0;
}
