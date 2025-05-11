#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "log.h"

#define MAX_ENTRIES 100  // Número máximo de entradas a recordar

typedef struct {
    char usuario[256];
    char operacion[256];
    char timestamp[256];
} LogEntryCache;

static LogEntryCache entry_cache[MAX_ENTRIES];
static int cache_index = 0;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

// Función para verificar si el registro ya existe
static int is_duplicate(const LogEntry *entrada) {
    for (int i = 0; i < cache_index; i++) {
        if (strcmp(entry_cache[i].usuario, entrada->usuario) == 0 &&
            strcmp(entry_cache[i].operacion, entrada->operacion) == 0 &&
            strcmp(entry_cache[i].timestamp, entrada->timestamp) == 0) {
            return 1;  // Es duplicado
        }
    }
    return 0;  // No es duplicado
}

void *log_event_1_svc(LogEntry *entrada, struct svc_req *rqstp) {
    if (entrada == NULL) return NULL;

    pthread_mutex_lock(&log_mutex);

    // Verificar si es duplicado
    if (is_duplicate(entrada)) {
        pthread_mutex_unlock(&log_mutex);
        return NULL;
    }

    // Almacenar en caché (con rotación circular)
    strncpy(entry_cache[cache_index].usuario, entrada->usuario, 255);
    strncpy(entry_cache[cache_index].operacion, entrada->operacion, 255);
    strncpy(entry_cache[cache_index].timestamp, entrada->timestamp, 255);
    
    cache_index = (cache_index + 1) % MAX_ENTRIES;  // Rotación circular

    // Imprimir
    printf("%s %s %s\n", entrada->usuario, entrada->operacion, entrada->timestamp);

    pthread_mutex_unlock(&log_mutex);

    return NULL;
}