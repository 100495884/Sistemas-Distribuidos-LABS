#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpc/rpc.h>
#include "claves.h"  // generado por rpcgen
#include "claves_real.h"  // para usar la definición de struct Coord

#define SERVER_PROG CLAVES_PROG
#define SERVER_VERS CLAVES_VERS

static CLIENT *create_rpc_client() {
    char *server_ip = getenv("IP_TUPLAS");
    if (!server_ip) {
        fprintf(stderr, "Error: variable de entorno IP_TUPLAS no definida\n");
        return NULL;
    }

    CLIENT *clnt = clnt_create(server_ip, SERVER_PROG, SERVER_VERS, "tcp");
    if (!clnt) {
        clnt_pcreateerror("Error al crear el cliente RPC");
    }
    return clnt;
}

int destroy() {
    CLIENT *clnt = create_rpc_client();
    if (!clnt) return -1;

    int *result = destroy_1(NULL, clnt);
    int res = (result != NULL) ? *result : -1;

    clnt_destroy(clnt);
    return res;
}

int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    CLIENT *clnt = create_rpc_client();
    if (!clnt) return -1;

    Tupla t;
    t.key = key;
    t.value1 = value1;
    t.N_value2 = N_value2;
    t.V_value2.V_value2_len = N_value2;
    t.V_value2.V_value2_val = V_value2;
    t.value3 = value3;

    int *result = set_value_1(&t, clnt);
    int res = (result != NULL) ? *result : -1;

    clnt_destroy(clnt);
    return res;
}

int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
    CLIENT *clnt = create_rpc_client();
    if (!clnt) return -1;

    TuplaRet *ret = get_value_1(&key, clnt);
    if (!ret || ret->result != 0) {
        clnt_destroy(clnt);
        return -1;
    }

    strncpy(value1, ret->value1, 255);
    value1[255] = '\0';
    *N_value2 = ret->N_value2;

    for (int i = 0; i < ret->N_value2; i++) {
        V_value2[i] = ret->V_value2.V_value2_val[i];
    }

    *value3 = ret->value3;

    clnt_destroy(clnt);
    return 0;
}

int delete_key(int key) {
    CLIENT *clnt = create_rpc_client();
    if (!clnt) return -1;

    int *result = delete_key_1(&key, clnt);
    int res = (result != NULL) ? *result : -1;

    clnt_destroy(clnt);
    return res;
}

int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    CLIENT *clnt = create_rpc_client();
    if (!clnt) return -1;

    Tupla t;
    t.key = key;
    t.value1 = value1;
    t.N_value2 = N_value2;
    t.V_value2.V_value2_len = N_value2;
    t.V_value2.V_value2_val = V_value2;
    t.value3 = value3;

    int *result = modify_value_1(&t, clnt);
    int res = (result != NULL) ? *result : -1;

    clnt_destroy(clnt);
    return res;
}

int exist(int key) {
    CLIENT *clnt = create_rpc_client();
    if (!clnt) return -1;

    int *result = exist_1(&key, clnt);
    int res = (result != NULL) ? *result : -1;

    clnt_destroy(clnt);
    return res;
}
