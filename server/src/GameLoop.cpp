#include "GameLoop.h"
#include <chrono>

#define SKIN_CIUDADANO_INICIAL 1
#define SPEED 5.0f

GameLoop::GameLoop(Queue<CommandDTO>& queueCMD): 
        queueCMD(queueCMD), 
        map(),
        anyPlayers(false) {}

void GameLoop::addPlayerToMatch(const int playerId, std::string& playerName) {
    auto player = std::make_unique<Player>(playerId, playerName);
    // Asignar posición inicial en el mapa
    std::pair<float, float> initPos = map.getInitialPosition();
    player->setX(initPos.first);
    player->setY(initPos.second);

    // Configuración inicial por defecto (Skin/Ropa inicial del AO)
    player->setSkin(SKIN_CIUDADANO_INICIAL); 
    players.push_back(player.release());
    // Control de estado de la partida
    if (!anyPlayers) {
        anyPlayers = true; 
    }
}

void GameLoop::removePlayerFromMatch(int playerId) {}

bool GameLoop::PlayerColisionOnTheMap(float new_x, float new_y) {
    return this->map.playerColision(new_x, new_y);
}

void GameLoop::exec_movement(Player* player) {
    float diagonalSpeed = SPEED / std::sqrt(2.0f);
    float new_x = player->getX();
    float new_y = player->getY();
    switch (player->getMovement()) {
        case Movement::UP:
            new_y += SPEED;
            if (map.heightLimit() < new_y) 
                return; 
            break;
        case Movement::DOWN:
            new_y -= SPEED;
            if (new_y < 0) 
                return; 
            break;
        case Movement::LEFT:
            new_x -= SPEED;
            if (new_x < 0) 
                return; 
            break;
        case Movement::RIGHT:
            new_x += SPEED;
            if (map.widthLimit() < new_x) 
                return; 
            break;
        case Movement::DIAGONAL_UP_LEFT:
            new_x -= diagonalSpeed;
            new_y += diagonalSpeed;
            if (new_x < 0 || map.heightLimit() < new_y) 
                return; 
            break;
        case Movement::DIAGONAL_UP_RIGHT:
            new_x += diagonalSpeed;
            new_y += diagonalSpeed;
            if (map.widthLimit() < new_x || map.heightLimit() < new_y) 
                return; 
            break;
        case Movement::DIAGONAL_DOWN_LEFT:
            new_x -= diagonalSpeed;
            new_y -= diagonalSpeed;
            if (new_x < 0 || new_y < 0) 
                return; 
            break;
        case Movement::DIAGONAL_DOWN_RIGHT:
            new_x += diagonalSpeed;
            new_y -= diagonalSpeed;
            if (map.widthLimit() < new_x || new_y < 0) 
                return; 
            break;
        case Movement::STOP:
            break;
    }
    if (PlayerColisionOnTheMap(new_x, new_y)) {
        // Si hay colisión, no se actualizan las coordenadas
       return;
    }
    player->setX(new_x);
    player->setY(new_y);

}

Player* GameLoop::getPlayer(int playerId) {
     for (Player *p : players) {
        if(p->getId() == playerId)
            return p;
    }
    return nullptr;
}

void GameLoop::run() {    
    // 20 Ticks por segundo = 50 milisegundos por ciclo
    const std::chrono::milliseconds TICK_RATE(50);
    auto next_tick = std::chrono::steady_clock::now();

    while (should_keep_running()) {
        next_tick += TICK_RATE;

        // --- PROCESAR COMANDOS DE RED ---
        CommandDTO cmd;
        // Vaciamos todos los comandos que se acumularon en los últimos 50ms
        while (this->queueCMD.try_pop(cmd)) {
            // Acá asocias el comando al player correspondiente
            Player* player = this->getPlayer(cmd.id_jugador);
            player->setMovement(cmd.movement);
        }

        // --- ACTUALIZAR LÓGICA ---
        // Iterar sobre tus jugadores y aplicar la física/colisiones
        for (auto& player : this->players) {
            this->exec_movement(player); 
        }

        // --- BROADCAST SNAPSHOT ---
        // Aquí generarías el WorldSnapshot y lo meterías en las sendersQueues 
        // de los jugadores para que el cliente de tu compañera dibuje.

        // --- SUEÑO INTELIGENTE ---
        // Se duerme el tiempo justo que sobre. Si la lógica tardó 10ms, se duerme 40ms.
        std::this_thread::sleep_until(next_tick);
    }
}

