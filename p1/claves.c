#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "claves.h"

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

int destroy() {
    Node *current = head;
    Node *temp;

    while (current != NULL) {
        temp = current;
        current = current->next;
        free(temp);  // Liberamos la memoria de cada nodo
    }

    head = NULL;  // La lista queda vacía
    return 0;
}

int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    // Validar si la clave ya existe
    if (exist(key)) {
        return -1;
    }

    // Validar rango de N_value2
    if (N_value2 < 1 || N_value2 > MAX_VECTOR) {
        return -1;
    }

    // Validar tamaño de value1
    if (strlen(value1) >= MAX_STRING) {
        return -1;
    }

    // Crear un nuevo nodo
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL) {
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

    return 0;  // Éxito
}

int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
    Node *current = head;

    // Buscar la clave en la lista
    while (current != NULL) {
        if (current->key == key) {
            // Copiar los valores en los parámetros de salida
            strncpy(value1, current->value1, MAX_STRING);
            *N_value2 = current->N_value2;
            memcpy(V_value2, current->value2, (*N_value2) * sizeof(double));
            *value3 = current->value3;
            return 0;  // Éxito
        }
        current = current->next;
    }

    return -1;  // Clave no encontrada
}

int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    Node *current = head;

    // Buscar la clave en la lista
    while (current != NULL) {
        if (current->key == key) {
            // Validar tamaño de value1
            if (strlen(value1) >= MAX_STRING) {
                return -1;
            }

            // Validar rango de N_value2
            if (N_value2 < 1 || N_value2 > MAX_VECTOR) {
                return -1;
            }

            // Modificar los valores
            strncpy(current->value1, value1, MAX_STRING);
            current->N_value2 = N_value2;
            memcpy(current->value2, V_value2, N_value2 * sizeof(double));
            current->value3 = value3;

            return 0;  // Éxito
        }
        current = current->next;
    }

    return -1;  // Clave no encontrada
}

int delete_key(int key) {
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
            return 0;  // Éxito
        }
        prev = current;
        current = current->next;
    }

    return -1;  // Clave no encontrada
}

int exist(int key) {
    Node *current = head;

    while (current != NULL) {
        if (current->key == key) {
            return 1;  // La clave existe
        }
        current = current->next;
    }

    return 0;  // La clave no existe
}
