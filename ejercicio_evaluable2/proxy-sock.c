// proxy-sock.c
// Esta biblioteca implementa la lógica cliente para comunicarse con el servidor de tuplas mediante sockets TCP y mensajes JSON.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h> // Necesario para gethostbyname y struct hostent
#include "claves.h"

#define BUFFER_SIZE 4096 // Tamaño del buffer para enviar y recibir datos

// Devuelve la IP del servidor desde la variable de entorno IP_TUPLAS (o 127.0.0.1 por defecto)
static char* get_server_ip() {
    char *ip = getenv("IP_TUPLAS");
    return ip ? ip : "127.0.0.1";
}

// Devuelve el puerto del servidor desde la variable de entorno PORT_TUPLAS (o 8080 por defecto)
static int get_server_port() {
    char *port_str = getenv("PORT_TUPLAS");
    if (port_str) {
        int port = atoi(port_str);
        if (port > 0 && port <= 65535) return port;
    }
    return 8080;
}

// Envía una petición al servidor y procesa la respuesta
int send_request(int operation, int key, char *value1, int N_value2, double *V_value2, struct Coord value3,
                 char *recv_value1, int *recv_N_value2, double *recv_V_value2, struct Coord *recv_value3) {

    int sock;
    struct sockaddr_in serv_addr;
    char send_buffer[BUFFER_SIZE] = {0};
    char recv_buffer[BUFFER_SIZE] = {0};

    // Crear socket TCP
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation error");
        return -2;
    }

    // Configurar dirección del servidor
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(get_server_port());

    struct hostent *he = gethostbyname(get_server_ip());
    if (!he) {
        fprintf(stderr, "Invalid address/Address not supported: %s\n", get_server_ip());
        close(sock);
        return -2;
    }
    memcpy(&serv_addr.sin_addr, he->h_addr_list[0], he->h_length);

    // Conectar con el servidor con reintentos
    int retries = 3;
    while (retries-- > 0) {
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0) break;
        perror("Connection failed, retrying");
        sleep(1);
    }

    if (retries < 0) {
        fprintf(stderr, "Connection failed after retries\n");
        close(sock);
        return -2;
    }

    // Crear el mensaje JSON
    int offset = snprintf(send_buffer, sizeof(send_buffer),
        "{\"operation\":%d,\"key\":%d,\"value1\":\"%s\",\"N_value2\":%d,\"V_value2\":[",
        operation, key, value1, N_value2);

    for (int i = 0; i < N_value2; i++) {
        int n = snprintf(send_buffer + offset, sizeof(send_buffer) - offset, "%.6g", V_value2[i]);
        offset += n;
        if (i < N_value2 - 1 && (size_t)offset < sizeof(send_buffer) - 1) {
            send_buffer[offset++] = ',';
            send_buffer[offset] = '\0';
        }
    }

    snprintf(send_buffer + offset, sizeof(send_buffer) - offset,
             "],\"value3\":{\"x\":%d,\"y\":%d}}", value3.x, value3.y);

    // Enviar mensaje
    if (send(sock, send_buffer, strlen(send_buffer), 0) < 0) {
        perror("Send failed");
        close(sock);
        return -2;
    }

    // Recibir respuesta
    ssize_t bytes_read = recv(sock, recv_buffer, sizeof(recv_buffer) - 1, 0);
    if (bytes_read < 0) {
        perror("Read failed");
        close(sock);
        return -2;
    }
    recv_buffer[bytes_read] = '\0';

    // Parsear respuesta
    int result = -1;
    if (sscanf(recv_buffer, "{\"result\":%d", &result) != 1) {
        fprintf(stderr, "Error parsing result from JSON\n");
        close(sock);
        return -2;
    }

    if (result == 0) {
        char *p;

        // value1
        p = strstr(recv_buffer, "\"value1\":\"");
        if (p) {
            p += strlen("\"value1\":\"");
            char *q = strchr(p, '\"');
            if (q) {
                size_t len = q - p;
                if (len >= 256) len = 255;
                strncpy(recv_value1, p, len);
                recv_value1[len] = '\0';
            }
        }

        // N_value2
        p = strstr(recv_buffer, "\"N_value2\":");
        if (p) {
            p += strlen("\"N_value2\":");
            sscanf(p, "%d", recv_N_value2);
        }

        // V_value2
        p = strstr(recv_buffer, "\"V_value2\":[");
        if (p) {
            p += strlen("\"V_value2\":[");
            for (int i = 0; i < *recv_N_value2; i++) {
                double d;
                if (sscanf(p, "%lf", &d) == 1) {
                    recv_V_value2[i] = d;
                }
                char *comma = strchr(p, ',');
                char *bracket = strchr(p, ']');
                if (comma && (!bracket || comma < bracket)) {
                    p = comma + 1;
                } else {
                    break;
                }
            }
        }

        // value3
        p = strstr(recv_buffer, "\"value3\":{\"x\":");
        if (p) {
            int x, y;
            p += strlen("\"value3\":{\"x\":");
            sscanf(p, "%d,\"y\":%d", &x, &y);
            recv_value3->x = x;
            recv_value3->y = y;
        }
    }

    close(sock);
    return result;
}

// API pública

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
