#ifndef KARATSUBA_H
#define KARATSUBA_H

#include <string>
#include <utility>

// ============================================================================
// Utilidades de normalización, manipulación y partición de cadenas
// ============================================================================
std::string quitarCerosIzquierda(const std::string& A);
size_t rellenarPotenciaDeDos(std::string& A, std::string& B);
std::pair<std::string, std::string> partir(const std::string& A, size_t m);

// ============================================================================
// Operaciones aritméticas auxiliares y método clásico
// ============================================================================
std::string sumar(const std::string& A, const std::string& B);
std::string restar(const std::string& A, const std::string& B); // Precondición: A >= B
std::string desplazar(const std::string& A, size_t k);
std::string multiplicacionClasica(const std::string& A, const std::string& B);

// ============================================================================
// Algoritmo de multiplicación rápida (Divide y Vencerás - Karatsuba)
// ============================================================================
std::string karatsuba(const std::string& A, const std::string& B);

#endif // KARATSUBA_H