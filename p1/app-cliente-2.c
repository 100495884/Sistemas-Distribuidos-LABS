#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "claves.h"

int main() {
    int key = 2;  // Usamos una clave diferente para evitar conflictos con el primer cliente
    char value1[255] = "ValorEjemplo2";
    int N_value2 = 2;
    double V_value2[] = {4.4, 5.5};
    struct Coord value3 = {30, 40};

    printf("Iniciando pruebas de la API de tuplas (Cliente 2)...\n");

    // Prueba: Crear una clave y asignar valores
    printf("Probando set_value...\n");
    int result = set_value(key, value1, N_value2, V_value2, value3);
    printf("Resultado: %d\n", result);

    // Prueba: Obtener el valor de la clave creada
    printf("Probando get_value...\n");
    char received_value1[256];
    int received_N;
    double received_V[32];
    struct Coord received_coord;
    result = get_value(key, received_value1, &received_N, received_V, &received_coord);
    printf("Resultado: %d\nValores: %s, %d, (%d,%d)\n", 
           result, received_value1, received_N, received_coord.x, received_coord.y);

    // Prueba: Modificar el valor de la clave
    char new_value1[] = "NuevoValor2";
    double new_V_value2[] = {6.6, 7.7};
    struct Coord new_value3 = {50, 60};

    if (modify_value(key, new_value1, N_value2, new_V_value2, new_value3) == 0) {
        printf("✅ modify_value exitoso\n");
    } else {
        printf("❌ Error en modify_value\n");
    }

    // Prueba: Comprobar existencia de la clave
    if (exist(key) == 1) {
        printf("✅ La clave %d existe\n", key);
    } else {
        printf("❌ Error en exist\n");
    }

    // Prueba: Eliminar la clave
    if (delete_key(key) == 0) {
        printf("✅ delete_key exitoso\n");
    } else {
        printf("❌ Error en delete_key\n");
    }

    // Prueba: Comprobar si la clave sigue existiendo tras la eliminación
    if (exist(key) == 0) {
        printf("✅ La clave %d ya no existe tras delete_key\n", key);
    } else {
        printf("❌ Error: La clave %d aún existe tras delete_key\n", key);
    }

    return 0;
}