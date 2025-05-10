#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include "log.h"
#include <rpc/rpc.h>


#define MAX_USERS 100
#define MAX_FILES 1000
#define BUFFER_SIZE 256
#define MAX_CONTENT_PER_USER 100
#define MAX_CONNECTED_USERS 50

// Estructuras para almacenar usuarios y archivos
typedef struct {
    char name[BUFFER_SIZE];
    char ip[BUFFER_SIZE];
    int port;
    int is_connected;
} User;

typedef struct {
    char user[BUFFER_SIZE];
    char file[BUFFER_SIZE];
    char description[BUFFER_SIZE];
} File;

User users[MAX_USERS];
File files[MAX_FILES];
int user_count = 0;
int file_count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Para evitar condiciones de carrera


ssize_t readLine(int fd, void *buffer, size_t n)
{
	ssize_t numRead;  /* num of bytes fetched by last read() */
	size_t totRead;	  /* total bytes read so far */
	char *buf;
	char ch;


	if (n <= 0 || buffer == NULL) { 
		errno = EINVAL;
		return -1; 
	}

	buf = buffer;
	totRead = 0;
	
	for (;;) {
        	numRead = read(fd, &ch, 1);	/* read a byte */

        	if (numRead == -1) {	
            		if (errno == EINTR)	/* interrupted -> restart read() */
                		continue;
            	else
			return -1;		/* some other error */
        	} else if (numRead == 0) {	/* EOF */
            		if (totRead == 0)	/* no byres read; return 0 */
                		return 0;
			else
                		break;
        	} else {			/* numRead must be 1 if we get here*/
            		if (ch == '\n')
                		break;
            		if (ch == '\0')
                		break;
            		if (totRead < n - 1) {		/* discard > (n-1) bytes */
				totRead++;
				*buf++ = ch; 
			}
		} 
	}
	
	*buf = '\0';
    	return totRead;
}

int sendMessage(int socket, char * buffer, int len)
{
	int r;
	int l = len;
		

	do {	
		r = write(socket, buffer, l);
		l = l -r;
		buffer = buffer + r;
	} while ((l>0) && (r>=0));
	
	if (r < 0)
		return (-1);   /* fail */
	else
		return(0);	/* full length has been sent */
}

// Función para enviar un log al servidor RPC
void enviar_log_rpc(const char *usuario, const char *operacion, const char *timestamp) {
    char *rpc_ip = getenv("LOG_RPC_IP");
    if (!rpc_ip) return;

    CLIENT *clnt = clnt_create(rpc_ip, LOGPROG, LOGVERS, "udp");
    if (!clnt) {
        fprintf(stderr, "No se pudo conectar al servidor RPC en %s\n", rpc_ip);
        return;
    }

    LogEntry entrada;
    entrada.usuario = (char *)usuario;
    entrada.operacion = (char *)operacion;
    entrada.timestamp = (char *)timestamp;

    log_event_1(&entrada, clnt);
    clnt_destroy(clnt);
}


// Función para buscar un usuario por nombre
int find_user(const char *name) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].name, name) == 0) {
            return i;
        }
    }
    return -1; // No encontrado
}

// Función para buscar un archivo por usuario y nombre de archivo
int find_file(const char *user, const char *file) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].user, user) == 0 && strcmp(files[i].file, file) == 0) {
            return i;
        }
    }
    return -1; // No encontrado
}


void handle_register(int client_socket, char *user_name, const char *timestamp) {
    pthread_mutex_lock(&mutex);
    int rc;
    if (find_user(user_name) != -1) {
        rc = 1; // Usuario ya existe
    } else if (user_count >= MAX_USERS) {
        rc = 2; // Error genérico
    } else {
        strcpy(users[user_count].name, user_name);
        users[user_count].is_connected = 0;
        user_count++;
        rc = 0; // Éxito
    }
    pthread_mutex_unlock(&mutex);
    send(client_socket, &rc, 1, 0); // Envía 1 byte de respuesta
    if (rc == 0) {
        enviar_log_rpc(user_name, "REGISTER", timestamp);
    }
}

void handle_unregister(int client_socket, char *user_name, const char *timestamp) {
    pthread_mutex_lock(&mutex);
    int rc;
    int user_idx = find_user(user_name);
    
    if (user_idx == -1) {
        rc = 1; // Usuario no existe
    } else {
        // Eliminar todos sus archivos primero
        for (int i = 0; i < file_count; ) {
            if (strcmp(files[i].user, user_name) == 0) {
                memmove(&files[i], &files[i+1], (file_count - i - 1) * sizeof(File));
                file_count--;
            } else {
                i++;
            }
        }
        // Eliminar usuario
        memmove(&users[user_idx], &users[user_idx+1], (user_count - user_idx - 1) * sizeof(User));
        user_count--;
        rc = 0; // Éxito
    }
    pthread_mutex_unlock(&mutex);
    send(client_socket, &rc, 1, 0);
    if (rc == 0) {
        enviar_log_rpc(user_name, "UNREGISTER", timestamp);
    }
}

void handle_connect(int client_socket, char *user_name, char *port_str, const char *timestamp) {
    pthread_mutex_lock(&mutex);
    int rc;
    int user_idx = find_user(user_name);
    int port = atoi(port_str);
    
    if (user_idx == -1) {
        rc = 1; // Usuario no existe
    } else if (users[user_idx].is_connected) {
        rc = 2; // Ya conectado
    } else {
        // Obtener IP del cliente (simplificado)
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        getpeername(client_socket, (struct sockaddr*)&addr, &len);
        char *ip = inet_ntoa(addr.sin_addr);
        
        strcpy(users[user_idx].ip, ip);
        users[user_idx].port = port;
        users[user_idx].is_connected = 1;
        rc = 0; // Éxito
    }
    pthread_mutex_unlock(&mutex);
    send(client_socket, &rc, 1, 0);
    if (rc == 0) {
        enviar_log_rpc(user_name, "CONNECT", timestamp);
    }
}


void handle_disconnect(int client_socket, char *user_name, const char *timestamp) {
    pthread_mutex_lock(&mutex);
    int rc;
    int user_idx = find_user(user_name);
    
    if (user_idx == -1) {
        rc = 1; // Usuario no existe
    } else if (!users[user_idx].is_connected) {
        rc = 2; // No conectado
    } else {
        users[user_idx].is_connected = 0;
        rc = 0; // Éxito
    }
    pthread_mutex_unlock(&mutex);
    send(client_socket, &rc, 1, 0);
    if (rc == 0) {
        enviar_log_rpc(user_name, "DISCONNECT", timestamp);
    }
}


void handle_publish(int client_socket, char *user_name, char *file_name, char *description, const char *timestamp) {
    pthread_mutex_lock(&mutex);
    int rc;
    int user_idx = find_user(user_name);
    if (user_idx == -1) {
        rc = 1; // Usuario no existe
    } else if (!users[user_idx].is_connected) {
        rc = 2; // Usuario no conectado
    } else if (find_file(user_name, file_name) != -1) {
        rc = 3; // Archivo ya publicado
    } else if (file_count >= MAX_FILES) {
        rc = 4; // Error genérico
    } else {
        strcpy(files[file_count].user, user_name);
        strcpy(files[file_count].file, file_name);
        strcpy(files[file_count].description, description);
        file_count++;
        rc = 0; // Éxito
    }
    pthread_mutex_unlock(&mutex);
    send(client_socket, &rc, 1, 0);
    if (rc == 0) {
        char full_msg[BUFFER_SIZE * 2];
        snprintf(full_msg, sizeof(full_msg), "PUBLISH %s", file_name);
        enviar_log_rpc(user_name, full_msg, timestamp);
    }
}


void handle_delete(int client_socket, char *user_name, char *file_name, const char *timestamp) {
    pthread_mutex_lock(&mutex);
    int rc;
    int user_idx = find_user(user_name);
    
    if (user_idx == -1) {
        rc = 1; // Usuario no existe
    } else if (!users[user_idx].is_connected) {
        rc = 2; // No conectado
    } else {
        int file_idx = find_file(user_name, file_name);
        if (file_idx == -1) {
            rc = 3; // Archivo no publicado
        } else {
            // Eliminar archivo
            memmove(&files[file_idx], &files[file_idx+1], (file_count - file_idx - 1) * sizeof(File));
            file_count--;
            rc = 0; // Éxito
        }
    }
    pthread_mutex_unlock(&mutex);
    send(client_socket, &rc, 1, 0);
    if (rc == 0) {
        char full_msg[BUFFER_SIZE * 2];
        snprintf(full_msg, sizeof(full_msg), "DELETE %s", file_name);
        enviar_log_rpc(user_name, full_msg, timestamp);
    }
}


void handle_list_users(int client_socket, char *user_name, const char *timestamp) {
    pthread_mutex_lock(&mutex);
    int rc;
    int user_idx = find_user(user_name);
    
    if (user_idx == -1) {
        rc = 1; // Usuario no existe
        send(client_socket, &rc, 1, 0);
        pthread_mutex_unlock(&mutex);
    } else if (!users[user_idx].is_connected) {
        rc = 2; // No conectado
        send(client_socket, &rc, 1, 0);
        pthread_mutex_unlock(&mutex);
    } else {
        rc = 0; // Éxito
        send(client_socket, &rc, 1, 0);
        
        // Contar usuarios conectados
        int connected_count = 0;
        for (int i = 0; i < user_count; i++) {
            if (users[i].is_connected) connected_count++;
        }
        
        // Enviar conteo
        char count_str[BUFFER_SIZE];
        snprintf(count_str, BUFFER_SIZE, "%d", connected_count);
        sendMessage(client_socket, count_str, strlen(count_str)+1);
        
        // Enviar lista de usuarios
        for (int i = 0; i < user_count; i++) {
            if (users[i].is_connected) {
                sendMessage(client_socket, users[i].name, strlen(users[i].name)+1);
                sendMessage(client_socket, users[i].ip, strlen(users[i].ip)+1);
                
                char port_str[BUFFER_SIZE];
                snprintf(port_str, BUFFER_SIZE, "%d", users[i].port);
                sendMessage(client_socket, port_str, strlen(port_str)+1);
            }
        }
    }
    pthread_mutex_unlock(&mutex);
    if (rc == 0) {
        enviar_log_rpc(user_name, "LIST USERS", timestamp);
    }
}

void handle_listcontent(int client_socket, char *requesting_user, char *target_user, const char *timestamp) {
    pthread_mutex_lock(&mutex);
    int rc;
    
    // Verificar usuario que hace la solicitud
    int req_user_idx = find_user(requesting_user);
    if (req_user_idx == -1) {
        rc = 1; // Usuario no existe
        send(client_socket, &rc, 1, 0);
        pthread_mutex_unlock(&mutex);
    } 
    else if (!users[req_user_idx].is_connected) {
        rc = 2; // Usuario no conectado
        send(client_socket, &rc, 1, 0);
        pthread_mutex_unlock(&mutex);
    } 
    // Verificar usuario objetivo
    else if (find_user(target_user) == -1) {
        rc = 3; // Usuario remoto no existe
        send(client_socket, &rc, 1, 0);
        pthread_mutex_unlock(&mutex);
    } 
    else {
        rc = 0; // Éxito
        send(client_socket, &rc, 1, 0);
        
        // Contar archivos del usuario objetivo
        int file_count_user = 0;
        for (int i = 0; i < file_count; i++) {
            if (strcmp(files[i].user, target_user) == 0) {
                file_count_user++;
            }
        }
        
        // Enviar conteo
        char count_str[BUFFER_SIZE];
        snprintf(count_str, BUFFER_SIZE, "%d", file_count_user);
        sendMessage(client_socket, count_str, strlen(count_str)+1);
        
        // Enviar lista de archivos (solo nombres)
        for (int i = 0; i < file_count; i++) {
            if (strcmp(files[i].user, target_user) == 0) {
                sendMessage(client_socket, files[i].file, strlen(files[i].file)+1);
            }
        }
    }
    pthread_mutex_unlock(&mutex);
    if (rc == 0) {
        enviar_log_rpc(requesting_user, "LIST CONTENT", timestamp);
    }
}


void *handle_request(void *arg) {
    int client_socket = *((int *)arg);
    char buffer[BUFFER_SIZE];
    char command[BUFFER_SIZE];
    char timestamp[BUFFER_SIZE];
    char user_name[BUFFER_SIZE];
    char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];

    // Leer comando principal
    if (readLine(client_socket, command, BUFFER_SIZE) <= 0) {
        close(client_socket);
        return NULL;
    }

    if (readLine(client_socket, timestamp, BUFFER_SIZE) <= 0) {
        close(client_socket);
        return NULL;
    }
    

    if (strcmp(command, "REGISTER") == 0) {
        readLine(client_socket, user_name, BUFFER_SIZE);
        handle_register(client_socket, user_name, timestamp);
    }
    else if (strcmp(command, "UNREGISTER") == 0) {
        readLine(client_socket, user_name, BUFFER_SIZE);
        handle_unregister(client_socket, user_name, timestamp);
    }
    else if (strcmp(command, "CONNECT") == 0) {
        readLine(client_socket, user_name, BUFFER_SIZE);
        readLine(client_socket, arg1, BUFFER_SIZE); // port
        handle_connect(client_socket, user_name, arg1, timestamp);
    }
    else if (strcmp(command, "PUBLISH") == 0) {
        readLine(client_socket, user_name, BUFFER_SIZE);
        readLine(client_socket, arg1, BUFFER_SIZE); // file_name
        readLine(client_socket, arg2, BUFFER_SIZE); // description
        handle_publish(client_socket, user_name, arg1, arg2, timestamp);
    }
    else if (strcmp(command, "DELETE") == 0) {
        readLine(client_socket, user_name, BUFFER_SIZE);
        readLine(client_socket, arg1, BUFFER_SIZE); // file_name
        handle_delete(client_socket, user_name, arg1, timestamp);
    }
    else if (strcmp(command, "LIST_USERS") == 0) {
        readLine(client_socket, user_name, BUFFER_SIZE);
        handle_list_users(client_socket, user_name, timestamp);
    }
    else if (strcmp(command, "DISCONNECT") == 0) {
        readLine(client_socket, user_name, BUFFER_SIZE);
        handle_disconnect(client_socket, user_name, timestamp);
    }
    else if (strcmp(command, "LIST_CONTENT") == 0) {
        readLine(client_socket, user_name, BUFFER_SIZE);
        readLine(client_socket, arg1, BUFFER_SIZE); // target_user
        handle_listcontent(client_socket, user_name, arg1, timestamp);
    }
    else {
        // Comando no reconocido
        char error_msg[] = "ERROR: Unknown command";
        send(client_socket, error_msg, strlen(error_msg)+1, 0);
    }
    close(client_socket);
    return NULL;
}
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Crear socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    // Enlazar y escuchar
    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket, 5);

    printf("Server listening on port %s\n", argv[1]);

    // Bucle principal
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        pthread_t thread;
        pthread_create(&thread, NULL, handle_request, &client_socket);
        pthread_detach(thread); // No esperar a que el hilo termine
    }

    close(server_socket);
    return 0;
}

