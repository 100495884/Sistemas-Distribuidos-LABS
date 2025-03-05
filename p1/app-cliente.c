#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "claves.h"
#include "claves_real.h"

int main() {
    int key = 1;
    char value1[255] = "ValorEjemplo";
    int N_value2 = 3;
    double V_value2[] = {1.1, 2.2, 3.3};
    struct Coord value3 = {10, 20};

    printf("Iniciando pruebas de la API de tuplas...\n");

    // Prueba: Crear una clave y asignar valores
    if (_set_value(key, value1, N_value2, V_value2, value3) == 0) {
        printf("✅ set_value exitoso para clave %d\n", key);
    } else {
        printf("❌ Error en set_value\n");
    }

    // Prueba: Obtener el valor de la clave creada
    char value1_get[255];
    int N_value2_get;
    double V_value2_get[32];
    struct Coord value3_get;

    if (_get_value(key, value1_get, &N_value2_get, V_value2_get, &value3_get) == 0) {
        printf("✅ get_value exitoso: value1='%s', N_value2=%d, value3=(%d, %d)\n",
               value1_get, N_value2_get, value3_get.x, value3_get.y);
    } else {
        printf("❌ Error en get_value\n");
    }

    // Prueba: Modificar el valor de la clave
    char new_value1[] = "NuevoValor";
    double new_V_value2[] = {4.4, 5.5, 6.6};
    struct Coord new_value3 = {30, 40};

    if (_modify_value(key, new_value1, N_value2, new_V_value2, new_value3) == 0) {
        printf("✅ modify_value exitoso\n");
    } else {
        printf("❌ Error en modify_value\n");
    }

    // Prueba: Comprobar existencia de la clave
    if (_exist(key) == 1) {
        printf("✅ La clave %d existe\n", key);
    } else {
        printf("❌ Error en exist\n");
    }

    // Prueba: Eliminar la clave
    if (_delete_key(key) == 0) {
        printf("✅ delete_key exitoso\n");
    } else {
        printf("❌ Error en delete_key\n");
    }

    // Prueba: Comprobar si la clave sigue existiendo tras la eliminación
    if (_exist(key) == 0) {
        printf("✅ La clave %d ya no existe tras delete_key\n", key);
    } else {
        printf("❌ Error: La clave %d aún existe tras delete_key\n", key);
    }

    // Prueba: Eliminar todas las claves
    if (_destroy() == 0) {
        printf("✅ destroy exitoso\n");
    } else {
        printf("❌ Error en destroy\n");
    }

    return 0;
}
