#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "claves.h"       // generado por rpcgen
#include "claves_real.h"  // contiene real_set_value, real_get_value, etc.

/* IMPORTANTE: todas las funciones *_1_svc deben devolver un puntero estático */

int *destroy_1_svc(void *arg, struct svc_req *req) {
    static int result;
    result = real_destroy();
    return &result;
}

int *set_value_1_svc(Tupla *t, struct svc_req *req) {
    static int result;
    result = real_set_value(t->key, t->value1, t->N_value2, t->V_value2.V_value2_val, t->value3);
    return &result;
}

TuplaRet *get_value_1_svc(int *key, struct svc_req *req) {
    static TuplaRet ret;
    static double vcopy[MAX_VEC]; // Almacén estático para el vector devuelto
    char value1[256];
    int N_value2;
    struct Coord coord;

    int result = real_get_value(*key, value1, &N_value2, vcopy, &coord);

    ret.result = result;

    if (result == 0) {
        ret.value1 = value1;
        ret.N_value2 = N_value2;
        ret.V_value2.V_value2_len = N_value2;
        ret.V_value2.V_value2_val = vcopy;
        ret.value3 = coord;
    }

    return &ret;
}

int *delete_key_1_svc(int *key, struct svc_req *req) {
    static int result;
    result = real_delete_key(*key);
    return &result;
}

int *modify_value_1_svc(Tupla *t, struct svc_req *req) {
    static int result;
    result = real_modify_value(t->key, t->value1, t->N_value2, t->V_value2.V_value2_val, t->value3);
    return &result;
}

int *exist_1_svc(int *key, struct svc_req *req) {
    static int result;
    result = real_exist(*key);
    return &result;
}
