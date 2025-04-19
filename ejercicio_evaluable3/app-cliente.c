// Incluye el archivo de cabecera "claves.h" que probablemente contiene las definiciones de las funciones utilizadas en este archivo
#include "claves.h"

// Incluye las bibliotecas estándar necesarias para el programa
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>

// Inicializa un mutex para proteger el acceso a la base de datos
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Inicializa un mutex para proteger la salida por pantalla
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

// Función auxiliar para impresión segura utilizando un mutex
void safe_printf(const char *format, ...) {
    va_list args; // Declara una lista de argumentos variables
    pthread_mutex_lock(&print_mutex); // Bloquea el mutex para proteger la salida
    va_start(args, format); // Inicializa la lista de argumentos variables
    vprintf(format, args); // Imprime el formato y los argumentos
    va_end(args); // Finaliza el uso de la lista de argumentos variables
    pthread_mutex_unlock(&print_mutex); // Desbloquea el mutex
}

// Función para probar operaciones básicas
void test_basic_operations() {
    // Genera una clave base única utilizando el PID del proceso y el tiempo actual
    int base_key = (getpid() * time(NULL)) % 1000000;
    int key = base_key; // Asigna la clave base a la variable "key"
    
    // Define valores iniciales para las pruebas
    char value1[256] = "Hola Mundo";
    int N_value2 = 3;
    double V_value2[3] = {1.1, 2.2, 3.3};
    struct Coord value3 = {4, 5};
    char recv_value1[256];
    int recv_N_value2;
    double recv_V_value2[32];
    struct Coord recv_value3;

    // Imprime un mensaje indicando el inicio de las pruebas
    safe_printf("\n[TEST] Probando operaciones básicas...\n");

    // Prueba la función set_value
    int res = set_value(key, value1, N_value2, V_value2, value3);
    safe_printf("set_value (key = %d) - Resultado: %d\n", key, res);

    // Prueba la función get_value
    res = get_value(key, recv_value1, &recv_N_value2, recv_V_value2, &recv_value3);
    safe_printf("get_value (key = %d): %s, %d elementos, (%d, %d)\n", key, recv_value1, recv_N_value2, recv_value3.x, recv_value3.y);

    // Prueba la función modify_value
    res = modify_value(key, "Nuevo Valor", 3, V_value2, value3);
    safe_printf("modify_value (key = %d) - Resultado: %d\n", key, res);

    // Prueba la función exist
    res = exist(key);
    res ? safe_printf("Existencia confirmada para key %d\n", key) : safe_printf("Clave %d no encontrada\n", key);

    // Prueba la función delete_key
    res = delete_key(key);
    safe_printf("delete_key (key = %d) - Resultado: %d\n", key, res);

    // Verifica nuevamente la existencia de la clave después de eliminarla
    res = exist(key);
    res ? safe_printf("Error al eliminar clave %d\n", key) : safe_printf("Clave %d eliminada correctamente\n", key);
}

// Función que cada hilo ejecutará para insertar claves
void *thread_function(void *arg) {
    int id = *(int *)arg; // Obtiene el ID del hilo a partir del argumento
    int base_key = (getpid() * time(NULL)) % 1000000; // Genera una clave base única por cliente

    // Inserta 10 claves únicas por hilo
    for (int i = base_key + id * 10; i < base_key + (id + 1) * 10; i++) {
        char str[256]; // Define un valor de tipo cadena
        snprintf(str, sizeof(str), "Valor_%d", i); // Formatea el valor como "Valor_<clave>"
        double vec[32]; // Define un vector de valores
        for (int j = 0; j < 32; j++) 
            vec[j] = i * 1.1; // Asigna valores al vector
        struct Coord coord = {i, i + 1}; // Define una coordenada
        set_value(i, str, 32, vec, coord); // Inserta la clave con sus valores
    }

    // Imprime un mensaje indicando que el hilo ha finalizado
    safe_printf("Hilo %d finalizado\n", id);
    return NULL; // Finaliza la ejecución del hilo
}

// Función para probar la concurrencia con múltiples hilos
void test_concurrency() {
    safe_printf("\n[TEST] Probando concurrencia con 10 hilos...\n");
    pthread_t threads[10]; // Declara un arreglo de 10 hilos
    int ids[10]; // Declara un arreglo para almacenar los IDs de los hilos

    // Crea 10 hilos
    for (int i = 0; i < 10; i++) {
        ids[i] = i; // Asigna un ID único a cada hilo
        if (pthread_create(&threads[i], NULL, thread_function, &ids[i]) != 0) { // Crea el hilo
            perror("pthread_create"); // Imprime un mensaje de error si falla
            return; // Finaliza la función en caso de error
        }
    }

    // Espera a que todos los hilos finalicen
    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL); // Une el hilo principal con cada hilo creado
    }
}

// Función para probar el límite de solicitudes
void test_limit_request() {
    safe_printf("\n[TEST] Probando el límite de requests...\n");

    // Limpia la base de datos antes de comenzar para evitar duplicados
    destroy();

    struct Coord coord = {0, 0}; // Define una coordenada inicial
    double vec[2] = {1.0, 2.0}; // Define un vector inicial
    int base_key = ((getpid() * (time(NULL) + 1)) % 1000000); // Genera una clave base única

    // Inserta 100 claves únicas
    for (int i = 0; i < 100; i++) {
        int res = set_value(base_key + i, "Prueba", 2, vec, coord); // Inserta la clave con sus valores
        safe_printf("Insertando request con clave %d - Resultado: %d\n", base_key + i, res);
    }

    // Borra todas las tuplas de la base de datos
    safe_printf("Borrando todas las tuplas...\n");
    pthread_mutex_lock(&mutex); // Bloquea el mutex para proteger la operación
    int res = destroy(); // Destruye todas las tuplas
    pthread_mutex_unlock(&mutex); // Desbloquea el mutex
    safe_printf("Resultado de destruir todas las tuplas: %d\n", res);
}

// Función para probar valores extremos
void test_extreme_values() {
    safe_printf("\n[TEST] Probando valores extremos...\n");

    char max_str[256]; // Define una cadena de tamaño máximo
    memset(max_str, 'X', 255); // Llena la cadena con el carácter 'X'
    max_str[254] = '\0'; // Asegura que la cadena termine en un carácter nulo
    int base_key = (getpid() * time(NULL)) % 1000000; // Genera una clave base única

    double max_vec[32]; // Define un vector de tamaño máximo
    for (int i = 0; i < 32; i++) 
        max_vec[i] = 1e+308; // Asigna el valor máximo permitido a cada elemento del vector

    struct Coord extreme_coord = {2147483647, -2147483648}; // Define coordenadas extremas

    // Inserta una clave con valores extremos
    int res = set_value(base_key, max_str, 32, max_vec, extreme_coord);
    safe_printf("set_value valores extremos - Resultado: %d\n", res);
}

// Función para probar entradas inválidas
void test_invalid_inputs() {
    safe_printf("\n[TEST] Probando entradas inválidas...\n");

    char over_str[257]; // Define una cadena que excede el tamaño permitido
    memset(over_str, 'Y', 256); // Llena la cadena con el carácter 'Y'
    over_str[256] = '\0'; // Asegura que la cadena termine en un carácter nulo
    int base_key = (getpid() * time(NULL)) % 1000000; // Genera una clave base única

    // Intenta insertar una clave con una cadena fuera de rango
    int res = set_value(base_key, over_str, 2, (double[2]){1.0, 2.0}, (struct Coord){0, 0});
    safe_printf("Intentando insertar cadena fuera de rango - Resultado: %d\n", res);
}

// Función principal del programa
int main() {
    test_basic_operations(); // Llama a la función para probar operaciones básicas
    test_concurrency(); // Llama a la función para probar concurrencia
    test_limit_request(); // Llama a la función para probar el límite de solicitudes
    test_extreme_values(); // Llama a la función para probar valores extremos
    test_invalid_inputs(); // Llama a la función para probar entradas inválidas

    // Imprime un mensaje indicando que todas las pruebas han finalizado
    safe_printf("\n[TEST] Todas las pruebas realizadas.\n");
    safe_printf("\nTodas las pruebas finalizadas.\n");
    safe_printf("Presione ENTER para finalizar...\n");
    fflush(stdout); // Fuerza la impresión inmediata del mensaje
    getchar(); // Espera a que el usuario presione ENTER

    return 0; // Finaliza el programa
}