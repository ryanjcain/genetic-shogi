#include "agent.hpp"


class Game {
	public:
		Game(Shogi s, Agent* agent1, Agent* agent2);

    // Play a game between two agents and return a winner
		int play(int max_round);

	private:
		Shogi s;
		Agent* sente;
    Agent* gote;
    const int sente_win = 0;
    const int gote_win = 1;

		unsigned int gameState;
		unsigned int moves = 0;

		void senteMove();
		void goteMove();

		Shogi getBoard();

		Agent& getSenteAgent();
		Agent& getGoteAgent();
};
