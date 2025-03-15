#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>  // Incluimos la biblioteca de hilos
#include "lines.h"

#define MAX_LINE 256

// Función que maneja la comunicación con un cliente
void *handle_client(void *arg) {
    int client_sd = *(int *)arg;  // Obtenemos el descriptor de socket del cliente
    char buffer[MAX_LINE];
    int n;

    // Bucle de recepción/envió de mensajes
    while (1) {
        // Recibir un mensaje del cliente
        n = readLine(client_sd, buffer, MAX_LINE);
        if (n == -1) {
            perror("Error en readLine");
            break;
        }

        // Si el cliente envía "EXIT", salir del bucle
        if (strcmp(buffer, "EXIT") == 0) {
            printf("Cliente envió EXIT. Cerrando conexión...\n");
            break;
        }

        // Devolver el mismo mensaje al cliente
        if (sendMessage(client_sd, buffer, n + 1) == -1) {
            perror("Error en sendMessage");
            break;
        }
    }

    // Cerrar la conexión con el cliente
    close(client_sd);

    // Liberar la memoria asignada para el descriptor de socket
    free(arg);

    // Terminar el hilo
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int sd;
    int val;
    int err;

    // Verificar que se pasó el puerto como argumento
    if (argc != 2) {
        printf("Usage: server <port>\n");
        exit(0);
    }

    // Crear el socket
    sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sd < 0) {
        perror("Error en socket");
        exit(1);
    }

    // Configurar el socket para reutilizar la dirección
    val = 1;
    err = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(int));
    if (err < 0) {
        perror("Error en setsockopt");
        exit(1);
    }

    // Configurar la estructura de dirección del servidor
    struct sockaddr_in server_addr;
    int port = atoi(argv[1]);

    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Atar el socket a la dirección y puerto
    if (bind(sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error en bind");
        close(sd);
        exit(1);
    }

    // Escuchar conexiones entrantes
    if (listen(sd, 5) < 0) {
        perror("Error en listen");
        close(sd);
        exit(1);
    }

    fprintf(stderr, "Servidor escuchando en el puerto %d...\n", port);

    // Configurar atributos de los hilos
    pthread_attr_t attr;
    pthread_attr_init(&attr);  // Inicializar los atributos del hilo
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);  // Hilos "detached"

    // Bucle principal para aceptar conexiones
    while (1) {
        // Aceptar una conexión de un cliente
        int client_sd;
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        client_sd = accept(sd, (struct sockaddr *)&client_addr, &client_len);
        if (client_sd < 0) {
            perror("Error en accept");
            continue;
        }

        // Reservar memoria para el descriptor de socket del cliente
        int *client_sd_ptr = (int *)malloc(sizeof(int));
        if (client_sd_ptr == NULL) {
            perror("Error en malloc");
            close(client_sd);
            continue;
        }
        *client_sd_ptr = client_sd;  // Guardar el descriptor de socket

        // Crear un hilo para manejar al cliente
        pthread_t thread;
        if (pthread_create(&thread, &attr, handle_client, (void *)client_sd_ptr) != 0) {
            perror("Error en pthread_create");
            free(client_sd_ptr);
            close(client_sd);
            continue;
        }
    }

    // Destruir los atributos de los hilos (esto nunca se ejecutará en este ejemplo)
    pthread_attr_destroy(&attr);

    // Cerrar el socket del servidor (esto nunca se ejecutará en este ejemplo)
    close(sd);
    return 0;
}