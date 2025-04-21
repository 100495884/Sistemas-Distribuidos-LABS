#ifndef CLAVES_API_H
#define CLAVES_API_H

#include "claves.h"  // Usa la definición de Coord que ya viene de rpcgen

int set_value(int, char *, int, double *, struct Coord);
int get_value(int, char *, int *, double *, struct Coord *);
int delete_key(int);
int modify_value(int, char *, int, double *, struct Coord);
int exist(int);
int destroy(void);

#endif
