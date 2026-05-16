#include "Match.h"

Match::Match(int matchId, const std::string& creatorPlayerName) : 
        matchId(matchId), 
        creatorPlayerName(creatorPlayerName),
        queueCMD(),
        isOnline(true),
        cantPlayers(0),
        cantMaxPlayers(10),
        gameLoop(queueCMD) {
            gameLoop.start();
        }
        
std::string Match::getCreatorPlayerName() const { return this->creatorPlayerName; }

int Match::getMatchId() const { return this->matchId; }

bool Match::addPlayer(int playerId, const std::string& playerName, Queue<Snapshot>& sender_queue) {
    if (isFull()) {
        return false;
    }
    cantPlayers++;
    return true;
}

bool Match::isFull() const { return cantPlayers >= cantMaxPlayers; }

bool Match::removePlayer(int playerId) {
    if (cantPlayers <= 0) {
        return false;
    }
    cantPlayers--;
    return true;
}

Queue<CommandDTO>& Match::getQueue() { return this->queueCMD; }

std::string Match::getMap() { return std::string(); }

Match::~Match() {
    isOnline = false;
    gameLoop.join();
}
