#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Uso: " << argv[0] << " <direccion_ip> <puerto>" << endl;
        return 1;
    }

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // Crear socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr << "Error en la creación del socket." << endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    // Convertir direcciones IPv4 de texto a binario
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        cerr << "Dirección IP inválida o no soportada." << endl;
        return -1;
    }

    // Conectar al servidor
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Conexión fallida." << endl;
        return -1;
    }

    cout << "Conectado al servidor.\n\n";

    // Loop de juego
while (true) {
    cout << "Ingrese el número de columna para jugar (0-6) o 'Q' para salir: ";
    string input;
    getline(cin, input);

    send(sock, input.c_str(), input.size(), 0);

    if (input == "Q" || input == "q") {
        cout << "Saliendo del juego." << endl;
        break;
    }

    memset(buffer, 0, sizeof(buffer)); // Limpia el buffer antes de recibir
    int valread = read(sock, buffer, 1024);
    if (valread > 0) {
        cout << "Respuesta del servidor: " << buffer << endl;
        if (strstr(buffer, "ha ganado")) {
            break;
        }
    } else {
        cerr << "Error de lectura o servidor desconectado." << endl;
        break;
    }
}

close(sock);

    return 0;
}