#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string>

using namespace std;

class Cliente {
private:
    int sock;
    struct sockaddr_in serv_addr;

public:
    Cliente(const char* ip, int puerto) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            cerr << "Error en la creación del socket." << endl;
            exit(1);
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(puerto);

        if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
            cerr << "Dirección IP inválida o no soportada." << endl;
            exit(1);
        }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            cerr << "Conexión fallida." << endl;
            exit(1);
        }
    }

    ~Cliente() {
        close(sock);
    }

    void jugar() {
        cout << "Conectado al servidor.\n\n";

        while (true) {
            cout << "Ingrese el número de columna para jugar (1-7) o 'Q' para salir: ";
            string input;
            getline(cin, input);

            if (input == "Q" || input == "q") {
                cout << "Saliendo del juego." << endl;
                break;
            }

            send(sock, input.c_str(), input.size(), 0);

            char buffer[1024] = {0};
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
    }
};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Uso: " << argv[0] << " <direccion_ip> <puerto>" << endl;
        return 1;
    }

    const char* ip = argv[1];
    int puerto = atoi(argv[2]);

    Cliente cliente(ip, puerto);
    cliente.jugar();

    return 0;
}
