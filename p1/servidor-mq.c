#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "claves.h"
#include "claves_real.h"

#define MQ_SERVER "/mq_server"
#define MAX_MSG_SIZE sizeof(message_t)

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

pthread_mutex_t sync_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sync_cond = PTHREAD_COND_INITIALIZER;
int sync_copied = 0;

void *handle_request(void *arg) {
    message_t p_local, response;
    mqd_t mq_client;

    pthread_mutex_lock(&sync_mutex);
    memcpy(&p_local, arg, sizeof(message_t));
    sync_copied = 1;
    pthread_cond_signal(&sync_cond);
    pthread_mutex_unlock(&sync_mutex);

    // Validaciones antes de contactar con el servidor
    if (strlen(p_local.value1) > 255 || p_local.N_value2 < 0 || p_local.N_value2 > 32) {
        response.operation = -1;
    } else {
        switch (p_local.operation) {
            case 1: response.operation = real_destroy(); break;
            case 2: response.operation = real_set_value(p_local.key, p_local.value1, p_local.N_value2, p_local.V_value2, p_local.value3); break;
            case 3: response.operation = real_get_value(p_local.key, response.value1, &response.N_value2, response.V_value2, &response.value3); break;
            case 4: response.operation = real_modify_value(p_local.key, p_local.value1, p_local.N_value2, p_local.V_value2, p_local.value3); break;
            case 5: response.operation = real_delete_key(p_local.key); break;
            case 6: response.operation = real_exist(p_local.key); break;
            default: response.operation = -1; break;
        }
    }

    mq_client = mq_open(p_local.q_name, O_WRONLY);
    if (mq_client != (mqd_t)-1) {
        mq_send(mq_client, (char*)&response, sizeof(response), 0);
        mq_close(mq_client);
    }

    pthread_exit(NULL);
}

int main() {
    mqd_t mq_server;
    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = 10,
        .mq_msgsize = MAX_MSG_SIZE,
        .mq_curmsgs = 0
    };
    
    mq_server = mq_open(MQ_SERVER, O_CREAT | O_RDONLY, 0666, &attr);
    if (mq_server == (mqd_t)-1) {
        perror("mq_open server");
        exit(EXIT_FAILURE);
    }

    printf("Servidor iniciado...\n");

    while(1) {
        message_t request;
        pthread_t thread;
        pthread_attr_t tattr;

        if(mq_receive(mq_server, (char*)&request, MAX_MSG_SIZE, NULL) == -1) {
            perror("mq_receive");
            continue;
        }

        pthread_attr_init(&tattr);
        pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
        
        pthread_create(&thread, &tattr, handle_request, &request);
        
        pthread_mutex_lock(&sync_mutex);
        while(!sync_copied) {
            pthread_cond_wait(&sync_cond, &sync_mutex);
        }
        sync_copied = 0;
        pthread_mutex_unlock(&sync_mutex);
    }

    mq_close(mq_server);
    mq_unlink(MQ_SERVER);
    return 0;
}