import socket
import threading

# Función que maneja la comunicación con un cliente
def handle_client(client_socket):
    while True:
        try:
            # Recibir datos del cliente
            data = client_socket.recv(1024).decode()  # Recibe hasta 1024 bytes y los decodifica
            if not data:
                break  # Si no hay datos, salir del bucle

            # Si el cliente envía "EXIT", cerrar la conexión
            if data.strip() == "EXIT":
                print("Cliente envió EXIT. Cerrando conexión...")
                break

            # Devolver el mismo mensaje al cliente
            client_socket.sendall(data.encode())
            print(f"Mensaje recibido: {data}")

        except Exception as e:
            print(f"Error: {e}")
            break

    # Cerrar la conexión con el cliente
    client_socket.close()

def main():
    # Configuración del servidor
    host = "0.0.0.0"  # Escuchar en todas las interfaces
    port = 2000       # Puerto en el que el servidor escuchará

    # Crear un socket TCP/IP
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        # Enlazar el socket a la dirección y puerto
        server_socket.bind((host, port))
        print(f"Servidor escuchando en {host}:{port}...")

        # Escuchar conexiones entrantes (máximo 5 clientes en espera)
        server_socket.listen(5)

        while True:
            # Aceptar una conexión de un cliente
            client_socket, client_address = server_socket.accept()
            print(f"Conexión aceptada desde {client_address}")

            # Crear un hilo para manejar al cliente
            client_thread = threading.Thread(target=handle_client, args=(client_socket,))
            client_thread.start()

    except Exception as e:
        print(f"Error en el servidor: {e}")
    finally:
        # Cerrar el socket del servidor
        server_socket.close()
        print("Servidor cerrado.")

if __name__ == "__main__":
    main()