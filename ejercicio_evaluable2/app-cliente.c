#include "claves.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;      // Mutex para proteger el acceso a la base de datos
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex para proteger la salida por pantalla

// Función auxiliar para impresión segura (mutex)
void safe_printf(const char *format, ...) {
    va_list args;
    pthread_mutex_lock(&print_mutex);
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    pthread_mutex_unlock(&print_mutex);
}

// Función para probar operaciones básicas
void test_basic_operations() {
    int base_key = (getpid() * time(NULL)) % 1000000;
    int key = base_key;  
    char value1[256] = "Hola Mundo";
    int N_value2 = 3;
    double V_value2[3] = {1.1, 2.2, 3.3};
    struct Coord value3 = {4, 5};
    char recv_value1[256];
    int recv_N_value2;
    double recv_V_value2[32];
    struct Coord recv_value3;

    safe_printf("\n[TEST] Probando operaciones básicas...\n");

    int res = set_value(key, value1, N_value2, V_value2, value3);
    safe_printf("set_value (key = %d) - Resultado: %d\n", key, res);

    res = get_value(key, recv_value1, &recv_N_value2, recv_V_value2, &recv_value3);
    safe_printf("get_value (key = %d): %s, %d elementos, (%d, %d)\n", key, recv_value1, recv_N_value2, recv_value3.x, recv_value3.y);

    res = modify_value(key, "Nuevo Valor", 3, V_value2, value3);
    safe_printf("modify_value (key = %d) - Resultado: %d\n", key, res);

    res = exist(key);
    res ? safe_printf("Existencia confirmada para key %d\n", key) : safe_printf("Clave %d no encontrada\n", key);

    res = delete_key(key);
    safe_printf("delete_key (key = %d) - Resultado: %d\n", key, res);

    res = exist(key);
    res ? safe_printf("Error al eliminar clave %d\n", key) : safe_printf("Clave %d eliminada correctamente\n", key);
}

// Función que cada hilo ejecutará para insertar claves
void *thread_function(void *arg) {
    int id = *(int *)arg;
    int base_key = (getpid() * time(NULL)) % 1000000; // Generar clave única por cliente
    for (int i = base_key + id * 10; i < base_key + (id + 1) * 10; i++) {
        char str[256];
        snprintf(str, sizeof(str), "Valor_%d", i);
        double vec[32];
        for (int j = 0; j < 32; j++) 
            vec[j] = i * 1.1;
        struct Coord coord = {i, i + 1};
        set_value(i, str, 32, vec, coord);
    }
    safe_printf("Hilo %d finalizado\n", id);
    return NULL;
}

void test_concurrency() {
    safe_printf("\n[TEST] Probando concurrencia con 10 hilos...\n");
    pthread_t threads[10];
    int ids[10];

    for (int i = 0; i < 10; i++) {
        ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_function, &ids[i]) != 0) {
            perror("pthread_create");
            return;
        }
    }
    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }
}

void test_limit_request() {
    safe_printf("\n[TEST] Probando el límite de requests...\n");

    // Limpiar la base de datos antes de comenzar para evitar duplicados
    destroy();

    struct Coord coord = {0, 0};
    double vec[2] = {1.0, 2.0};
    int base_key = ((getpid() * (time(NULL) + 1)) % 1000000);

    for (int i = 0; i < 100; i++) {
        int res = set_value(base_key + i, "Prueba", 2, vec, coord);
        safe_printf("Insertando request con clave %d - Resultado: %d\n", base_key + i, res);
    }

    safe_printf("Borrando todas las tuplas...\n");
    pthread_mutex_lock(&mutex);
    int res = destroy();
    pthread_mutex_unlock(&mutex);
    safe_printf("Resultado de destruir todas las tuplas: %d\n", res);
}

void test_extreme_values() {
    safe_printf("\n[TEST] Probando valores extremos...\n");

    char max_str[256];
    memset(max_str, 'X', 255);
    max_str[254] = '\0';
    int base_key = (getpid() * time(NULL)) % 1000000;

    double max_vec[32];
    // Ajustamos el valor máximo a 1e+308 para evitar problemas en el formateo
    for (int i = 0; i < 32; i++) 
        max_vec[i] = 1e+308;

    struct Coord extreme_coord = {2147483647, -2147483648};

    int res = set_value(base_key, max_str, 32, max_vec, extreme_coord);
    safe_printf("set_value valores extremos - Resultado: %d\n", res);
}

void test_invalid_inputs() {
    safe_printf("\n[TEST] Probando entradas inválidas...\n");

    char over_str[257];
    memset(over_str, 'Y', 256);
    over_str[256] = '\0';
    int base_key = (getpid() * time(NULL)) % 1000000;

    int res = set_value(base_key, over_str, 2, (double[2]){1.0, 2.0}, (struct Coord){0, 0});
    safe_printf("Intentando insertar cadena fuera de rango - Resultado: %d\n", res);
}

int main() {
    // Definir variables de entorno para IP y puerto (se puede modificar según convenga)
    if (setenv("IP_TUPLAS", "127.0.0.1", 1) != 0 || setenv("PORT_TUPLAS", "8080", 1) != 0) {
        perror("Error al definir variables de entorno");
        exit(EXIT_FAILURE);
    }
    test_basic_operations();
    test_concurrency();
    test_limit_request();
    test_extreme_values();
    test_invalid_inputs();

    safe_printf("\n[TEST] Todas las pruebas realizadas.\n");
    safe_printf("\nTodas las pruebas finalizadas.\n");
    safe_printf("Presione ENTER para finalizar...\n");
    fflush(stdout);
    getchar();

    return 0;
}
