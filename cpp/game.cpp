#include "game.hpp"

Game::Game(Shogi game, Agent* agent1, Agent* agent2) : sente(agent1), gote(agent2) {
    s = game;
}

// Playout the game between the two players and determine a winner
int Game::play(int max_round) {
	while(s.round < max_round) {
    s.EasyBoardPrint();
    if (s.round % 2 == 0) {
        int move = sente->getMove();
        if (move == -1) {
            // No Moves for Sente, return gote winner
            return gote_win;
        }
        s.MakeMove(move);
    } else {
        int move = gote->getMove();
        if (move == -1) {
            // No moves for gote, return sente winner
            return sente_win;
        }
        s.MakeMove(move);
    }
  }
  return -1;
}


Shogi Game::getBoard() { return s; }
Agent& Game::getSenteAgent() { return *sente; }
Agent& Game::getGoteAgent() { return *gote; }

