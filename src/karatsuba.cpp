#include "karatsuba.h"
#include <algorithm>

// Tamano (en digitos) por debajo del cual se usa multiplicacion directa.
// Con 1 se cumple el caso base minimo del enunciado; con 32 se reduce el
// costo de las copias de strings en los niveles bajos del arbol (ver informe).
#ifndef UMBRAL
#define UMBRAL 32
#endif

// Elimina los ceros redundantes al inicio ("000123" -> "123"; "000" -> "0").
std::string quitarCerosIzquierda(const std::string& A) {
    size_t i = 0;
    while (i + 1 < A.length() && A[i] == '0') {
        i++;
    }
    return A.substr(i);
}

// Siguiente potencia de 2 mayor o igual a n.
static size_t siguientePotenciaDeDos(size_t n) {
    size_t p = 1;
    while (p < n) {
        p *= 2;
    }
    return p;
}

// Rellena ambas cadenas con ceros a la izquierda hasta la misma potencia de 2.
size_t rellenarPotenciaDeDos(std::string& A, std::string& B) {
    std::string limpioA = quitarCerosIzquierda(A);
    std::string limpioB = quitarCerosIzquierda(B);
    size_t n = siguientePotenciaDeDos(std::max(limpioA.length(), limpioB.length()));
    A = std::string(n - limpioA.length(), '0') + limpioA;
    B = std::string(n - limpioB.length(), '0') + limpioB;
    return n;
}

// Divide A en mitad alta (x1) y baja (x0): A = x1 * 10^m + x0.
std::pair<std::string, std::string> partir(const std::string& A, size_t m) {
    size_t len = A.length();
    if (len <= m) {
        return {"0", quitarCerosIzquierda(A)};
    }
    std::string x1 = A.substr(0, len - m);
    std::string x0 = A.substr(len - m);
    return {quitarCerosIzquierda(x1), quitarCerosIzquierda(x0)};
}

// Recursion de Karatsuba. NO vuelve a rellenar a potencia de 2: trabaja con la
// longitud real de los operandos. Asi, cuando x1 + x0 produce m + 1 digitos,
// el subproblema P3 sigue siendo de tamano ~n/2 y no salta de nuevo a n.
static std::string karatsubaRec(const std::string& a, const std::string& b) {
    size_t n = std::max(a.length(), b.length());

    // Caso base: multiplicacion directa
    if (n <= UMBRAL) {
        if (UMBRAL == 1) {
            return std::to_string((a[0] - '0') * (b[0] - '0'));
        }
        return multiplicacionClasica(a, b);
    }

    size_t m = n / 2;
    auto [x1, x0] = partir(a, m);
    auto [y1, y0] = partir(b, m);

    // Las 3 llamadas recursivas
    std::string P1 = karatsubaRec(x1, y1);                        // x1 * y1
    std::string P2 = karatsubaRec(x0, y0);                        // x0 * y0
    std::string P3 = karatsubaRec(sumar(x1, x0), sumar(y1, y0));  // (x1+x0)(y1+y0)

    // Orden de resta de la seccion 3: (P3 - P1) - P2, siempre no negativo
    std::string terminoCentral = restar(restar(P3, P1), P2);

    std::string parte1 = desplazar(P1, 2 * m);
    std::string parte2 = desplazar(terminoCentral, m);
    return sumar(sumar(parte1, parte2), P2);
}

// Punto de entrada: rellena UNA sola vez (seccion 3, regla 2) y recurre.
std::string karatsuba(const std::string& A, const std::string& B) {
    std::string a = A;
    std::string b = B;
    rellenarPotenciaDeDos(a, b);
    return karatsubaRec(a, b);
}

