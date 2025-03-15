#include "claves.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// Función para probar operaciones básicas
void test_basic_operations() {
    printf("\n[TEST] Probando operaciones básicas...\n");
    int key = 10;
    char value1[256] = "Hola Mundo";
    int N_value2 = 3;
    double V_value2[3] = {1.1, 2.2, 3.3};
    struct Coord value3 = {4, 5};
    char recv_value1[256];
    int recv_N_value2;
    double recv_V_value2[32];
    struct Coord recv_value3;

    // Prueba set_value
    int res = set_value(key, value1, N_value2, V_value2, value3);
    printf("set_value - Resultado: %d\n", res);

    // Prueba get_value
    res = get_value(key, recv_value1, &recv_N_value2, recv_V_value2, &recv_value3);
    printf("get_value: %s, %d elementos, (%d, %d)\n", recv_value1, recv_N_value2, recv_value3.x, recv_value3.y);

    // Prueba modify_value
    res = modify_value(key, "Nuevo Valor", 3, V_value2, value3);
    printf("modify_value - Resultado: %d\n", res);

    // Prueba exist
    res = exist(key);
    res ? printf("Existencia confirmada\n") : printf("Clave no encontrada\n");

    // Prueba delete_key
    res = delete_key(key);
    printf("delete_key - Resultado: %d\n", res);

    // Verificar que la clave fue eliminada
    res = exist(key);
    res ? printf("Error al eliminar clave\n") : printf("Clave eliminada correctamente\n");
}

// Función que cada hilo ejecutará para insertar claves
void *thread_function(void *arg) {
    int id = *(int *)arg;
    for (int i = id * 10; i < (id + 1) * 10; i++) {
        char str[256];
        snprintf(str, sizeof(str), "Valor_%d", i);
        double vec[32];
        for (int j = 0; j < 32; j++) vec[j] = i * 1.1;
        struct Coord coord = {i, i + 1};
        set_value(i, str, 32, vec, coord);
    }
    return NULL;
}

// Prueba de concurrencia con múltiples hilos
void test_concurrency() {
    printf("\n[TEST] Probando concurrencia con 10 hilos...\n");
    pthread_t threads[10];
    int ids[10];

    for (int i = 0; i < 10; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, thread_function, &ids[i]);
    }

    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
        printf("Hilo %d finalizado\n", i);
    }
}

// Prueba para insertar muchas claves y verificar si el sistema colapsa
void test_max_queue_size() {
    printf("\n[TEST] Probando el límite de la cola de mensajes...\n");
    struct Coord coord = {0, 0};
    double vec[2] = {1.0, 2.0};

    for (int i = 0; i < 60; i++) {
        int res = set_value(i, "Prueba", 2, vec, coord);
        printf("Insertando clave %d - Resultado: %d\n", i, res);
    }
}

// Prueba de valores extremos en clave, cadena y vectores
void test_extreme_values() {
    printf("\n[TEST] Probando valores extremos...\n");

    char max_str[256];
    memset(max_str, 'X', 255);
    max_str[255] = '\0';

    double max_vec[32];
    for (int i = 0; i < 32; i++) max_vec[i] = 1.7E+308;

    struct Coord extreme_coord = {2147483647, -2147483648};

    int res = set_value(999, max_str, 32, max_vec, extreme_coord);
    printf("set_value valores extremos - Resultado: %d\n", res);

    char recv_value1[256];
    int recv_N_value2;
    double recv_V_value2[32];
    struct Coord recv_value3;

    res = get_value(999, recv_value1, &recv_N_value2, recv_V_value2, &recv_value3);
    printf("get_value valores extremos - Resultado: %d\n", res);
}

// Prueba de entradas inválidas
void test_invalid_inputs() {
    printf("\n[TEST] Probando entradas inválidas...\n");

    char over_str[257];
    memset(over_str, 'Y', 256);
    over_str[256] = '\0';

    double over_vector[33];
    for (int i = 0; i < 33; i++) over_vector[i] = i * 1.1;

    int res = set_value(1001, over_str, 2, (double[2]){1.0, 2.0}, (struct Coord){0, 0});
    printf("Intentando insertar cadena fuera de rango - Resultado: %d\n", res);

    res = set_value(1002, "VectorLargo", 33, over_vector, (struct Coord){0, 0});
    printf("Intentando insertar vector fuera de rango - Resultado: %d\n", res);
}

// Prueba de eliminación masiva de claves
void test_massive_delete() {
    printf("\n[TEST] Probando eliminación masiva...\n");

    struct Coord coord = {0, 0};
    double vec[2] = {1.0, 2.0};

    for (int i = 2000; i < 2100; i++) {
        set_value(i, "Temp", 2, vec, coord);
    }

    for (int i = 2000; i < 2100; i++) {
        delete_key(i);
        int exists = exist(i);
        if (exists) {
            printf("Error: clave %d no eliminada correctamente.\n", i);
        }
    }
}

// Prueba de destroy() concurrente
void *destroy_thread(void *arg) {
    destroy();
    return NULL;
}

void test_concurrent_destroy() {
    printf("\n[TEST] Probando destroy() concurrente...\n");
    pthread_t threads[5];

    for (int i = 0; i < 5; i++) {
        pthread_create(&threads[i], NULL, destroy_thread, NULL);
    }

    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Prueba de destroy() concurrente finalizada.\n");
}

// Función principal que ejecuta todas las pruebas
int main() {
    test_basic_operations();
    test_extreme_values();
    test_invalid_inputs();
    test_concurrency();
    test_max_queue_size();
    test_massive_delete();
    test_concurrent_destroy();
    destroy();

    printf("\nTodas las pruebas finalizadas.\n");
    return 0;
}