#include "karatsuba.h"
#include <algorithm>
#include <cmath>

// Elimina los ceros redundantes al inicio de una cadena numérica (ej. "000123" -> "123").
// Si el número es completamente "000...", devuelve "0".
std::string quitarCerosIzquierda(const std::string& A) {
    size_t i = 0;
    while (i < A.length() - 1 && A[i] == '0') {
        i++;
    }
    return A.substr(i);
}

// Encuentra la siguiente potencia de 2 mayor o igual a un número dado
static size_t siguientePotenciaDeDos(size_t n) {
    size_t p = 1;
    while (p < n) {
        p *= 2;
    }
    return p;
}

// Rellena ambas cadenas con ceros a la izquierda hasta igualar su longitud
// a la misma potencia de 2, cumpliendo con las reglas de diseño del proyecto.
size_t rellenarPotenciaDeDos(std::string& A, std::string& B) {
    std::string limpioA = quitarCerosIzquierda(A);
    std::string limpioB = quitarCerosIzquierda(B);
    
    size_t max_len = std::max(limpioA.length(), limpioB.length());
    size_t n = siguientePotenciaDeDos(max_len);
    if (n == 0) n = 1;

    A = std::string(n - limpioA.length(), '0') + limpioA;
    B = std::string(n - limpioB.length(), '0') + limpioB;
    
    return n;
}

// Divide una cadena A de longitud n en su mitad alta (x1) y baja (x0) respecto a la posición m.
// Matemáticamente: A = x1 * 10^m + x0, donde m es la cantidad de dígitos de la mitad baja.
std::pair<std::string, std::string> partir(const std::string& A, size_t m) {
    size_t len = A.length();
    if (len <= m) {
        return {"0", A};
    }
    std::string x1 = A.substr(0, len - m);
    std::string x0 = A.substr(len - m);
    return {quitarCerosIzquierda(x1), quitarCerosIzquierda(x0)};
}

// Implementación recursiva del algoritmo de multiplicación de Karatsuba O(n^log2(3))
std::string karatsuba(const std::string& A, const std::string& B) {
    std::string a_copia = A;
    std::string b_copia = B;
    
    size_t n = rellenarPotenciaDeDos(a_copia, b_copia);

    // Caso base: si la longitud es 1 o menor, multiplicación directa
    if (n <= 1) {
        long long valA = std::stoll(a_copia);
        long long valB = std::stoll(b_copia);
        return std::to_string(valA * valB);
    }

    size_t m = n / 2;

    // Partición en mitades altas y bajas
    auto [x1, x0] = partir(a_copia, m);
    auto [y1, y0] = partir(b_copia, m);

    // Las 3 llamadas recursivas de Karatsuba
    std::string P1 = karatsuba(x1, y1);                                  // P1 = x1 * y1
    std::string P2 = karatsuba(x0, y0);                                  // P2 = x0 * y0
    std::string P3 = karatsuba(sumar(x1, x0), sumar(y1, y0));            // P3 = (x1 + x0) * (y1 + y0)

    // Recomposición algebraica respetando el orden estricto de resta: (P3 - P1) - P2
    std::string termino_central = restar(restar(P3, P1), P2);

    std::string parte1 = desplazar(P1, 2 * m);
    std::string parte2 = desplazar(termino_central, m);

    return sumar(sumar(parte1, parte2), P2);
}