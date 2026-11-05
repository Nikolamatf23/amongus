#include "network/GameController.h"

GameController::~GameController() {}

void GameController::onMessage(int, const Message&) {}
void GameController::onPlayerLeft(int) {}
void GameController::tick() {}
void GameController::sendPositions() {}
void GameController::endMeeting() {}