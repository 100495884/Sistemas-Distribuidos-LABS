#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "claves.h"
#include "claves_real.h"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 4096

// Función auxiliar para extraer un campo numérico de la cadena JSON.
static int extract_int(const char *json, const char *field, int *value) {
    char *p = strstr(json, field);
    if (p) {
        p += strlen(field);
        return sscanf(p, "%d", value);
    }
    return 0;
}

// Función auxiliar para extraer un campo de cadena entre comillas.
static int extract_string(const char *json, const char *field, char *dest, size_t dest_size) {
    char *p = strstr(json, field);
    if (p) {
        p += strlen(field);
        char *q = strchr(p, '\"');
        if (q) {
            size_t len = q - p;
            if (len >= dest_size) len = dest_size - 1;
            strncpy(dest, p, len);
            dest[len] = '\0';
            return 1;
        }
    }
    return 0;
}

// Función que maneja la comunicación con un cliente mediante JSON.
void *handle_client(void *arg) {
    int sock = *(int *)arg;
    char buffer[BUFFER_SIZE] = {0};

    // Recibir el mensaje JSON completo.
    ssize_t bytes_read = read(sock, buffer, sizeof(buffer) - 1);
    if (bytes_read < 0) {
        perror("read failed");
        close(sock);
        free(arg);
        pthread_exit(NULL);
    }
    buffer[bytes_read] = '\0';

    // Variables para almacenar los datos recibidos.
    int operation = -1, key = 0, N_value2 = 0;
    char value1[256] = {0};
    double V_value2[32] = {0.0};
    struct Coord value3 = {0, 0};

    // Parseo básico del JSON entrante.
    if (extract_int(buffer, "\"operation\":", &operation) != 1) {
        fprintf(stderr, "Error: No se pudo parsear 'operation'\n");
        close(sock);
        free(arg);
        pthread_exit(NULL);
    }
    extract_int(buffer, "\"key\":", &key);
    extract_string(buffer, "\"value1\":\"", value1, sizeof(value1));
    extract_int(buffer, "\"N_value2\":", &N_value2);

    // Parseo del array V_value2.
    char *p = strstr(buffer, "\"V_value2\":[");
    if (p) {
        p += strlen("\"V_value2\":[");
        for (int i = 0; i < N_value2; i++) {
            if (sscanf(p, "%lf", &V_value2[i]) != 1) break;
            char *comma = strchr(p, ',');
            char *bracket = strchr(p, ']');
            if (comma && (!bracket || comma < bracket)) {
                p = comma + 1;
            } else {
                break;
            }
        }
    }
    // Parseo de value3.
    p = strstr(buffer, "\"value3\":{\"x\":");
    if (p) {
        p += strlen("\"value3\":{\"x\":");
        sscanf(p, "%d,\"y\":%d", &value3.x, &value3.y);
    }

    // Realizar la operación solicitada.
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

    // Construir respuesta JSON.
    char send_buffer[BUFFER_SIZE] = {0};
    if (result == 0) {
        size_t offset = snprintf(send_buffer, sizeof(send_buffer),
            "{\"result\":0,\"value1\":\"%s\",\"N_value2\":%d,\"V_value2\":[", value1, N_value2);
        for (int i = 0; i < N_value2; i++) {
            int n = snprintf(send_buffer + offset, sizeof(send_buffer) - offset, "%.6g", V_value2[i]);
            offset += n;
            if (i < N_value2 - 1) {
                if (offset < sizeof(send_buffer) - 1) {
                    send_buffer[offset++] = ',';
                    send_buffer[offset] = '\0';
                }
            }
        }
        snprintf(send_buffer + offset, sizeof(send_buffer) - offset, "],\"value3\":{\"x\":%d,\"y\":%d}}", value3.x, value3.y);
    } else {
        snprintf(send_buffer, sizeof(send_buffer), "{\"result\":%d}", result);
    }

    if (send(sock, send_buffer, strlen(send_buffer), 0) < 0) {
        perror("send failed");
    }

    close(sock);
    free(arg);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Error -> Uso: %s <PUERTO>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    if (port <= 0) {
        fprintf(stderr, "Puerto inválido.\n");
        exit(EXIT_FAILURE);
    }
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    int val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (void*) &val, sizeof(val));
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Servidor iniciado en el puerto %d\n", port);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    while (1) {
        int new_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_sock < 0) {
            perror("Fallo en accept");
            continue;
        }
    
        int *client_sock = malloc(sizeof(int));
        if (!client_sock) {
            perror("Fallo en malloc");
            close(new_sock);
            continue;
        }
        *client_sock = new_sock;
    
        pthread_t thread;
        if (pthread_create(&thread, &attr, handle_client, client_sock) != 0) {
            perror("Fallo en pthread_create");
            close(new_sock);
            free(client_sock);
            continue;
        }
    }
    pthread_attr_destroy(&attr);
    close(server_fd);
    return 0;
}
