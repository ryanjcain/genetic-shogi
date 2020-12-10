#include "agent.hpp"
#include "features.hpp"
#include <limits.h>
#include <algorithm>

class GShogiAgent : public Agent {
	public:
		GShogiAgent(bool, unsigned int, vector<int> h_weights);
		int getMove();

	private:

    ShogiFeatures heuristic;
    bool log_stats = true;
		unsigned int depth;
		unsigned int node_count = 0;
		unsigned int prune_count = 0;

    // List of moves that have been played recently to avoid repeats in search
		vector<int> played_buffer;
    int buffer_size = 4;

    // Relative order of importance of pieces when deciding moves to search through
    vector<int> search_order = {KING, PRO_BISHOP, PRO_ROOK, ROOK, BISHOP, PRO_PAWN, PRO_SILVER,
                                PRO_KNIGHT, PRO_LANCE, PAWN, SILVER, KNIGHT, GOLD, LANCE, -1};

    vector<int> orderMoves(vector<int> moves);
		int negamaxHelper(int, int);
		int negamax(Shogi& s, unsigned int, int, int, bool);
		int heuristic_value(Shogi& s);
		void printStats(int, int);

		unsigned int getDepth();
		void setDepth(unsigned int);
};
