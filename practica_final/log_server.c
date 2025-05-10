#include <stdio.h>
#include <string.h>
#include "log.h"

static char last_user[256] = "";
static char last_op[256] = "";
static char last_time[256] = "";

void *log_event_1_svc(LogEntry *entrada, struct svc_req *rqstp) {
    if (entrada == NULL) return NULL;

    // Ignorar duplicado exacto
    if (strcmp(last_user, entrada->usuario) == 0 &&
        strcmp(last_op, entrada->operacion) == 0 &&
        strcmp(last_time, entrada->timestamp) == 0) {
        return NULL;
    }

    // Guardar último log
    strncpy(last_user, entrada->usuario, 255);
    strncpy(last_op, entrada->operacion, 255);
    strncpy(last_time, entrada->timestamp, 255);

    // Imprimir
    printf("%s %s %s\n", entrada->usuario, entrada->operacion, entrada->timestamp);
    fflush(stdout);
    return NULL;
}
