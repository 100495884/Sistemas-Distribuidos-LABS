import socket
import argparse
from enum import Enum
import threading

class client:
    class RC(Enum):
        OK = 0
        ERROR = 1
        USER_ERROR = 2

    _server = None
    _port = -1
    _current_user = None  # Usuario actualmente conectado
    _listen_port = None   # Puerto para escuchar conexiones P2P
    _listen_thread = None # Hilo para escuchar conexiones entrantes

    @staticmethod
    def _send_command_to_server(command, *args):
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                # Enviar comando y argumentos según el protocolo
                message = command + '\0' + '\0'.join(args) + '\0'
                s.sendall(message.encode())
                # Leer respuesta del servidor
                response = s.recv(1)
                return int.from_bytes(response, byteorder='little')
        except Exception as e:
            print(f"Error al comunicar con el servidor: {e}")
            return client.RC.ERROR.value



    @staticmethod
    def register(user):
        rc = client._send_command_to_server("REGISTER", user)
        if rc == 0:
            print("REGISTER OK")
            return client.RC.OK
        elif rc == 1:
            print("USERNAME IN USE")
            return client.RC.USER_ERROR
        else:
            print("REGISTER FAIL")
            return client.RC.ERROR


   
    @staticmethod
    def unregister(user):
        rc = client._send_command_to_server("UNREGISTER", user)
        if rc == 0:
            print("UNREGISTER OK")
            return client.RC.OK
        elif rc == 1:
            print("USER DOES NOT EXIST")
            return client.RC.USER_ERROR
        else:
            print("UNREGISTER FAIL")
            return client.RC.ERROR


    

    @staticmethod
    def _listen_for_peers(port):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind(('0.0.0.0', port))
            s.listen()
            while True:
                conn, addr = s.accept()
                # Aquí manejamos la solicitud de descarga (GET_FILE)
                conn.close()

    @staticmethod
    def connect(user):
        # Buscar puerto libre
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind(('0.0.0.0', 0))
        port = s.getsockname()[1]
        s.close()

        # Iniciar hilo para escuchar conexiones P2P
        client._listen_thread = threading.Thread(target=client._listen_for_peers, args=(port,))
        client._listen_thread.start()

        # Enviar comando CONNECT al servidor
        rc = client._send_command_to_server("CONNECT", user, str(port))
        if rc == 0:
            print("CONNECT OK")
            client._current_user = user
            client._listen_port = port
            return client.RC.OK
        elif rc == 1:
            print("CONNECT FAIL, USER DOES NOT EXIST")
            return client.RC.USER_ERROR
        elif rc == 2:
            print("USER ALREADY CONNECTED")
            return client.RC.USER_ERROR
        else:
            print("CONNECT FAIL")
            return client.RC.ERROR





    @staticmethod
    def disconnect(user):
        rc = client._send_command_to_server("DISCONNECT", user)
        if rc == 0:
            print("DISCONNECT OK")
            client._current_user = None
            # Detener el hilo de escucha (implementación simplificada)
            return client.RC.OK
        elif rc == 1:
            print("DISCONNECT FAIL, USER DOES NOT EXIST")
            return client.RC.USER_ERROR
        elif rc == 2:
            print("DISCONNECT FAIL, USER NOT CONNECTED")
            return client.RC.USER_ERROR
        else:
            print("DISCONNECT FAIL")
            return client.RC.ERROR



    @staticmethod
    def publish(fileName, description):
        rc = client._send_command_to_server("PUBLISH", client._current_user, fileName, description)
        if rc == 0:
            print("PUBLISH OK")
            return client.RC.OK
        elif rc == 1:
            print("PUBLISH FAIL, USER DOES NOT EXIST")
            return client.RC.USER_ERROR
        elif rc == 2:
            print("PUBLISH FAIL, USER NOT CONNECTED")
            return client.RC.USER_ERROR
        elif rc == 3:
            print("PUBLISH FAIL, CONTENT ALREADY PUBLISHED")
            return client.RC.USER_ERROR
        else:
            print("PUBLISH FAIL")
            return client.RC.ERROR

    @staticmethod
    def delete(fileName):
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                # Conectar al servidor
                s.connect((client._server, client._port))
                
                # Enviar comando DELETE, usuario actual y nombre del archivo
                message = f"DELETE\0{client._current_user}\0{fileName}\0"
                s.sendall(message.encode())
                
                # Recibir respuesta del servidor (1 byte)
                rc = int.from_bytes(s.recv(1), byteorder='little')
                
                # Manejar respuesta
                if rc == 0:
                    print("DELETE OK")
                    return client.RC.OK
                elif rc == 1:
                    print("DELETE FAIL, USER DOES NOT EXIST")
                    return client.RC.USER_ERROR
                elif rc == 2:
                    print("DELETE FAIL, USER NOT CONNECTED")
                    return client.RC.USER_ERROR
                elif rc == 3:
                    print("DELETE FAIL, CONTENT NOT PUBLISHED")
                    return client.RC.USER_ERROR
                else:
                    print("DELETE FAIL")
                    return client.RC.ERROR
                    
        except Exception as e:
            print(f"DELETE FAIL: {e}")
            return client.RC.ERROR



    @staticmethod
    def listusers():
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                # Enviar comando LIST_USERS y nombre de usuario
                message = f"LIST_USERS\0{client._current_user}\0"
                s.sendall(message.encode())

                # Recibir respuesta del servidor (1 byte: código de retorno)
                rc = int.from_bytes(s.recv(1), byteorder='little')

                if rc == 0:
                    # Recibir número de usuarios
                    num_users = int(s.recv(256).decode().strip('\0'))
                    print(f"LIST_USERS OK ({num_users} users connected)")

                    # Recibir lista de usuarios (nombre, IP, puerto)
                    for _ in range(num_users):
                        user_name = s.recv(256).decode().strip('\0')
                        user_ip = s.recv(256).decode().strip('\0')
                        user_port = s.recv(256).decode().strip('\0')
                        print(f"- {user_name} ({user_ip}:{user_port})")

                    return client.RC.OK
                elif rc == 1:
                    print("LIST_USERS FAIL, USER DOES NOT EXIST")
                    return client.RC.USER_ERROR
                elif rc == 2:
                    print("LIST_USERS FAIL, USER NOT CONNECTED")
                    return client.RC.USER_ERROR
                else:
                    print("LIST_USERS FAIL")
                    return client.RC.ERROR
        except Exception as e:
            print(f"LIST_USERS FAIL: {e}")
            return client.RC.ERROR



    @staticmethod
    def listcontent(user):
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                # Enviar comando LIST_CONTENT, usuario actual y usuario objetivo
                message = f"LIST_CONTENT\0{client._current_user}\0{user}\0"
                s.sendall(message.encode())

                # Recibir respuesta del servidor (1 byte: código de retorno)
                rc = int.from_bytes(s.recv(1), byteorder='little')

                if rc == 0:
                    # Recibir número de archivos
                    num_files = int(s.recv(256).decode().strip('\0'))
                    print(f"LIST_CONTENT OK ({num_files} files)")

                    # Recibir lista de archivos (nombre)
                    for _ in range(num_files):
                        file_name = s.recv(256).decode().strip('\0')
                        print(f"- {file_name}")

                    return client.RC.OK
                elif rc == 1:
                    print("LIST_CONTENT FAIL, USER DOES NOT EXIST")
                    return client.RC.USER_ERROR
                elif rc == 2:
                    print("LIST_CONTENT FAIL, USER NOT CONNECTED")
                    return client.RC.USER_ERROR
                elif rc == 3:
                    print("LIST_CONTENT FAIL, REMOTE USER DOES NOT EXIST")
                    return client.RC.USER_ERROR
                else:
                    print("LIST_CONTENT FAIL")
                    return client.RC.ERROR
        except Exception as e:
            print(f"LIST_CONTENT FAIL: {e}")
            return client.RC.ERROR



    @staticmethod
    def getfile(user, remoteFileName, localFileName):
        # Obtener IP y puerto del usuario remoto (requiere LIST_USERS implementado)
        # Conectar al cliente remoto y solicitar el archivo
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((remote_ip, remote_port))
                s.sendall(f"GET_FILE\0{remoteFileName}\0".encode())
                response = s.recv(1)
                if response == b'\x00':
                    # Recibir el archivo y guardarlo localmente
                    with open(localFileName, 'wb') as f:
                        while True:
                            data = s.recv(1024)
                            if not data:
                                break
                            f.write(data)
                    print("GET_FILE OK")
                    return client.RC.OK
                else:
                    print("GET_FILE FAIL")
                    return client.RC.ERROR
        except Exception as e:
            print(f"GET_FILE FAIL: {e}")
            return client.RC.ERROR



    # *

    # **

    # * @brief Command interpreter for the client. It calls the protocol functions.

    @staticmethod

    def shell():



        while (True) :

            try :

                command = input("c> ")

                line = command.split(" ")

                if (len(line) > 0):



                    line[0] = line[0].upper()



                    if (line[0]=="REGISTER") :

                        if (len(line) == 2) :

                            client.register(line[1])

                        else :

                            print("Syntax error. Usage: REGISTER <userName>")



                    elif(line[0]=="UNREGISTER") :

                        if (len(line) == 2) :

                            client.unregister(line[1])

                        else :

                            print("Syntax error. Usage: UNREGISTER <userName>")



                    elif(line[0]=="CONNECT") :

                        if (len(line) == 2) :

                            client.connect(line[1])

                        else :

                            print("Syntax error. Usage: CONNECT <userName>")

                    

                    elif(line[0]=="PUBLISH") :

                        if (len(line) >= 3) :

                            #  Remove first two words

                            description = ' '.join(line[2:])

                            client.publish(line[1], description)

                        else :

                            print("Syntax error. Usage: PUBLISH <fileName> <description>")



                    elif(line[0]=="DELETE") :

                        if (len(line) == 2) :

                            client.delete(line[1])

                        else :

                            print("Syntax error. Usage: DELETE <fileName>")



                    elif(line[0]=="LIST_USERS") :

                        if (len(line) == 1) :

                            client.listusers()

                        else :

                            print("Syntax error. Use: LIST_USERS")



                    elif(line[0]=="LIST_CONTENT") :

                        if (len(line) == 2) :

                            client.listcontent(line[1])

                        else :

                            print("Syntax error. Usage: LIST_CONTENT <userName>")



                    elif(line[0]=="DISCONNECT") :

                        if (len(line) == 2) :

                            client.disconnect(line[1])

                        else :

                            print("Syntax error. Usage: DISCONNECT <userName>")



                    elif(line[0]=="GET_FILE") :

                        if (len(line) == 4) :

                            client.getfile(line[1], line[2], line[3])

                        else :

                            print("Syntax error. Usage: GET_FILE <userName> <remote_fileName> <local_fileName>")



                    elif(line[0]=="QUIT") :

                        if (len(line) == 1) :

                            break

                        else :

                            print("Syntax error. Use: QUIT")

                    else :

                        print("Error: command " + line[0] + " not valid.")

            except Exception as e:

                print("Exception: " + str(e))



    # *

    # * @brief Prints program usage

    @staticmethod

    def usage() :

        print("Usage: python3 client.py -s <server> -p <port>")





    # *

    # * @brief Parses program execution arguments

    @staticmethod

    def  parseArguments(argv) :

        parser = argparse.ArgumentParser()

        parser.add_argument('-s', type=str, required=True, help='Server IP')

        parser.add_argument('-p', type=int, required=True, help='Server Port')

        args = parser.parse_args()



        if (args.s is None):

            parser.error("Usage: python3 client.py -s <server> -p <port>")

            return False



        if ((args.p < 1024) or (args.p > 65535)):

            parser.error("Error: Port must be in the range 1024 <= port <= 65535");

            return False;

        

        client._server = args.s

        client._port = args.p



        return True





    # ******************** MAIN *********************

    @staticmethod

    def main(argv) :

        if (not client.parseArguments(argv)) :

            client.usage()

            return



        #  Write code here

        client.shell()

        print("+++ FINISHED +++")

    



if __name__=="__main__":

    client.main([])