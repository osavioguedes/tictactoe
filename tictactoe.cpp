#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdlib>
#include <ctime>
#include <chrono>

using namespace std;

char board[3][3];
mutex board_mutex;
condition_variable turn_cv;
char jogador_atual = 'X';
bool jogo_terminado = false;
char vencedor = ' ';

void exibir_tabuleiro() {
    #ifdef _WIN32
        system("cls");
    #else
    #endif

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j)
            cout << (board[i][j] == ' ' ? '.' : board[i][j]) << ' ';
        cout << endl;
    }
    cout << endl;
}

bool checar_vitoria(char jogador) {
    for (int i = 0; i < 3; ++i)
        if ((board[i][0] == jogador && board[i][1] == jogador && board[i][2] == jogador) ||
            (board[0][i] == jogador && board[1][i] == jogador && board[2][i] == jogador))
            return true;

    return (board[0][0] == jogador && board[1][1] == jogador && board[2][2] == jogador) ||
           (board[0][2] == jogador && board[1][1] == jogador && board[2][0] == jogador);
}

bool checar_empate() {
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            if (board[i][j] == ' ')
                return false;
    return true;
}

bool fazer_jogada(char jogador, int linha, int coluna) {
    unique_lock<mutex> lock(board_mutex);
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Delay para simular tempo da jogada

    turn_cv.wait(lock, [&]() {
        return jogador_atual == jogador || jogo_terminado;
    });

    if (jogo_terminado || board[linha][coluna] != ' ' || jogador != jogador_atual)
        return false;

    system("clear");

    board[linha][coluna] = jogador;
    cout << "Jogador " << jogador << " jogou em [" << linha << "][" << coluna << "]\n";
    exibir_tabuleiro();

    if (checar_vitoria(jogador)) {
        vencedor = jogador;
        jogo_terminado = true;
    } else if (checar_empate()) {
        vencedor = 'D';
        jogo_terminado = true;
    } else {
        jogador_atual = (jogador == 'X') ? 'O' : 'X';
    }

    turn_cv.notify_all();
    return true;
}

void jogador_sequencial(char simbolo) {
    while (true) {
        {
            lock_guard<mutex> lock(board_mutex);
            if (jogo_terminado) break;
        }

        for (int i = 0; i < 3 && !jogo_terminado; ++i) {
            for (int j = 0; j < 3 && !jogo_terminado; ++j) {
                if (fazer_jogada(simbolo, i, j))
                    goto fim_turno;
            }
        }

    fim_turno:
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void jogador_aleatorio(char simbolo) {
    while (true) {
        {
            lock_guard<mutex> lock(board_mutex);
            if (jogo_terminado) break;
        }

        vector<pair<int, int>> vazios;
        {
            lock_guard<mutex> lock(board_mutex);
            for (int i = 0; i < 3; ++i)
                for (int j = 0; j < 3; ++j)
                    if (board[i][j] == ' ')
                        vazios.emplace_back(i, j);
        }

        if (!vazios.empty()) {
            auto escolha = vazios[rand() % vazios.size()];
            fazer_jogada(simbolo, escolha.first, escolha.second);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main() {
    srand(time(nullptr));

    // Inicializa o tabuleiro vazio
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            board[i][j] = ' ';

    // Cria as threads dos jogadores
    thread t1(jogador_sequencial, 'X');
    thread t2(jogador_aleatorio, 'O');

    t1.join();
    t2.join();

    system("clear");

    cout << "\n=== Resultado Final ===\n";
    exibir_tabuleiro();
    if (vencedor == 'D')
        cout << "Empate!\n";
    else
        cout << "Jogador " << vencedor << " venceu!\n";

    return 0;
}
