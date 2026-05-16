#include "GameMonitor.h"

GameMonitor::GameMonitor() :
        matches(){}

int GameMonitor::createMatch(int playerId, std::string& creatorPlayerName,
                             Queue<Snapshot>& senderQueue) {
    std::lock_guard<std::mutex> lock(mtx);
    matches[GameId] = std::make_unique<Match>(GameId, creatorPlayerName);
    matches[GameId]->addPlayer(playerId, creatorPlayerName, senderQueue);
    int currentId = GameId;
    GameId++;
    return currentId;
}

bool GameMonitor::joinMatch(int playerId, int matchId, std::string& playerName,
                            Queue<Snapshot>& senderQueue) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = matches.find(matchId);
    if (it != matches.end()) {
        return it->second->addPlayer(playerId, playerName, senderQueue);    
    }
    return false;
}

void GameMonitor::removePlayerFromMatch(int playerId, int matchId) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = matches.find(matchId);
    if (it != matches.end()) {
        if (it->second->removePlayer(playerId)) {
            matches.erase(it);
        }
    }
}

std::vector<std::string> GameMonitor::listActiveMatches() {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<std::string> activeMatches;
    for (const auto& pair : matches) {
        if (!pair.second->isFull()) {
            std::string newMatch = "Id: " + std::to_string(pair.first) + "  ||  Creada por: " + pair.second->getCreatorPlayerName() + "\n";
            activeMatches.push_back(newMatch);
        }
    }
    return activeMatches;   
}

void GameMonitor::deleteAllMatches() {
    std::lock_guard<std::mutex> lock(mtx);
    matches.clear();
}

std::string GameMonitor::getMapFromId(int matchId) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = matches.find(matchId);
    if (it != matches.end()) {
        return it->second->getMap();    
    }
    return "";
}

Queue<CommandDTO>* GameMonitor::getQueueFromMatch(int matchId) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = matches.find(matchId);
    if (it != matches.end()) {
        return &it->second->getQueue();
    }
    return nullptr;
}
