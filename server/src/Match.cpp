#include "Match.h"

Match::Match(int matchId, const std::string& creatorPlayerName) : 
        matchId(matchId), 
        creatorPlayerName(creatorPlayerName),
        queueCMD(),
        isOnline(true),
        gameLoop() {
            gameLoop.start();
        }

std::string Match::getCreatorPlayerName() const { return this->creatorPlayerName; }

int Match::getMatchId() const { return this->matchId; }

bool Match::addPlayer(const std::string& playerName, Queue<Snapshot>& sender_queue) {
    if (isFull()) {
        return false;
    }
    cantPlayers++;
    return true;
}

bool Match::isFull() const { return cantPlayers >= cantMaxPlayers; }

bool Match::removePlayer(const std::string& playerName) {
    if (cantPlayers <= 0) {
        return false;
    }
    cantPlayers--;
    return true;
}

std::string Match::getMap() { return std::string(); }

Match::~Match() {
    isOnline = false;
    gameLoop.join();
}
