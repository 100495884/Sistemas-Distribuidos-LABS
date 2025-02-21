#include <stdio.h>
#include <stdlib.h>
#include "lib.h"
int N = 10 ;
char *A = "nombre" ;
int E = 1 ;
int V = 0x123 ;
int main ( int argc, char *argv[] ) {
int ret, val ;
ret = init(A, N) ;
if (ret < 0) { printf("init: error code %d\n", ret); exit(-1); }
ret = set (A, E, V) ;
if (ret < 0) { printf("set: error code %d\n", ret); exit(-1); }
ret = get (A, E, &val) ;
if (ret < 0) { printf("get: error code %d\n", ret); exit(-1); }
return 0 ;
}





int a_neltos = 0 ; // = N ; número de arrays
int * a_values[100] ; // = [ [0…N1], [0…N2], … [0…NN] ] ; lista de punteros
char * a_keys [100] ; // = [ “key1”, “key2”, … “keyN” ] ; lista de claves

int buscar (char *nombre){
    int index = -1 ;
    for (int i=0; i<a_neltos; i++){
        if (!strcmp(a_keys[i], nombre)) {
        return i;
        }
    }
    return index;
}

int insertar (char *nombre, int N)
{
    a_values[a_neltos] = (int *)malloc(N*sizeof(int)) ;
    if (a_values[a_neltos] == NULL) {
        return -1 ; // en caso de error => -1
    }
    a_keys[a_neltos] = strdup(nombre) ; // strdup hace un malloc y copia el string para no perder la info en la pila 
    if (a_keys[a_neltos] == NULL) {
        free(a_values[a_neltos]);
        return -1 ; // en caso de error => -1
    }
    a_neltos++ ;
    return 1 ; // todo bien => devolver 1
}

// Inserta el valor en la posición i del array nombre.
int set (char *nombre, int i, int valor)
{
int index = buscar(nombre) ;
if (index == -1) return -1 ; // Si error => devolver -1
a_values[index][i] = valor ;
return 1;
}
// Recuperar el valor del elemento i del array nombre.
int get (char*nombre, int i, int *valor)
{
int index = buscar(nombre) ;
if (index == -1) return -1 ; // Si error => devolver -1
*valor = a_values[index][i] ;
return 1;
}
