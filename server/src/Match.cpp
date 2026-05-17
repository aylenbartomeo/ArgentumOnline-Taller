#include "Match.h"

#include <string>

Match::Match(int matchId, const std::string& creatorPlayerName):
        matchId(matchId),
        creatorPlayerName(creatorPlayerName),
        gameQueue(),
        monitor(),
        isOnline(true),
        gameLoop(gameQueue, monitor),
        cantMaxPlayers(0) {
    gameLoop.start();
}

std::string Match::getCreatorPlayerName() const { return this->creatorPlayerName; }

int Match::getMatchId() const { return this->matchId; }

bool Match::addPlayer(const std::string& playerName, Queue<SnapshotDTO>& sender_queue) {
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
