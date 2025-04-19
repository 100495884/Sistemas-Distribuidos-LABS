/* claves.x - Interfaz RPC para gestión de tuplas */

const MAX_STR = 256;
const MAX_VEC = 32;

/* Estructura para la coordenada */
struct Coord {
    int x;
    int y;
};

/* Estructura para enviar datos de una tupla */
struct Tupla {
    int key;
    string value1<MAX_STR>;
    int N_value2;
    double V_value2<MAX_VEC>;
    Coord value3;
};

/* Estructura para recibir los datos en operaciones como get_value */
struct TuplaRet {
    int result;  /* 0 si OK, otro valor si error*/
    string value1<MAX_STR>;
    int N_value2;
    double V_value2<MAX_VEC>;
    Coord value3;
};

/* Definición del programa RPC */
program CLAVES_PROG {
    version CLAVES_VERS {
        int DESTROY(void) = 1;
        int SET_VALUE(Tupla) = 2;
        TuplaRet GET_VALUE(int) = 3;
        int DELETE_KEY(int) = 4;
        int MODIFY_VALUE(Tupla) = 5;
        int EXIST(int) = 6;
    } = 1;
} = 0x20000001;
