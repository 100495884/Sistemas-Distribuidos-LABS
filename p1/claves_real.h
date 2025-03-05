#ifndef REAL_H
#define REAL_H

#include "claves.h"

int _destroy(void);
int _set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3);
int _get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3);
int _modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3);
int _delete_key(int key);
int _exist(int key);

#endif