#ifndef REAL_H
#define REAL_H

#include "claves.h"

int real_destroy(void);
int real_set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3);
int real_get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3);
int real_modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3);
int real_delete_key(int key);
int real_exist(int key);

#endif