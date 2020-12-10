#include "agent.hpp"

bool Agent::getColor() { return color; }
Shogi Agent::getBoard() { return s; }
void Agent::setColor(bool c) {	color = c; }
void Agent::setBoard(Shogi b) { s = b; }
