#include <iostream>
#include <vector>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

using namespace std;

class Juego {
private:
    vector<vector<char>> tablero;
    const int FILAS = 6;
    const int COLUMNAS = 7;

public:
    Juego() {
        tablero = vector<vector<char>>(FILAS, vector<char>(COLUMNAS, ' '));
    }

    string tableroToString() {
        stringstream ss;
        int numeroFila = 1;
        for (const auto& fila : tablero) {
            ss << numeroFila << " ";
            for (char c : fila) {
                ss << (c == ' ' ? '.' : c) << " ";
            }
            ss << '\n';
            ++numeroFila;
        }

        ss << "  ";
        for (int i = 1; i <= COLUMNAS; ++i) {
            ss << i << " ";
        }
        ss << '\n';

        return ss.str();
    }

    bool esJugadaValida(int columna) {
        int columnaIndex = columna - 1;
        return columnaIndex >= 0 && columnaIndex < COLUMNAS && tablero[0][columnaIndex] == ' ';
    }

    bool aplicarJugada(int columna, char jugador) {
        int columnaIndex = columna - 1;
        if (!esJugadaValida(columna)) return false;
        for (int i = FILAS - 1; i >= 0; i--) {
            if (tablero[i][columnaIndex] == ' ') {
                tablero[i][columnaIndex] = jugador;
                return true;
            }
        }
        return false;
    }

    bool verificarGanador(char jugador) {
        // Verificación horizontal
        for (int i = 0; i < FILAS; i++) {
            for (int j = 0; j < COLUMNAS - 3; j++) {
                if (tablero[i][j] == jugador && tablero[i][j + 1] == jugador &&
                    tablero[i][j + 2] == jugador && tablero[i][j + 3] == jugador) {
                    return true;
                }
            }
        }

        // Verificación vertical
        for (int j = 0; j < COLUMNAS; j++) {
            for (int i = 0; i < FILAS - 3; i++) {
                if (tablero[i][j] == jugador && tablero[i + 1][j] == jugador &&
                    tablero[i + 2][j] == jugador && tablero[i + 3][j] == jugador) {
                    return true;
                }
            }
        }

        // Verificación diagonal descendente (de izquierda a derecha)
        for (int i = 0; i < FILAS - 3; i++) {
            for (int j = 0; j < COLUMNAS - 3; j++) {
                if (tablero[i][j] == jugador && tablero[i + 1][j + 1] == jugador &&
                    tablero[i + 2][j + 2] == jugador && tablero[i + 3][j + 3] == jugador) {
                    return true;
                }
            }
        }

        // Verificación diagonal ascendente (de derecha a izquierda)
        for (int i = 3; i < FILAS; i++) {
            for (int j = 0; j < COLUMNAS - 3; j++) {
                if (tablero[i][j] == jugador && tablero[i - 1][j + 1] == jugador &&
                    tablero[i - 2][j + 2] == jugador && tablero[i - 3][j + 3] == jugador) {
                    return true;
                }
            }
        }

        return false; // Si no se encontraron 4 en línea, retornar falso.
    }

    bool jugarServidor(int& columna_jugada) {
        for (int columna = 1; columna <= COLUMNAS; columna++) {
            if (aplicarJugada(columna, 'S')) {
                columna_jugada = columna;
                return true;
            }
        }
        return false;
    }
};

class Server {
private:
    int socket_servidor;
    struct sockaddr_in direccionServidor;

public:
    Server(int puerto) {
        socket_servidor = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_servidor < 0) {
            cerr << "Error al crear el socket.\n";
            exit(1);
        }

        memset(&direccionServidor, 0, sizeof(direccionServidor));
        direccionServidor.sin_family = AF_INET;
        direccionServidor.sin_addr.s_addr = htonl(INADDR_ANY);
        direccionServidor.sin_port = htons(puerto);

        if (bind(socket_servidor, (struct sockaddr *)&direccionServidor, sizeof(direccionServidor)) < 0) {
            cerr << "Error en bind().\n";
            exit(1);
        }

        if (listen(socket_servidor, 10) < 0) {
            cerr << "Error en listen().\n";
            exit(1);
        }
    }

    ~Server() {
        close(socket_servidor);
    }

    void aceptarClientes() {
        cout << "Servidor esperando conexiones...\n";
        while (true) {
            int* socket_cliente = new int;
            *socket_cliente = accept(socket_servidor, nullptr, nullptr);
            if (*socket_cliente < 0) {
                cerr << "Error en accept().\n";
                delete socket_cliente;
                continue;
            }

            struct sockaddr_in addr;
            socklen_t addr_len = sizeof(addr);
            getpeername(*socket_cliente, (struct sockaddr*)&addr, &addr_len);
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
            int port = ntohs(addr.sin_port);

            cout << "Juego nuevo[" << ip << ":" << port << "]" << endl;

            pthread_t thread_id;
            pthread_create(&thread_id, nullptr, &Server::jugar, socket_cliente);
            pthread_detach(thread_id);
        }
    }

    static void* jugar(void* arg) {
        int socket_cliente = *((int*)arg);
        Juego juego;
        char buffer[1024];
        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);
        getpeername(socket_cliente, (struct sockaddr*)&addr, &addr_len);

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
        int port = ntohs(addr.sin_port);

        while (true) {
            memset(buffer, 0, 1024);
            int n_bytes = recv(socket_cliente, buffer, 1024, 0);
            if (n_bytes <= 0) break;

            int columna = atoi(buffer);
            stringstream ss;
            if (!juego.aplicarJugada(columna, 'C')) {
                ss << "Movimiento no válido\n";
            } else {
                ss << "Juego [" << ip << ":" << port << "]: cliente juega columna " << columna << ".\n";
                ss << juego.tableroToString();
                cout << "Juego [" << ip << ":" << port << "]: cliente juega columna " << columna << ".\n";

                if (juego.verificarGanador('C')) {
                    ss << "¡El cliente ha ganado!\n";
                    cout << "Juego [" << ip << ":" << port << "]: ¡El cliente ha ganado!\n";
                    send(socket_cliente, ss.str().c_str(), ss.str().length(), 0);
                    break;
                }

                int columna_jugada;
                if (juego.jugarServidor(columna_jugada)) {
                    ss << "Juego [" << ip << ":" << port << "]: servidor juega columna " << columna_jugada << ".\n";
                    ss << juego.tableroToString();
                    cout << "Juego [" << ip << ":" << port << "]: servidor juega columna " << columna_jugada << ".\n";

                    if (juego.verificarGanador('S')) {
                        ss << "¡El servidor ha ganado!\n";
                        cout << "Juego [" << ip << ":" << port << "]: ¡El servidor ha ganado!\n";
                        send(socket_cliente, ss.str().c_str(), ss.str().length(), 0);
                        break;
                    }
                }
            }
            send(socket_cliente, ss.str().c_str(), ss.str().length(), 0);
        }

        close(socket_cliente);
        delete (int*)arg;
        return nullptr;
    }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        cerr << "Uso: " << argv[0] << " <puerto>\n";
        return 1;
    }

    int puerto = atoi(argv[1]);
    Server servidor(puerto);
    servidor.aceptarClientes();

    return 0;
}