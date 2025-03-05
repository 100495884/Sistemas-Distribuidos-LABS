#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <mqueue.h>
#include "claves.h"
#include "claves_real.h"

#define MQ_SERVER "/mq_server"
#define MQ_CLIENT "/mq_client"
#define MAX_MSG_SIZE 1024

// Estructura para mensajes
typedef struct {
    int operation;
    int key;
    char value1[255];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
} message_t;

void *handle_request(void *arg) {
    message_t request, response;
    mqd_t mq_client;
    memcpy(&request, arg, sizeof(message_t));
    free(arg);

    // Validaciones antes de contactar con el servidor
    if (strlen(request.value1) > 255 || request.N_value2 > 32) {
        response.operation = -1;
    } else {
        // Procesar la solicitud
        switch (request.operation) {
            case 1: response.operation = _destroy(); break;
            case 2: response.operation = _set_value(request.key, request.value1, request.N_value2, request.V_value2, request.value3); break;
            case 3: response.operation = _get_value(request.key, response.value1, &response.N_value2, response.V_value2, &response.value3); break;
            case 4: response.operation = _modify_value(request.key, request.value1, request.N_value2, request.V_value2, request.value3); break;
            case 5: response.operation = _delete_key(request.key); break;
            case 6: response.operation = _exist(request.key); break;
            default: response.operation = -1; break;
        }
    }

    // Enviar respuesta al cliente
    mq_client = mq_open(MQ_CLIENT, O_WRONLY);
    if (mq_client == -1) {
        response.operation = -2;
    } else {
        if (mq_send(mq_client, (char *)&response, sizeof(response), 0) == -1) {
            response.operation = -2;
        }
        mq_close(mq_client);
    }
    
    return NULL;
}

int main() {
    mqd_t mq_server;
    struct mq_attr attr = {0, 10, MAX_MSG_SIZE, 0};
    mq_server = mq_open(MQ_SERVER, O_CREAT | O_RDONLY, 0666, &attr);
    if (mq_server == -1) {
        perror("Error abriendo cola de mensajes");
        exit(EXIT_FAILURE);
    }
    
    printf("Servidor iniciado y esperando peticiones...\n");
    
    while (1) {
        message_t *request = malloc(sizeof(message_t));
        if (mq_receive(mq_server, (char *)request, MAX_MSG_SIZE, NULL) > 0) {
            pthread_t thread;
            pthread_create(&thread, NULL, handle_request, (void *)request);
            pthread_detach(thread);
        } else {
            free(request);
        }
    }
    
    mq_close(mq_server);
    mq_unlink(MQ_SERVER);
    return 0;
}