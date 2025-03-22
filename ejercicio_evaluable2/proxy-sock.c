#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "claves.h"

#define SERVER_IP "127.0.0.1"  // IP del servidor
#define SERVER_PORT 8080       // Puerto del servidor

int send_request(int operation, int key, char *value1, int N_value2, double *V_value2, struct Coord value3, char *recv_value1, int *recv_N_value2, double *recv_V_value2, struct Coord *recv_value3) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -2;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        close(sock);
        return -2;
    }

    // Intentar conectar varias veces en caso de fallo
    int retries = 3;
    while (retries > 0) {
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("\nConnection Failed, retrying...\n");
            retries--;
            sleep(1);  // Esperar un segundo antes de reintentar
        } else {
            break;
        }
    }

    if (retries == 0) {
        printf("\nConnection Failed after retries\n");
        close(sock);
        return -2;
    }

    // Serializar los datos
    int offset = 0;
    buffer[offset++] = operation;
    memcpy(buffer + offset, &key, sizeof(int));
    offset += sizeof(int);
    strncpy(buffer + offset, value1, 255);
    offset += 255;
    memcpy(buffer + offset, &N_value2, sizeof(int));
    offset += sizeof(int);
    memcpy(buffer + offset, V_value2, N_value2 * sizeof(double));
    offset += N_value2 * sizeof(double);
    memcpy(buffer + offset, &value3, sizeof(struct Coord));
    offset += sizeof(struct Coord);

    // Enviar los datos al servidor
    if (send(sock, buffer, offset, 0) < 0) {
        printf("\nSend failed\n");
        close(sock);
        return -2;
    }

    // Recibir la respuesta del servidor
    ssize_t bytes_read = read(sock, buffer, 1024);
    if (bytes_read < 0) {
        printf("\nRead failed\n");
        close(sock);
        return -2;
    }

    // Deserializar la respuesta
    offset = 0;
    int result = buffer[offset++];
    if (result == 0) {
        strncpy(recv_value1, buffer + offset, 255);
        offset += 255;
        memcpy(recv_N_value2, buffer + offset, sizeof(int));
        offset += sizeof(int);
        memcpy(recv_V_value2, buffer + offset, *recv_N_value2 * sizeof(double));
        offset += *recv_N_value2 * sizeof(double);
        memcpy(recv_value3, buffer + offset, sizeof(struct Coord));
    }

    close(sock);
    return result;
}

int destroy() {
    char recv_value1[256];
    int recv_N_value2;
    double recv_V_value2[32];
    struct Coord recv_value3;
    return send_request(1, 0, "", 0, NULL, (struct Coord){0, 0}, recv_value1, &recv_N_value2, recv_V_value2, &recv_value3);
}

int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    char recv_value1[256];
    int recv_N_value2;
    double recv_V_value2[32];
    struct Coord recv_value3;
    return send_request(2, key, value1, N_value2, V_value2, value3, recv_value1, &recv_N_value2, recv_V_value2, &recv_value3);
}

int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
    return send_request(3, key, "", 0, NULL, (struct Coord){0, 0}, value1, N_value2, V_value2, value3);
}

int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    char recv_value1[256];
    int recv_N_value2;
    double recv_V_value2[32];
    struct Coord recv_value3;
    return send_request(4, key, value1, N_value2, V_value2, value3, recv_value1, &recv_N_value2, recv_V_value2, &recv_value3);
}

int delete_key(int key) {
    char recv_value1[256];
    int recv_N_value2;
    double recv_V_value2[32];
    struct Coord recv_value3;
    return send_request(5, key, "", 0, NULL, (struct Coord){0, 0}, recv_value1, &recv_N_value2, recv_V_value2, &recv_value3);
}

int exist(int key) {
    char recv_value1[256];
    int recv_N_value2;
    double recv_V_value2[32];
    struct Coord recv_value3;
    return send_request(6, key, "", 0, NULL, (struct Coord){0, 0}, recv_value1, &recv_N_value2, recv_V_value2, &recv_value3);
}