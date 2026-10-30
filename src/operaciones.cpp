#include "karatsuba.h"
#include <stdexcept>
#include <vector>

namespace {
std::string normalizar(const std::string& numero) {
    if (numero.empty()) {
        throw std::invalid_argument("El numero no puede estar vacio");
    }

    for (char digito : numero) {
        if (digito < '0' || digito > '9') {
            throw std::invalid_argument("El numero debe contener solo digitos");
        }
    }

    size_t primerDigitoSignificativo = numero.find_first_not_of('0');
    if (primerDigitoSignificativo == std::string::npos) {
        return "0";
    }
    return numero.substr(primerDigitoSignificativo);
}

bool esMenor(const std::string& A, const std::string& B) {
    return A.length() < B.length() ||
           (A.length() == B.length() && A < B);
}
}

// Realiza la adición de dos enteros grandes representados como cadenas
std::string sumar(const std::string& A, const std::string& B) {
    const std::string numeroA = normalizar(A);
    const std::string numeroB = normalizar(B);
    size_t i = numeroA.length();
    size_t j = numeroB.length();
    int acarreo = 0;
    std::string resultado;

    while (i > 0 || j > 0 || acarreo > 0) {
        int suma = acarreo;
        if (i > 0) {
            suma += numeroA[--i] - '0';
        }
        if (j > 0) {
            suma += numeroB[--j] - '0';
        }
        resultado.push_back(static_cast<char>('0' + suma % 10));
        acarreo = suma / 10;
    }

    for (size_t inicio = 0, fin = resultado.length() - 1; inicio < fin; ++inicio, --fin) {
        std::swap(resultado[inicio], resultado[fin]);
    }
    return resultado;
}

// Realiza la sustracción de dos enteros grandes (Asume A >= B)
std::string restar(const std::string& A, const std::string& B) {
    const std::string numeroA = normalizar(A);
    const std::string numeroB = normalizar(B);
    if (esMenor(numeroA, numeroB)) {
        throw std::invalid_argument("La resta requiere que A sea mayor o igual que B");
    }

    size_t i = numeroA.length();
    size_t j = numeroB.length();
    int prestamo = 0;
    std::string resultado;

    while (i > 0) {
        int diferencia = numeroA[--i] - '0' - prestamo;
        if (j > 0) {
            diferencia -= numeroB[--j] - '0';
        }
        if (diferencia < 0) {
            diferencia += 10;
            prestamo = 1;
        } else {
            prestamo = 0;
        }
        resultado.push_back(static_cast<char>('0' + diferencia));
    }

    while (resultado.length() > 1 && resultado.back() == '0') {
        resultado.pop_back();
    }
    for (size_t inicio = 0, fin = resultado.length() - 1; inicio < fin; ++inicio, --fin) {
        std::swap(resultado[inicio], resultado[fin]);
    }
    return resultado;
}

// Multiplica un número por la base elevada a k (añade k ceros a la derecha)
std::string desplazar(const std::string& A, size_t k) {
    const std::string numero = normalizar(A);
    if (numero == "0") {
        return "0";
    }
    return numero + std::string(k, '0');
}

// Algoritmo de multiplicación escolar en tiempo O(n^2)
std::string multiplicacionClasica(const std::string& A, const std::string& B) {
    const std::string numeroA = normalizar(A);
    const std::string numeroB = normalizar(B);
    if (numeroA == "0" || numeroB == "0") {
        return "0";
    }

    std::vector<int> digitos(numeroA.length() + numeroB.length(), 0);
    for (size_t i = numeroA.length(); i > 0; --i) {
        int acarreo = 0;
        for (size_t j = numeroB.length(); j > 0; --j) {
            size_t posicion = i + j - 1;
            int producto = (numeroA[i - 1] - '0') * (numeroB[j - 1] - '0')
                         + digitos[posicion] + acarreo;
            digitos[posicion] = producto % 10;
            acarreo = producto / 10;
        }
        digitos[i - 1] = acarreo;
    }

    std::string resultado;
    size_t primerDigitoSignificativo = 0;
    while (primerDigitoSignificativo < digitos.size() &&
           digitos[primerDigitoSignificativo] == 0) {
        ++primerDigitoSignificativo;
    }
    for (; primerDigitoSignificativo < digitos.size(); ++primerDigitoSignificativo) {
        resultado.push_back(static_cast<char>('0' + digitos[primerDigitoSignificativo]));
    }
    return resultado;
}