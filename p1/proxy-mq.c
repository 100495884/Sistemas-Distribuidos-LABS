#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include "claves.h"

#define MQ_SERVER "/mq_server"
#define MAX_MSG_SIZE 1024

// Estructura para mensajes
typedef struct {
    int operation;
    int key;
    char value1[255];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
    char q_name[1024];
} message_t;

int send_request(message_t *request, message_t *response) {
    mqd_t mq_server, mq_client;
    char client_queue[1024];
    int prio;
    struct mq_attr attr = {0, 10, MAX_MSG_SIZE, 0};
    
    // Crear nombre único para la cola del cliente
    sprintf(client_queue, "/CLIENTE_%d", getpid());
    strcpy(request->q_name, client_queue);
    
    // Abrir la cola del servidor
    mq_server = mq_open(MQ_SERVER, O_WRONLY);
    if (mq_server == -1) return -2;
    
    // Abrir la cola del cliente para recibir respuesta
    mq_client = mq_open(client_queue, O_CREAT | O_RDONLY, 0666, &attr);
    if (mq_client == -1) {
        mq_close(mq_server);
        return -2;
    }
    
    // Enviar solicitud
    if (mq_send(mq_server, (char *)request, sizeof(message_t), 0) == -1) {
        mq_close(mq_server);
        mq_close(mq_client);
        mq_unlink(client_queue);
        return -2;
    }
    mq_close(mq_server);
    
    // Recibir respuesta
    if (mq_receive(mq_client, (char *)response, MAX_MSG_SIZE, &prio) == -1) {
        mq_close(mq_client);
        mq_unlink(client_queue);
        return -2;
    }
    
    mq_close(mq_client);
    mq_unlink(client_queue);
    return response->operation;
}

int destroy() {
    message_t request = {1, 0, "", 0, {0}, {0, 0}, ""};
    message_t response;
    return send_request(&request, &response);
}

int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    if (strlen(value1) > 255 || N_value2 > 32) return -1;
    message_t request = {2, key, "", N_value2, {0}, {0, 0}, ""};
    message_t response;
    strcpy(request.value1, value1);
    memcpy(request.V_value2, V_value2, N_value2 * sizeof(double));
    request.value3 = value3;
    return send_request(&request, &response);
}

int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
    message_t request = {3, key, "", 0, {0}, {0, 0}, ""};
    message_t response;
    int result = send_request(&request, &response);
    if (result == 0) {
        strcpy(value1, response.value1);
        *N_value2 = response.N_value2;
        memcpy(V_value2, response.V_value2, response.N_value2 * sizeof(double));
        *value3 = response.value3;
    }
    return result;
}

int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    if (strlen(value1) > 255 || N_value2 > 32) return -1;
    message_t request = {4, key, "", N_value2, {0}, {0, 0}, ""};
    message_t response;
    strcpy(request.value1, value1);
    memcpy(request.V_value2, V_value2, N_value2 * sizeof(double));
    request.value3 = value3;
    return send_request(&request, &response);
}

int delete_key(int key) {
    message_t request = {5, key, "", 0, {0}, {0, 0}, ""};
    message_t response;
    return send_request(&request, &response);
}

int exist(int key) {
    message_t request = {6, key, "", 0, {0}, {0, 0}, ""};
    message_t response;
    return send_request(&request, &response);
}
