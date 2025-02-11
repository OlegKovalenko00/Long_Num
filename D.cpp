#include <iostream>
#include <cstdint>

enum class DataType : uint8_t {
    Int = 0,
    Float = 1,
    String = 2,
    Timestamp = 3
};

enum class CastType : uint8_t {
    Implicit = 0,
    Explicit = 1,
    None = 2
};

CastType get_cast_type(DataType from, DataType to) {
    if (from == DataType::Int) {
        if (to == DataType::Int) return CastType::Implicit;
        if (to == DataType::Float) return CastType::Implicit;
        if (to == DataType::String) return CastType::Explicit;
        if (to == DataType::Timestamp) return CastType::Explicit;
    }
    
    if (from == DataType::Float) {
        if (to == DataType::Int) return CastType::Explicit;
        if (to == DataType::Float) return CastType::Implicit;
        if (to == DataType::String) return CastType::Explicit;
        if (to == DataType::Timestamp) return CastType::None;
    }
    
    if (from == DataType::String) {
        if (to == DataType::Int) return CastType::None;
        if (to == DataType::Float) return CastType::None;
        if (to == DataType::String) return CastType::Implicit;
        if (to == DataType::Timestamp) return CastType::None;
    }
    
    if (from == DataType::Timestamp) {
        if (to == DataType::Int) return CastType::Explicit;
        if (to == DataType::Float) return CastType::None;
        if (to == DataType::String) return CastType::Explicit;
        if (to == DataType::Timestamp) return CastType::Implicit;
    }

    return CastType::None;
}

/*
int main() {
    DataType from = DataType::Int;
    DataType to = DataType::String;
    CastType result = get_cast_type(from, to);

    std::cout << "Cast result: " << static_cast<int>(result) << std::endl;

    return 0;
}
*/
