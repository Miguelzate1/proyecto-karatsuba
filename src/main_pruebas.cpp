// src/main_pruebas.cpp
//
// Integrante 3: bateria de pruebas de correctitud + benchmark de tiempos.
//
// Que hace:
//   1) Valida karatsuba() contra multiplicacionClasica() en los 5 grupos de
//      casos obligatorios del enunciado (1 digito, ceros a la izquierda,
//      ceros intercalados, longitudes impares, numeros grandes). Varios casos
//      ademas se comparan contra un resultado esperado calculado de forma
//      independiente, para no validar el codigo solo contra si mismo.
//   2) Mide tiempos de ambos algoritmos para n = 2, 4, 8, ... , 4096 digitos
//      con std::chrono::steady_clock, promediando repeticiones y consumiendo
//      el resultado para que el compilador no elimine el calculo.
//   3) Escribe resultados/tiempos.csv y reporta el punto de cruce.
//
// Compilar (desde la raiz del repo; ajustar la lista de .cpp a la real):
//   g++ -std=c++17 -O2 -Iinclude src/main_pruebas.cpp src/operaciones.cpp \
//       src/karatsuba.cpp -o pruebas
// Ejecutar:
//   ./pruebas                 (escribe resultados/tiempos.csv)
//   ./pruebas otra/ruta.csv   (escribe en otra ruta)
//
// SUPUESTO sobre karatsuba.h: los numeros son std::string de digitos en base
// 10, digito mas significativo primero, y las funciones tienen la forma
//   std::string multiplicacionClasica(const std::string&, const std::string&);
//   std::string karatsuba(const std::string&, const std::string&);

#include "karatsuba.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>

namespace {

using Reloj = std::chrono::steady_clock;

// ---------------------------------------------------------------------
// Utilidades propias (no dependen del codigo de los companeros)
// ---------------------------------------------------------------------

// Quita ceros a la izquierda (deja "0" si todo eran ceros).
std::string normalizar(const std::string& s) {
    std::size_t i = 0;
    while (i + 1 < s.size() && s[i] == '0') ++i;
    return s.empty() ? std::string("0") : s.substr(i);
}

// Numero aleatorio de exactamente n digitos (primer digito distinto de 0).
std::string generarNumero(std::size_t n, std::mt19937_64& rng) {
    std::uniform_int_distribution<int> dig(0, 9), primero(1, 9);
    std::string s(n, '0');
    s[0] = static_cast<char>('0' + primero(rng));
    for (std::size_t i = 1; i < n; ++i) s[i] = static_cast<char>('0' + dig(rng));
    return s;
}

// ---------------------------------------------------------------------
// 1) VALIDACION DE CORRECTITUD
// ---------------------------------------------------------------------

struct Caso {
    std::string grupo;      // a cual de los 5 grupos obligatorios pertenece
    std::string a, b;
    std::string esperado;   // "" = sin resultado externo conocido
};

// --- Oraculo independiente: no usa ninguno de los dos algoritmos ---
// Valor del numero decimal s modulo p, leyendo los digitos con Horner.
std::uint64_t modStr(const std::string& s, std::uint64_t p) {
    std::uint64_t r = 0;
    for (char c : s) r = (r * 10 + static_cast<std::uint64_t>(c - '0')) % p;
    return r;
}

// C es "coherente" con A y B si solo tiene digitos y cumple
// (A*B) mod p == C mod p para dos primos distintos. Un resultado equivocado casi
// nunca pasa esto. Asi evitamos el falso acierto de que ambos algoritmos
// devuelvan el mismo valor equivocado (por ejemplo, los dos "0").
bool coherente(const std::string& a, const std::string& b, const std::string& c) {
    if (c.empty()) return false;
    for (char ch : c)
        if (ch < '0' || ch > '9') return false;
    for (std::uint64_t p : {1000000007ULL, 998244353ULL})
        if ((modStr(a, p) * modStr(b, p)) % p != modStr(c, p)) return false;
    return true;
}

struct ResumenGrupo { int total = 0, clasicaMal = 0, karatsubaMal = 0; };
std::map<std::string, ResumenGrupo> resumen;
std::vector<std::string> ordenGrupos;
int totalCasos = 0, fallidos = 0, totClasicaMal = 0, totKaratsubaMal = 0;
int detallesMostrados = 0;
constexpr int MAX_DETALLES = 8;  // evita inundar la consola si todo falla

void ejecutarCaso(const Caso& c) {
    ++totalCasos;
    ResumenGrupo& g = resumen[c.grupo];
    if (g.total == 0) ordenGrupos.push_back(c.grupo);
    ++g.total;

    const std::string clasica = normalizar(multiplicacionClasica(c.a, c.b));
    const std::string kara = normalizar(karatsuba(c.a, c.b));
    const std::string esp = c.esperado.empty() ? std::string() : normalizar(c.esperado);

    // Cada algoritmo se juzga por separado contra el oraculo independiente
    // (y contra el resultado esperado, cuando existe).
    const bool clasicaOk = coherente(c.a, c.b, clasica) && (esp.empty() || clasica == esp);
    const bool karaOk = coherente(c.a, c.b, kara) && (esp.empty() || kara == esp);

    if (!clasicaOk) { ++g.clasicaMal; ++totClasicaMal; }
    if (!karaOk) { ++g.karatsubaMal; ++totKaratsubaMal; }
    if (clasicaOk && karaOk) return;

    ++fallidos;
    if (detallesMostrados++ >= MAX_DETALLES) return;

    auto cortar = [](const std::string& s) {
        return s.size() > 60 ? s.substr(0, 57) + "..." : s;
    };
    std::cout << "  [FALLA] " << c.grupo
              << (clasicaOk ? "" : "   <- clasica incorrecta")
              << (karaOk ? "" : "   <- karatsuba incorrecta") << "\n"
              << "     A         = " << cortar(c.a) << "\n"
              << "     B         = " << cortar(c.b) << "\n"
              << "     clasica   = " << cortar(clasica) << "\n"
              << "     karatsuba = " << cortar(kara) << "\n";
    if (!esp.empty()) std::cout << "     esperado  = " << cortar(esp) << "\n";
}

bool validarCorrectitud() {
    std::cout << "=== 1) Validacion de correctitud: karatsuba vs clasica ===\n";
    std::mt19937_64 rng(2026);  // semilla fija: pruebas reproducibles
    std::vector<Caso> casos;

    // Grupo 1: numeros de 1 digito
    const std::string g1 = "1 digito";
    casos.push_back({g1, "7", "8", "56"});
    casos.push_back({g1, "9", "9", "81"});
    casos.push_back({g1, "0", "0", "0"});
    casos.push_back({g1, "5", "0", "0"});
    casos.push_back({g1, "1", "1", "1"});
    for (int x = 0; x <= 9; ++x)          // tabla completa 10x10
        for (int y = 0; y <= 9; ++y)
            casos.push_back({g1, std::to_string(x), std::to_string(y),
                             std::to_string(x * y)});

    // Grupo 2: operandos con muchos ceros a la izquierda
    const std::string g2 = "ceros a la izquierda";
    casos.push_back({g2, "000000123", "00045", "5535"});
    casos.push_back({g2, "0000", "0000", "0"});
    casos.push_back({g2, "0000000000000001", "0000000000000001", "1"});
    casos.push_back({g2, "00000000000000000000000000000012", "3", "36"});

    // Grupo 3: ceros intercalados
    const std::string g3 = "ceros intercalados";
    casos.push_back({g3, "100200300400", "5000600070", "501061629194261028000"});
    casos.push_back({g3, "1000000001", "1000000001", "1000000002000000001"});
    casos.push_back({g3, "10101010101010101", "10101010101010101", ""});
    casos.push_back({g3, "90009", "90009", "8101620081"});

    // Grupo 4: longitudes impares (antes del relleno a potencia de 2)
    const std::string g4 = "longitudes impares";
    casos.push_back({g4, "12345", "678", "8369910"});
    casos.push_back({g4, "1234567", "123456789", "152415677625363"});
    casos.push_back({g4, "9876543210123", "7", "69135802470861"});
    casos.push_back({g4, "12345678901234567890", "98765432109876543210",
                     "1219326311370217952237463801111263526900"});
    for (std::size_t la : {3, 5, 7, 9, 11, 13, 17, 31, 33, 63, 65, 127})
        for (std::size_t lb : {1, 2, 5, 8, 21})
            casos.push_back({g4, generarNumero(la, rng), generarNumero(lb, rng), ""});

    // Grupo 5: numeros grandes (varios cientos de digitos)
    const std::string g5 = "numeros grandes";
    for (std::size_t n : {200, 333, 500, 512, 777, 1000, 1024}) {
        casos.push_back({g5, generarNumero(n, rng), generarNumero(n, rng), ""});
        casos.push_back({g5, generarNumero(n, rng), generarNumero(n / 3 + 1, rng), ""});
        // 99..9 x 99..9 = 99..98 00..01 : resultado conocido sin usar ningun algoritmo
        const std::string nueves(n, '9');
        const std::string esp = std::string(n - 1, '9') + "8" +
                                std::string(n - 1, '0') + "1";
        casos.push_back({g5, nueves, nueves, esp});
    }
    // 10^k x 10^k = 10^(2k)
    casos.push_back({g5, "1" + std::string(399, '0'), "1" + std::string(399, '0'),
                     "1" + std::string(798, '0')});

    for (const auto& c : casos) ejecutarCaso(c);

    std::cout << "\n  " << std::left << std::setw(24) << "Grupo" << std::right
              << std::setw(8) << "Casos" << std::setw(16) << "Clasica OK"
              << std::setw(16) << "Karatsuba OK" << "\n";
    for (const auto& nombre : ordenGrupos) {
        const ResumenGrupo& g = resumen[nombre];
        const std::string c1 = std::to_string(g.total - g.clasicaMal) + "/" + std::to_string(g.total);
        const std::string c2 = std::to_string(g.total - g.karatsubaMal) + "/" + std::to_string(g.total);
        std::cout << "  " << std::left << std::setw(24) << nombre << std::right
                  << std::setw(8) << g.total << std::setw(16) << c1 << std::setw(16) << c2 << "\n";
    }
    std::cout << "\n  Casos: " << totalCasos << " | con algun error: " << fallidos
              << " | errores en clasica: " << totClasicaMal
              << " | errores en karatsuba: " << totKaratsubaMal << "\n";
    if (fallidos > MAX_DETALLES)
        std::cout << "  (solo se detallaron los primeros " << MAX_DETALLES << " casos con error)\n";
    if (totClasicaMal == totalCasos)
        std::cout << "  Pista: la clasica falla en TODOS los casos; probablemente su funcion\n"
                  << "  aun no esta implementada (o devuelve siempre \"0\").\n";
    if (totKaratsubaMal == totalCasos)
        std::cout << "  Pista: karatsuba falla en TODOS los casos; probablemente su funcion\n"
                  << "  o las operaciones auxiliares aun no estan implementadas.\n";
    if (fallidos == 0) std::cout << "  RESULTADO: todas las pruebas pasaron.\n\n";
    else std::cout << "  RESULTADO: HAY FALLAS. Se omite el benchmark.\n\n";
    return fallidos == 0;
}

// ---------------------------------------------------------------------
// 2) BENCHMARK
// ---------------------------------------------------------------------

// Sumidero volatil: obliga al compilador a considerar "usado" cada resultado.
volatile std::uint64_t sumidero = 0;

inline void consumir(const std::string& r) {
    sumidero = sumidero + r.size() + (r.empty() ? 0 : static_cast<unsigned char>(r.back()));
}

constexpr double OBJETIVO_MS = 60.0;  // cada medicion debe durar >= 60 ms
constexpr int    ENSAYOS     = 5;     // mediciones independientes por tamano

template <typename Func>
double tiempoPorOperacionNs(Func f, const std::string& a, const std::string& b,
                            std::size_t& repsUsadas) {
    // Calentamiento (caches, branch predictor, asignador de memoria)
    for (int i = 0; i < 3; ++i) consumir(f(a, b));

    // Calibracion: duplicar repeticiones hasta que la medicion dure
    // OBJETIVO_MS, muy por encima de la resolucion del reloj.
    std::size_t reps = 1;
    while (true) {
        const auto t0 = Reloj::now();
        for (std::size_t i = 0; i < reps; ++i) consumir(f(a, b));
        const double ms = std::chrono::duration<double, std::milli>(Reloj::now() - t0).count();
        if (ms >= OBJETIVO_MS || reps >= (1u << 28)) break;
        reps *= 2;
    }
    repsUsadas = reps;

    // Mediciones finales: promedio de ENSAYOS ensayos de `reps` repeticiones.
    double acumulado = 0.0;
    for (int e = 0; e < ENSAYOS; ++e) {
        const auto t0 = Reloj::now();
        for (std::size_t i = 0; i < reps; ++i) consumir(f(a, b));
        const double ns = std::chrono::duration<double, std::nano>(Reloj::now() - t0).count();
        acumulado += ns / static_cast<double>(reps);
    }
    return acumulado / ENSAYOS;
}

struct Fila {
    std::size_t n;
    std::size_t reps;
    double clasicaNs;
    double karatsubaNs;
};

std::vector<Fila> ejecutarBenchmark() {
    std::cout << "=== 2) Benchmark: n = 2, 4, ..., 4096 digitos ===\n";
    std::cout << "  (objetivo >= " << OBJETIVO_MS << " ms por medicion, "
              << ENSAYOS << " ensayos por tamano; puede tardar unos minutos)\n\n";

    std::mt19937_64 rng(4096);
    std::vector<Fila> filas;

    std::cout << std::setw(8) << "digitos" << std::setw(12) << "reps"
              << std::setw(16) << "clasica (us)" << std::setw(16) << "karatsuba (us)"
              << std::setw(12) << "clas/kara" << "\n";

    for (std::size_t n = 2; n <= 4096; n *= 2) {
        const std::string a = generarNumero(n, rng);
        const std::string b = generarNumero(n, rng);

        // Verificacion fuera del cronometro: si difieren, la medicion no vale.
        if (normalizar(multiplicacionClasica(a, b)) != normalizar(karatsuba(a, b))) {
            std::cerr << "ERROR: resultados distintos para n=" << n
                      << ". Medicion abortada.\n";
            std::exit(2);
        }

        Fila f{n, 0, 0.0, 0.0};
        std::size_t repsC = 0, repsK = 0;
        f.clasicaNs = tiempoPorOperacionNs(
            [](const std::string& x, const std::string& y) { return multiplicacionClasica(x, y); },
            a, b, repsC);
        f.karatsubaNs = tiempoPorOperacionNs(
            [](const std::string& x, const std::string& y) { return karatsuba(x, y); },
            a, b, repsK);
        f.reps = std::min(repsC, repsK);
        filas.push_back(f);

        std::cout << std::setw(8) << f.n << std::setw(12) << f.reps
                  << std::setw(16) << std::fixed << std::setprecision(3) << f.clasicaNs / 1e3
                  << std::setw(16) << f.karatsubaNs / 1e3
                  << std::setw(12) << std::setprecision(2) << f.clasicaNs / f.karatsubaNs
                  << "\n";
    }
    std::cout << "\n";
    return filas;
}

// ---------------------------------------------------------------------
// 3) CSV + PUNTO DE CRUCE
// ---------------------------------------------------------------------

void escribirCSV(const std::vector<Fila>& filas, const std::string& ruta) {
    const std::filesystem::path p(ruta);
    if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path());

    std::ofstream out(ruta);
    if (!out) {
        std::cerr << "No se pudo escribir " << ruta << "\n";
        return;
    }
    out << "digitos,repeticiones,clasica_ns,karatsuba_ns,razon_clasica_sobre_karatsuba,mas_rapido\n";
    out << std::fixed << std::setprecision(1);
    for (const auto& f : filas) {
        out << f.n << "," << f.reps << "," << f.clasicaNs << "," << f.karatsubaNs << ","
            << std::setprecision(3) << f.clasicaNs / f.karatsubaNs << std::setprecision(1) << ","
            << (f.karatsubaNs < f.clasicaNs ? "karatsuba" : "clasica") << "\n";
    }
    std::cout << "CSV escrito en " << ruta << "\n";
}

void reportarCruce(const std::vector<Fila>& filas) {
    std::cout << "=== 3) Punto de cruce ===\n";
    long primero = -1, sostenido = -1;
    for (std::size_t i = 0; i < filas.size(); ++i) {
        const bool kGana = filas[i].karatsubaNs < filas[i].clasicaNs;
        if (kGana && primero < 0) primero = static_cast<long>(filas[i].n);
        if (kGana) {
            bool todosDespues = true;
            for (std::size_t j = i; j < filas.size(); ++j)
                if (!(filas[j].karatsubaNs < filas[j].clasicaNs)) { todosDespues = false; break; }
            if (todosDespues && sostenido < 0) sostenido = static_cast<long>(filas[i].n);
        }
    }
    if (primero < 0) {
        std::cout << "  Karatsuba NO supera al clasico en el rango medido (2..4096).\n"
                  << "  Hay que explicarlo en el informe (constantes ocultas, copias de\n"
                  << "  strings, asignaciones de memoria, caso base muy pequeno).\n";
    } else {
        std::cout << "  Primer tamano donde Karatsuba gana : " << primero << " digitos\n";
        if (sostenido >= 0)
            std::cout << "  Gana de forma sostenida desde      : " << sostenido << " digitos\n";
        else
            std::cout << "  (no gana de forma sostenida: hay tamanos mayores donde pierde)\n";
        std::cout << "  Nota: se midieron solo potencias de 2, asi que el cruce real esta\n"
                  << "  entre " << std::max<long>(primero / 2, 1) << " y " << primero
                  << " digitos.\n";
    }
}

}  // namespace

int main(int argc, char** argv) {
    const std::string rutaCSV = (argc > 1) ? argv[1] : "resultados/tiempos.csv";

    if (!validarCorrectitud()) return 1;

    const auto filas = ejecutarBenchmark();
    escribirCSV(filas, rutaCSV);
    reportarCruce(filas);

    std::cout << "\n(checksum anti-optimizacion: " << sumidero << ")\n";
    return 0;
}