// Incluye las bibliotecas estándar necesarias para el manejo de sockets, entrada/salida y funciones auxiliares
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "claves.h" // Incluye el archivo de cabecera "claves.h" que define las estructuras y funciones utilizadas

#define BUFFER_SIZE 4096 // Define el tamaño máximo del búfer para enviar y recibir datos

// Función auxiliar para obtener la IP del servidor desde una variable de entorno
static char* get_server_ip() {
    char *ip = getenv("IP_TUPLAS"); // Obtiene el valor de la variable de entorno "IP_TUPLAS"
    if (!ip) // Si no está definida, utiliza "127.0.0.1" como valor predeterminado
        ip = "127.0.0.1";
    return ip; // Devuelve la IP del servidor
}

// Función auxiliar para obtener el puerto del servidor desde una variable de entorno
static int get_server_port() {
    char *port_str = getenv("PORT_TUPLAS"); // Obtiene el valor de la variable de entorno "PORT_TUPLAS"
    if (port_str) // Si está definida, convierte el valor a entero
        return atoi(port_str);
    return 8080; // Si no está definida, utiliza 8080 como valor predeterminado
}

// Función para enviar un mensaje JSON al servidor y recibir la respuesta, parseándola
int send_request(int operation, int key, char *value1, int N_value2, double *V_value2, struct Coord value3,
                 char *recv_value1, int *recv_N_value2, double *recv_V_value2, struct Coord *recv_value3) {
    int sock; // Descriptor del socket
    struct sockaddr_in serv_addr; // Estructura para almacenar la dirección del servidor
    char send_buffer[BUFFER_SIZE] = {0}; // Búfer para enviar datos
    char recv_buffer[BUFFER_SIZE] = {0}; // Búfer para recibir datos

    // Crea un socket TCP
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation error"); // Imprime un mensaje de error si falla
        return -2; // Devuelve un código de error
    }

    // Configura la dirección del servidor
    serv_addr.sin_family = AF_INET; // Familia de direcciones (IPv4)
    serv_addr.sin_port = htons(get_server_port()); // Puerto del servidor (convertido a formato de red)
    if (inet_pton(AF_INET, get_server_ip(), &serv_addr.sin_addr) <= 0) { // Convierte la IP a formato binario
        fprintf(stderr, "Invalid address/Address not supported\n"); // Imprime un mensaje de error si falla
        close(sock); // Cierra el socket
        return -2; // Devuelve un código de error
    }

    // Intenta conectar al servidor con varios reintentos
    int retries = 3; // Número de reintentos permitidos
    while (retries > 0) {
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { // Intenta conectar
            perror("Connection failed, retrying"); // Imprime un mensaje de error si falla
            retries--; // Decrementa el contador de reintentos
            sleep(1); // Espera 1 segundo antes de reintentar
        } else {
            break; // Sale del bucle si la conexión es exitosa
        }
    }
    if (retries == 0) { // Si se agotaron los reintentos
        fprintf(stderr, "Connection failed after retries\n"); // Imprime un mensaje de error
        close(sock); // Cierra el socket
        return -2; // Devuelve un código de error
    }

    // Construye el mensaje JSON para la solicitud
    int offset = snprintf(send_buffer, sizeof(send_buffer),
        "{\"operation\":%d,\"key\":%d,\"value1\":\"%s\",\"N_value2\":%d,\"V_value2\":[",
        operation, key, value1, N_value2); // Escribe los campos iniciales del JSON
    for (int i = 0; i < N_value2; i++) { // Añade los valores del array "V_value2"
        int n = snprintf(send_buffer + offset, sizeof(send_buffer) - offset, "%.6g", V_value2[i]);
        offset += n; // Actualiza el offset
        if (i < N_value2 - 1) { // Añade una coma entre los valores
            if ((size_t)offset < sizeof(send_buffer) - 1) {
                send_buffer[offset++] = ','; // Añade la coma
                send_buffer[offset] = '\0'; // Asegura que el búfer termine en nulo
            }
        }
    }
    snprintf(send_buffer + offset, sizeof(send_buffer) - offset, "],\"value3\":{\"x\":%d,\"y\":%d}}", value3.x, value3.y); // Añade el campo "value3"

    // Envía el mensaje al servidor
    if (send(sock, send_buffer, strlen(send_buffer), 0) < 0) {
        perror("Send failed"); // Imprime un mensaje de error si falla
        close(sock); // Cierra el socket
        return -2; // Devuelve un código de error
    }

    // Lee la respuesta del servidor
    ssize_t bytes_read = recv(sock, recv_buffer, sizeof(recv_buffer) - 1, 0); // Lee los datos del socket
    if (bytes_read < 0) {
        perror("Read failed"); // Imprime un mensaje de error si falla
        close(sock); // Cierra el socket
        return -2; // Devuelve un código de error
    }
    recv_buffer[bytes_read] = '\0'; // Asegura que el búfer termine en nulo

    // Parseo de la respuesta JSON
    int result = -1;
    if (sscanf(recv_buffer, "{\"result\":%d", &result) != 1) { // Extrae el campo "result"
        fprintf(stderr, "Error parsing result from JSON\n"); // Imprime un mensaje de error si falla
        close(sock); // Cierra el socket
        return -2; // Devuelve un código de error
    }
    if (result == 0) { // Si la operación fue exitosa
        char *p = strstr(recv_buffer, "\"value1\":\""); // Busca el campo "value1"
        if (p) {
            p += strlen("\"value1\":\"");
            char *q = strchr(p, '\"'); // Encuentra el final del valor
            if (q) {
                size_t len = q - p;
                if (len >= 256) len = 255; // Limita el tamaño del valor
                strncpy(recv_value1, p, len); // Copia el valor
                recv_value1[len] = '\0'; // Asegura que termine en nulo
            }
        }
        p = strstr(recv_buffer, "\"N_value2\":"); // Busca el campo "N_value2"
        if (p) {
            p += strlen("\"N_value2\":");
            sscanf(p, "%d", recv_N_value2); // Extrae el valor
        }
        p = strstr(recv_buffer, "\"V_value2\":["); // Busca el campo "V_value2"
        if (p) {
            p += strlen("\"V_value2\":[");
            for (int i = 0; i < *recv_N_value2; i++) { // Extrae los valores del array
                double d;
                if (sscanf(p, "%lf", &d) == 1) {
                    recv_V_value2[i] = d;
                }
                char *comma = strchr(p, ','); // Busca la siguiente coma
                char *bracket = strchr(p, ']'); // Busca el cierre del array
                if (comma && (!bracket || comma < bracket)) {
                    p = comma + 1; // Avanza al siguiente valor
                } else {
                    break; // Sale del bucle si no hay más valores
                }
            }
        }
        p = strstr(recv_buffer, "\"value3\":{\"x\":"); // Busca el campo "value3"
        if (p) {
            int x, y;
            p += strlen("\"value3\":{\"x\":");
            sscanf(p, "%d,\"y\":%d", &x, &y); // Extrae las coordenadas
            recv_value3->x = x;
            recv_value3->y = y;
        }
    }
    close(sock); // Cierra el socket
    return result; // Devuelve el resultado de la operación
}

// Implementación de las funciones de la API para interactuar con el servidor
int destroy(void) {
    char recv_value1[256];
    int recv_N_value2;
    double recv_V_value2[32];
    struct Coord recv_value3;
    return send_request(1, 0, "", 0, NULL, (struct Coord){0, 0},
                        recv_value1, &recv_N_value2, recv_V_value2, &recv_value3);
}

int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    char recv_value1[256];
    int recv_N_value2;
    double recv_V_value2[32];
    struct Coord recv_value3;
    return send_request(2, key, value1, N_value2, V_value2, value3,
                        recv_value1, &recv_N_value2, recv_V_value2, &recv_value3);
}

int get_value(int key, char *value1, int *recv_N_value2, double *recv_V_value2, struct Coord *recv_value3) {
    return send_request(3, key, "", 0, NULL, (struct Coord){0, 0},
                        value1, recv_N_value2, recv_V_value2, recv_value3);
}

int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    char recv_value1[256];
    int recv_N_value2;
    double recv_V_value2[32];
    struct Coord recv_value3;
    return send_request(4, key, value1, N_value2, V_value2, value3,
                        recv_value1, &recv_N_value2, recv_V_value2, &recv_value3);
}

int delete_key(int key) {
    char recv_value1[256];
    int recv_N_value2;
    double recv_V_value2[32];
    struct Coord recv_value3;
    return send_request(5, key, "", 0, NULL, (struct Coord){0, 0},
                        recv_value1, &recv_N_value2, recv_V_value2, &recv_value3);
}

int exist(int key) {
    char recv_value1[256];
    int recv_N_value2;
    double recv_V_value2[32];
    struct Coord recv_value3;
    return send_request(6, key, "", 0, NULL, (struct Coord){0, 0},
                        recv_value1, &recv_N_value2, recv_V_value2, &recv_value3);
}