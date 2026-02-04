#pragma once
#include <string_view>
#include <string>

enum class VType { Unknown, Number, String, Array, Function, Module };

inline VType stringToVType(std::string_view name) {
    if (name == "Array")  return VType::Array;
    if (name == "Number") return VType::Number;
    if (name == "String") return VType::String;
    return VType::Unknown;
}

inline std::string VTypeToString(VType type) {
    switch (type) {
        case VType::Array:   return "Array";
        case VType::String:  return "String";
        case VType::Number:  return "Number";
        case VType::Function:return "Function";
        case VType::Module:  return "Module";
        default:             return "Unknown";
    }
}