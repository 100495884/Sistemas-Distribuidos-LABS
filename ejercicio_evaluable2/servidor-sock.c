#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "claves.h"
#include "claves_real.h"

#define PORT 8080
#define MAX_CLIENTS 10

// Función que maneja la comunicación con un cliente
void *handle_client(void *arg) {
    int sock = *(int *)arg;
    char buffer[1024] = {0};
    int operation, key, N_value2;
    char value1[256];
    double V_value2[32];
    struct Coord value3;

    // Recibir los datos del cliente
    ssize_t bytes_read = read(sock, buffer, 1024);
    if (bytes_read < 0) {
        perror("read failed");
        close(sock);
        free(arg);
        pthread_exit(NULL);
    }

    // Deserializar los datos
    int offset = 0;
    operation = buffer[offset++];
    memcpy(&key, buffer + offset, sizeof(int));
    offset += sizeof(int);
    strncpy(value1, buffer + offset, 255);
    offset += 255;
    memcpy(&N_value2, buffer + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(V_value2, buffer + offset, N_value2 * sizeof(double));
    offset += N_value2 * sizeof(double);
    memcpy(&value3, buffer + offset, sizeof(struct Coord));

    // Realizar la operación correspondiente
    int result;
    switch (operation) {
        case 1: result = real_destroy(); break;
        case 2: result = real_set_value(key, value1, N_value2, V_value2, value3); break;
        case 3: result = real_get_value(key, value1, &N_value2, V_value2, &value3); break;
        case 4: result = real_modify_value(key, value1, N_value2, V_value2, value3); break;
        case 5: result = real_delete_key(key); break;
        case 6: result = real_exist(key); break;
        default: result = -1; break;
    }

    // Serializar la respuesta
    offset = 0;
    buffer[offset++] = result;
    if (result == 0) {
        strncpy(buffer + offset, value1, 255);
        offset += 255;
        memcpy(buffer + offset, &N_value2, sizeof(int));
        offset += sizeof(int);
        memcpy(buffer + offset, V_value2, N_value2 * sizeof(double));
        offset += N_value2 * sizeof(double);
        memcpy(buffer + offset, &value3, sizeof(struct Coord));
    }

    // Enviar la respuesta al cliente
    if (send(sock, buffer, offset, 0) < 0) {
        perror("send failed");
    }

    close(sock);
    free(arg);
    pthread_exit(NULL);
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Crear el socket y configurar el servidor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor iniciado en el puerto %d\n", PORT);

    // Configurar atributos de los hilos
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    // Bucle principal para aceptar conexiones
    while (1) {
        int *new_sock = malloc(sizeof(int));
        if (!new_sock) {
            perror("malloc failed");
            continue;
        }

        if ((*new_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            free(new_sock);
            continue;
        }

        // Crear un hilo para manejar la conexión
        pthread_t thread;
        if (pthread_create(&thread, &attr, handle_client, new_sock) != 0) {
            perror("pthread_create");
            close(*new_sock);
            free(new_sock);
        }
    }

    // Destruir los atributos de los hilos
    pthread_attr_destroy(&attr);

    // Cerrar el socket del servidor
    close(server_fd);
    return 0;
}