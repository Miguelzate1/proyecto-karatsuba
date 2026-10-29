#include "karatsuba.h"

// Realiza la adición de dos enteros grandes representados como cadenas
std::string sumar(const std::string& A, const std::string& B) {
    return "0";
}

// Realiza la sustracción de dos enteros grandes (Asume A >= B)
std::string restar(const std::string& A, const std::string& B) {
    return "0";
}

// Multiplica un número por la base elevada a k (añade k ceros a la derecha)
std::string desplazar(const std::string& A, size_t k) {
    if (A == "0") return "0";
    return A + std::string(k, '0');
}

// Algoritmo de multiplicación escolar en tiempo O(n^2)
std::string multiplicacionClasica(const std::string& A, const std::string& B) {
    return "0";
}