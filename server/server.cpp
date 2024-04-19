#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include <vector>
#include <sstream>

using namespace std;

const int FILAS = 6;
const int COLUMNAS = 7;

vector<vector<char>> inicializarTablero() {
    return vector<vector<char>>(FILAS, vector<char>(COLUMNAS, ' '));
}

string tableroToString(const vector<vector<char>>& tablero) {
    stringstream ss;
    for (const auto& fila : tablero) {
        for (char c : fila) {
            ss << (c == ' ' ? '.' : c) << " ";
        }
        ss << '\n';
    }
    return ss.str();
}

bool esJugadaValida(int columna, const vector<vector<char>>& tablero) {
    return columna >= 0 && columna < COLUMNAS && tablero[0][columna] == ' ';
}

bool aplicarJugada(int columna, char jugador, vector<vector<char>>& tablero) {
    if (!esJugadaValida(columna, tablero)) return false;
    for (int i = FILAS - 1; i >= 0; i--) {
        if (tablero[i][columna] == ' ') {
            tablero[i][columna] = jugador;
            return true;
        }
    }
    return false;
}

bool verificarGanador(char jugador, const vector<vector<char>>& tablero) {
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


bool jugarServidor(vector<vector<char>>& tablero, const char* ip, int puerto, stringstream& ss) {
    for (int columna = 0; columna < COLUMNAS; columna++) {
        if (aplicarJugada(columna, 'S', tablero)) {
            // Mensaje en el servidor
            cout << "Juego [" << ip << ":" << puerto << "]: servidor juega columna " << columna << "." << endl;
            // Construye el mensaje que será enviado al cliente, incluyendo el estado del tablero
            ss << "Juego [" << ip << ":" << puerto << "]: servidor juega columna " << columna << ".\n";
            ss << tableroToString(tablero);
            return true;
        }
    }
    return false;
}


void* jugar(void* arg) {
    int socket_cliente = *((int*)arg);
    struct sockaddr_in direccionCliente;
    socklen_t addr_size = sizeof(direccionCliente);
    getpeername(socket_cliente, (struct sockaddr*)&direccionCliente, &addr_size);
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(direccionCliente.sin_addr), ip, INET_ADDRSTRLEN);

    cout << "Juego nuevo[" << ip << ":" << ntohs(direccionCliente.sin_port) << "]" << endl;

    vector<vector<char>> tablero = inicializarTablero();
    char buffer[1024];

    while (true) {
        memset(buffer, 0, 1024);
        int n_bytes = recv(socket_cliente, buffer, 1024, 0);
        if (n_bytes <= 0) break;

        int columna = atoi(buffer);
        stringstream ss;
        if (!aplicarJugada(columna, 'C', tablero)) {
            ss << "Movimiento no válido\n";
        } else {
            ss << "Juego [" << ip << ":" << ntohs(direccionCliente.sin_port) << "]: cliente juega columna " << columna << ".\n";
            ss << tableroToString(tablero);
            cout << "Juego [" << ip << ":" << ntohs(direccionCliente.sin_port) << "]: cliente juega columna " << columna << ".\n";

            if (verificarGanador('C', tablero)) {
                ss << "¡El cliente ha ganado!\n";
                cout << "Juego [" << ip << ":" << ntohs(direccionCliente.sin_port) << "]: ¡El cliente ha ganado!\n";
            } else {
                if (jugarServidor(tablero, ip, ntohs(direccionCliente.sin_port), ss)) {
                    if (verificarGanador('S', tablero)) {
                        ss << "¡El servidor ha ganado!\n";
                        cout << "Juego [" << ip << ":" << ntohs(direccionCliente.sin_port) << "]: ¡El servidor ha ganado!\n";
                    }
                }
            }
        }
        // Enviar el mensaje acumulado, que incluye la jugada del servidor y el estado del tablero
        send(socket_cliente, ss.str().c_str(), ss.str().length(), 0);
    }

    close(socket_cliente);
    delete (int*)arg;
    return NULL;
}


int main(int argc, char **argv) {
    if (argc != 2) {
        cerr << "Uso: " << argv[0] << " <puerto>\n";
        return 1;
    }

    int puerto = atoi(argv[1]);
    int socket_servidor;
    struct sockaddr_in direccionServidor;

    socket_servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_servidor < 0) {
        cerr << "Error al crear el socket.\n";
        return 1;
    }

    memset(&direccionServidor, 0, sizeof(direccionServidor));
    direccionServidor.sin_family = AF_INET;
    direccionServidor.sin_addr.s_addr = htonl(INADDR_ANY);
    direccionServidor.sin_port = htons(puerto);

    if (bind(socket_servidor, (struct sockaddr *)&direccionServidor, sizeof(direccionServidor)) < 0) {
        cerr << "Error en bind().\n";
        return 1;
    }

    if (listen(socket_servidor, 10) < 0) {
        cerr << "Error en listen().\n";
        return 1;
    }

    cout << "Servidor esperando conexiones...\n";

    while (true) {
        int* socket_cliente = new int;
        *socket_cliente = accept(socket_servidor, (struct sockaddr*)NULL, NULL);
        if (*socket_cliente < 0) {
            cerr << "Error en accept().\n";
            delete socket_cliente;
            continue;
        }

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, jugar, (void*)socket_cliente);
        pthread_detach(thread_id);
    }

    return 0;
}