#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "lines.h"

#define MAX_LINE 256

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

    // Aceptar conexiones y manejar mensajes
    int client_sd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[MAX_LINE];
    int n;

    fprintf(stderr, "Servidor escuchando en el puerto %d...\n", port);

    while (1) {
        // Aceptar una conexión de un cliente
        client_sd = accept(sd, (struct sockaddr *)&client_addr, &client_len);
        if (client_sd < 0) {
            perror("Error en accept");
            continue;
        }

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
        break;
    }

    // Cerrar el socket del servidor
    close(sd);
    return 0;
}