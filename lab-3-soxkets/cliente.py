import socket

def read_string(sock):
    """
    Lee una cadena del socket hasta encontrar el carácter nulo ('\0').
    """
    buffer = ""
    while True:
        char = sock.recv(1).decode()  # Recibe un byte y lo decodifica como carácter
        if char == '\0':  # Si encontramos el carácter nulo, terminamos
            break
        buffer += char
    return buffer

def main():
    # Configuración del servidor
    server_address = 'localhost'
    server_port = 2000

    # Crear un socket TCP/IP
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        # Conectar al servidor
        print(f"Conectando al servidor {server_address}:{server_port}...")
        sock.connect((server_address, server_port))
        print("Conexión establecida.")

        while True:
            # Leer un mensaje del usuario
            message = input("Introduce un mensaje (o 'EXIT' para salir): ")

            # Enviar el mensaje al servidor (agregamos el carácter nulo al final)
            sock.sendall((message + '\0').encode())

            # Si el usuario escribe "EXIT", salir del bucle
            if message == "EXIT":
                print("Saliendo...")
                break

            # Recibir la respuesta del servidor
            response = read_string(sock)
            print(f"Respuesta del servidor: {response}")

    finally:
        # Cerrar la conexión
        print("Cerrando la conexión...")
        sock.close()

if __name__ == "__main__":
    main()