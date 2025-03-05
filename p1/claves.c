#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>  // Biblioteca para hilos y mutex
#include "claves.h"
#include "claves_real.h"

#define MAX_STRING 255
#define MAX_VECTOR 32

// Estructura para almacenar una tupla en la lista enlazada
typedef struct Node {
    int key;
    char value1[MAX_STRING];  // Cadena de caracteres
    int N_value2;
    double value2[MAX_VECTOR];  // Vector de doubles
    struct Coord value3;  // Estructura Coord
    struct Node *next;  // Puntero al siguiente nodo
} Node;

// Puntero al inicio de la lista (almacenará todas las tuplas)
Node *head = NULL;

// Definir un mutex global
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int _destroy() {
    pthread_mutex_lock(&mutex);  // Bloquear el mutex

    Node *current = head;
    Node *temp;

    while (current != NULL) {
        temp = current;
        current = current->next;
        free(temp);  // Liberamos la memoria de cada nodo
    }

    head = NULL;  // La lista queda vacía

    pthread_mutex_unlock(&mutex);  // Desbloquear el mutex
    return 0;
}

int _set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    pthread_mutex_lock(&mutex);  // Bloquear el mutex

    // Validar si la clave ya existe
    if (exist(key)) {
        pthread_mutex_unlock(&mutex);  // Desbloquear el mutex
        return -1;
    }

    // Validar rango de N_value2
    if (N_value2 < 1 || N_value2 > MAX_VECTOR) {
        pthread_mutex_unlock(&mutex);  // Desbloquear el mutex
        return -1;
    }

    // Validar tamaño de value1
    if (strlen(value1) >= MAX_STRING) {
        pthread_mutex_unlock(&mutex);  // Desbloquear el mutex
        return -1;
    }

    // Crear un nuevo nodo
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL) {
        pthread_mutex_unlock(&mutex);  // Desbloquear el mutex
        return -1;  // Error de memoria
    }

    // Asignar valores a la tupla
    new_node->key = key;
    strncpy(new_node->value1, value1, MAX_STRING);  // Copiar cadena con seguridad
    new_node->N_value2 = N_value2;
    memcpy(new_node->value2, V_value2, N_value2 * sizeof(double));  // Copiar el vector
    new_node->value3 = value3;
    new_node->next = head;  // Insertar al inicio de la lista

    // Actualizar el puntero de la lista
    head = new_node;

    pthread_mutex_unlock(&mutex);  // Desbloquear el mutex
    return 0;  // Éxito
}

int _get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
    pthread_mutex_lock(&mutex);  // Bloquear el mutex

    Node *current = head;

    // Buscar la clave en la lista
    while (current != NULL) {
        if (current->key == key) {
            // Copiar los valores en los parámetros de salida
            strncpy(value1, current->value1, MAX_STRING);
            *N_value2 = current->N_value2;
            memcpy(V_value2, current->value2, (*N_value2) * sizeof(double));
            *value3 = current->value3;

            pthread_mutex_unlock(&mutex);  // Desbloquear el mutex
            return 0;  // Éxito
        }
        current = current->next;
    }

    pthread_mutex_unlock(&mutex);  // Desbloquear el mutex
    return -1;  // Clave no encontrada
}

int _modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    pthread_mutex_lock(&mutex);  // Bloquear el mutex

    Node *current = head;

    // Buscar la clave en la lista
    while (current != NULL) {
        if (current->key == key) {
            // Validar tamaño de value1
            if (strlen(value1) >= MAX_STRING) {
                pthread_mutex_unlock(&mutex);  // Desbloquear el mutex
                return -1;
            }

            // Validar rango de N_value2
            if (N_value2 < 1 || N_value2 > MAX_VECTOR) {
                pthread_mutex_unlock(&mutex);  // Desbloquear el mutex
                return -1;
            }

            // Modificar los valores
            strncpy(current->value1, value1, MAX_STRING);
            current->N_value2 = N_value2;
            memcpy(current->value2, V_value2, N_value2 * sizeof(double));
            current->value3 = value3;

            pthread_mutex_unlock(&mutex);  // Desbloquear el mutex
            return 0;  // Éxito
        }
        current = current->next;
    }

    pthread_mutex_unlock(&mutex);  // Desbloquear el mutex
    return -1;  // Clave no encontrada
}

int _delete_key(int key) {
    pthread_mutex_lock(&mutex);  // Bloquear el mutex

    Node *current = head;
    Node *prev = NULL;

    // Buscar la clave en la lista
    while (current != NULL) {
        if (current->key == key) {
            // Si es el primer nodo de la lista
            if (prev == NULL) {
                head = current->next;
            } else {
                prev->next = current->next;
            }

            free(current);  // Liberar la memoria del nodo eliminado

            pthread_mutex_unlock(&mutex);  // Desbloquear el mutex
            return 0;  // Éxito
        }
        prev = current;
        current = current->next;
    }

    pthread_mutex_unlock(&mutex);  // Desbloquear el mutex
    return -1;  // Clave no encontrada
}

int _exist(int key) {
    pthread_mutex_lock(&mutex);  // Bloquear el mutex

    Node *current = head;

    while (current != NULL) {
        if (current->key == key) {
            pthread_mutex_unlock(&mutex);  // Desbloquear el mutex
            return 1;  // La clave existe
        }
        current = current->next;
    }

    pthread_mutex_unlock(&mutex);  // Desbloquear el mutex
    return 0;  // La clave no existe
}