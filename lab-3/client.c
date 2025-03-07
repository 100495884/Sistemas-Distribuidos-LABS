#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "lines.h"

#define MAX_LINE 	256

int main(int argc, char *argv[])
{
    int sd;
    struct sockaddr_in server_addr;
    struct hostent *hp;

    // Verificar que se pasaron los argumentos correctos
    if (argc != 3) {
        printf("Usage: client <serverAddress> <serverPort>\n");
        exit(0);
    }

    // Crear el socket
    sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sd < 0){
        perror("Error en socket");
        exit(1);
    }

    // Obtener la dirección del servidor
    bzero((char *)&server_addr, sizeof(server_addr));
    hp = gethostbyname(argv[1]);
    if (hp == NULL) {
        perror("Error en gethostbyname");
        exit(1);
    }

    // Configurar la estructura de dirección del servidor
    memcpy(&(server_addr.sin_addr), hp->h_addr, hp->h_length);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));

    // Conectar al servidor
    if (connect(sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error en connect");
        close(sd);
        exit(1);
    }

    char buffer[MAX_LINE];
    int n;

    // Bucle de envío/recepción de mensajes
    while (1) {
        // Leer una línea desde la entrada estándar (teclado)
        printf("Introduce un mensaje (o 'EXIT' para salir): ");
        n = readLine(0, buffer, MAX_LINE);
        if (n == -1) {
            perror("Error en readLine");
            break;
        }

        // Enviar el mensaje al servidor
        if (sendMessage(sd, buffer, n + 1) == -1) {  // n + 1 para incluir el '\0'
            perror("Error en sendMessage");
            break;
        }

        // Si el usuario escribe "EXIT", salir del bucle
        if (strcmp(buffer, "EXIT") == 0) {
            printf("Saliendo...\n");
            break;
        }

        // Recibir la respuesta del servidor
        n = readLine(sd, buffer, MAX_LINE);
        if (n == -1) {
            perror("Error en readLine");
            break;
        }

        // Imprimir la respuesta
        printf("Respuesta del servidor: %s\n", buffer);
    }

    // Cerrar la conexión
    close(sd);
    return 0;
}