// Incluye las bibliotecas estándar necesarias para el manejo de sockets, hilos y funciones auxiliares
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "claves.h"       // Incluye las definiciones de estructuras y funciones relacionadas con claves
#include "claves_real.h"  // Incluye las implementaciones reales de las funciones de claves

#define MAX_CLIENTS 10    // Define el número máximo de clientes que pueden conectarse simultáneamente
#define BUFFER_SIZE 4096  // Define el tamaño máximo del búfer para enviar y recibir datos

// Función auxiliar para extraer un campo numérico de la cadena JSON
static int extract_int(const char *json, const char *field, int *value) {
    char *p = strstr(json, field); // Busca el campo en la cadena JSON
    if (p) {
        p += strlen(field);        // Avanza hasta el valor del campo
        return sscanf(p, "%d", value); // Extrae el valor como entero
    }
    return 0; // Devuelve 0 si no se encuentra el campo
}

// Función auxiliar para extraer un campo de cadena entre comillas
static int extract_string(const char *json, const char *field, char *dest, size_t dest_size) {
    char *p = strstr(json, field); // Busca el campo en la cadena JSON
    if (p) {
        p += strlen(field);        // Avanza hasta el valor del campo
        char *q = strchr(p, '\"'); // Busca el cierre de las comillas
        if (q) {
            size_t len = q - p;    // Calcula la longitud del valor
            if (len >= dest_size) len = dest_size - 1; // Limita el tamaño del valor
            strncpy(dest, p, len); // Copia el valor al destino
            dest[len] = '\0';      // Asegura que termine en nulo
            return 1;              // Devuelve 1 si se extrae correctamente
        }
    }
    return 0; // Devuelve 0 si no se encuentra el campo
}

// Función que maneja la comunicación con un cliente mediante JSON
void *handle_client(void *arg) {
    int sock = *(int *)arg; // Obtiene el descriptor del socket del cliente
    char buffer[BUFFER_SIZE] = {0}; // Búfer para recibir datos

    // Recibir el mensaje JSON completo
    ssize_t bytes_read = recv(sock, buffer, sizeof(buffer) - 1,0); // Lee datos del socket
    if (bytes_read < 0) { // Si ocurre un error al leer
        perror("read failed");
        close(sock);      // Cierra el socket
        free(arg);        // Libera la memoria asignada al cliente
        pthread_exit(NULL); // Finaliza el hilo
    }
    buffer[bytes_read] = '\0'; // Asegura que el búfer termine en nulo

    // Variables para almacenar los datos recibidos
    int operation = -1, key = 0, N_value2 = 0;
    char value1[256] = {0};
    double V_value2[32] = {0.0};
    struct Coord value3 = {0, 0};

    // Parseo básico del JSON entrante
    if (extract_int(buffer, "\"operation\":", &operation) != 1) { // Extrae el campo "operation"
        fprintf(stderr, "Error: No se pudo parsear 'operation'\n");
        close(sock);      // Cierra el socket
        free(arg);        // Libera la memoria asignada al cliente
        pthread_exit(NULL); // Finaliza el hilo
    }
    extract_int(buffer, "\"key\":", &key); // Extrae el campo "key"
    extract_string(buffer, "\"value1\":\"", value1, sizeof(value1)); // Extrae el campo "value1"
    extract_int(buffer, "\"N_value2\":", &N_value2); // Extrae el campo "N_value2"

    // Parseo del array V_value2
    char *p = strstr(buffer, "\"V_value2\":[");
    if (p) {
        p += strlen("\"V_value2\":["); // Avanza hasta el inicio del array
        for (int i = 0; i < N_value2; i++) {
            if (sscanf(p, "%lf", &V_value2[i]) != 1) break; // Extrae cada valor del array
            char *comma = strchr(p, ','); // Busca la siguiente coma
            char *bracket = strchr(p, ']'); // Busca el cierre del array
            if (comma && (!bracket || comma < bracket)) {
                p = comma + 1; // Avanza al siguiente valor
            } else {
                break; // Sale del bucle si no hay más valores
            }
        }
    }

    // Parseo de value3
    p = strstr(buffer, "\"value3\":{\"x\":");
    if (p) {
        p += strlen("\"value3\":{\"x\":"); // Avanza hasta el inicio de value3
        sscanf(p, "%d,\"y\":%d", &value3.x, &value3.y); // Extrae las coordenadas x e y
    }

    // Realizar la operación solicitada
    int result;
    switch (operation) {
        case 1: result = real_destroy(); break; // Llama a la función real_destroy
        case 2: result = real_set_value(key, value1, N_value2, V_value2, value3); break; // Llama a real_set_value
        case 3: result = real_get_value(key, value1, &N_value2, V_value2, &value3); break; // Llama a real_get_value
        case 4: result = real_modify_value(key, value1, N_value2, V_value2, value3); break; // Llama a real_modify_value
        case 5: result = real_delete_key(key); break; // Llama a real_delete_key
        case 6: result = real_exist(key); break; // Llama a real_exist
        default: result = -1; break; // Operación no válida
    }

    // Construir respuesta JSON
    char send_buffer[BUFFER_SIZE] = {0};
    if (result == 0) { // Si la operación fue exitosa
        size_t offset = snprintf(send_buffer, sizeof(send_buffer),
            "{\"result\":0,\"value1\":\"%s\",\"N_value2\":%d,\"V_value2\":[", value1, N_value2);
        for (int i = 0; i < N_value2; i++) {
            int n = snprintf(send_buffer + offset, sizeof(send_buffer) - offset, "%.6g", V_value2[i]);
            offset += n;
            if (i < N_value2 - 1) {
                if (offset < sizeof(send_buffer) - 1) {
                    send_buffer[offset++] = ','; // Añade una coma entre los valores
                    send_buffer[offset] = '\0';
                }
            }
        }
        snprintf(send_buffer + offset, sizeof(send_buffer) - offset, "],\"value3\":{\"x\":%d,\"y\":%d}}", value3.x, value3.y);
    } else { // Si hubo un error
        snprintf(send_buffer, sizeof(send_buffer), "{\"result\":%d}", result);
    }

    if (send(sock, send_buffer, strlen(send_buffer), 0) < 0) { // Envía la respuesta al cliente
        perror("send failed");
    }

    close(sock); // Cierra el socket
    free(arg);   // Libera la memoria asignada al cliente
    pthread_exit(NULL); // Finaliza el hilo
}

// Función principal del servidor
int main(int argc, char *argv[]) {
    if (argc != 2) { // Verifica que se pase el puerto como argumento
        fprintf(stderr, "Error -> Uso: %s <PUERTO>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]); // Convierte el argumento a entero
    if (port <= 0) { // Verifica que el puerto sea válido
        fprintf(stderr, "Puerto inválido.\n");
        exit(EXIT_FAILURE);
    }
    
    int server_fd; // Descriptor del socket del servidor
    struct sockaddr_in address; // Dirección del servidor
    int addrlen = sizeof(address);

    // Crea el socket del servidor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET; // Familia de direcciones (IPv4)
    address.sin_addr.s_addr = INADDR_ANY; // Acepta conexiones de cualquier dirección
    address.sin_port = htons(port); // Puerto del servidor
    int val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (void*) &val, sizeof(val)); // Permite reutilizar la dirección
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) { // Asocia el socket a la dirección
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, MAX_CLIENTS) < 0) { // Escucha conexiones entrantes
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Servidor iniciado en el puerto %d\n", port);

    pthread_attr_t attr; // Atributos para los hilos
    pthread_attr_init(&attr); // Inicializa los atributos
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); // Configura los hilos como "desprendidos"

    while (1) { // Bucle principal para aceptar conexiones
        int new_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen); // Acepta una conexión
        if (new_sock < 0) {
            perror("Fallo en accept");
            continue;
        }
    
        int *client_sock = malloc(sizeof(int)); // Reserva memoria para el descriptor del cliente
        if (!client_sock) {
            perror("Fallo en malloc");
            close(new_sock);
            continue;
        }
        *client_sock = new_sock; // Asigna el descriptor del cliente
    
        pthread_t thread; // Crea un nuevo hilo para manejar al cliente
        if (pthread_create(&thread, &attr, handle_client, client_sock) != 0) {
            perror("Fallo en pthread_create");
            close(new_sock);
            free(client_sock);
            continue;
        }
    }
    pthread_attr_destroy(&attr); // Destruye los atributos de los hilos
    close(server_fd); // Cierra el socket del servidor
    return 0; // Finaliza el programa
}