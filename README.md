# Práctica 2 - Multiplicación Rápida de Enteros (Karatsuba)

Este proyecto implementa y compara el algoritmo de multiplicación clásica O(n^2) con el algoritmo de Divide y Vencerás de Karatsuba O(n^1.585) para enteros grandes representados como cadenas.

## Estructura del Proyecto
* `include/`: Contiene la cabecera `karatsuba.h` con las firmas de las funciones.
* `src/`: Implementación de las operaciones aritméticas, el algoritmo de Karatsuba y el programa principal de pruebas.
* `resultados/`: Almacena el archivo `tiempos.csv` generado por el benchmark y la gráfica de rendimiento.
* `Informe_Practica2_Karatsuba_Final.docx.pdf`: Análisis teórico, experimentación y conclusiones.
* `Makefile`: Archivo de configuración para automatizar la compilación del proyecto.
* `.gitignore`: Archivo para excluir los binarios compilados y ejecutables del control de versiones.
* `README.md`: Contiene información breve sobre el proyecto y las instrucciones de ejecución.

## Instrucciones de Compilación y Ejecución

Para compilar el proyecto, asegúrate de tener g++ y make instalados, y ejecuta en la raíz del repositorio:
```bash
make.

## Para ejecutar la batería de pruebas y generar los tiempos de ejecución:
./karatsuba_app
