#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include <vector>

using namespace std;

// Dimensiones del tablero
const int FILAS = 6;
const int COLUMNAS = 7;

// Función para inicializar el tablero
vector<vector<char>> inicializarTablero() {
    return vector<vector<char>>(FILAS, vector<char>(COLUMNAS, ' '));
}

// Función para imprimir el tablero
void imprimirTablero(const vector<vector<char>>& tablero) {
    for (int i = 0; i < FILAS; i++) {
        for (int j = 0; j < COLUMNAS; j++) {
            cout << (tablero[i][j] == ' ' ? '.' : tablero[i][j]) << " ";
        }
        cout << endl;
    }
    cout << endl;
}

// Función para verificar si una jugada es válida
bool esJugadaValida(int columna, const vector<vector<char>>& tablero) {
    return tablero[0][columna] == ' ';
}

// Función para aplicar la jugada
bool aplicarJugada(int columna, char jugador, vector<vector<char>>& tablero) {
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


// Thread para manejar cada conexión de cliente
void* jugar(void* arg) {
    int socket_cliente = *((int*)arg);
    struct sockaddr_in direccionCliente;
    socklen_t addr_size = sizeof(direccionCliente);
    getpeername(socket_cliente, (struct sockaddr*)&direccionCliente, &addr_size);
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(direccionCliente.sin_addr), ip, INET_ADDRSTRLEN);
    cout << "[" << ip << ":" << ntohs(direccionCliente.sin_port) << "] Nuevo jugador." << endl;

    vector<vector<char>> tablero = inicializarTablero();
    imprimirTablero(tablero);

    char buffer[1024];
    while (true) {
        memset(buffer, 0, 1024);
        int n_bytes = recv(socket_cliente, buffer, 1024, 0);
        if (n_bytes <= 0) break;

        int columna = atoi(buffer);
        if (!esJugadaValida(columna, tablero)) {
            const char* msg = "Movimiento no válido\n";
            send(socket_cliente, msg, strlen(msg), 0);
            continue;
        }

        aplicarJugada(columna, 'X', tablero);
        imprimirTablero(tablero);

        if (verificarGanador('X', tablero)) {
            const char* msg = "¡Ganaste!\n";
            send(socket_cliente, msg, strlen(msg), 0);
            break;
        }
    }

    close(socket_cliente);
    delete (int*)arg;
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        cerr << "Uso: " << argv[0] << " <puerto>" << endl;
        return 1;
    }

    int port = atoi(argv[1]);
    int socket_server;
    struct sockaddr_in direccionServidor, direccionCliente;

    socket_server = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_server < 0) {
        cerr << "Error al crear el socket." << endl;
        return 1;
    }

    memset(&direccionServidor, 0, sizeof(direccionServidor));
    direccionServidor.sin_family = AF_INET;
    direccionServidor.sin_addr.s_addr = htonl(INADDR_ANY);
    direccionServidor.sin_port = htons(port);

    if (bind(socket_server, (struct sockaddr *)&direccionServidor, sizeof(direccionServidor)) < 0) {
        cerr << "Error en bind()." << endl;
        return 1;
    }

    if (listen(socket_server, 10) < 0) {
        cerr << "Error en listen()." << endl;
        return 1;
    }

    cout << "Servidor esperando conexiones..." << endl;
    while (true) {
        int *socket_cliente = new int;
        socklen_t addr_size = sizeof(struct sockaddr_in);
        *socket_cliente = accept(socket_server, (struct sockaddr *)&direccionCliente, &addr_size);
        if (*socket_cliente < 0) {
            cerr << "Error en accept()." << endl;
            delete socket_cliente;
            continue;
        }

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, jugar, (void*)socket_cliente);
        pthread_detach(thread_id);
    }

    return 0;
}
