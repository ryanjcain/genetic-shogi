#include "gshogi-agent.hpp"

GShogiAgent::GShogiAgent(bool color, unsigned int d, vector<int> h_weights)
    : heuristic(color, h_weights)
{
	setColor(color);
	setDepth(d);
}

int GShogiAgent::getMove() {
	return negamaxHelper(INT_MIN, INT_MAX);
}

// Order moves based on relative impoortance of of piece
vector<int> GShogiAgent::orderMoves(vector<int> moves) {
    vector<int> ordered;
    ordered.reserve(moves.size());
    Shogi s = getBoard();

    for (int piece : search_order) {
        // Consider moving a regular piece first
        if (piece != -1) {
            for (int move : moves) {
                // Skip drop move for now
                if (movePlaying(move)) continue;
                int pos = movePrepos(move);
                if (gomakindEID(s.gomaKind[s.board[pos]]) == piece) {
                    ordered.push_back(move);
                }
            }
        } else {
            // Add drop moves last
            for (int move : moves) {
                if (movePlaying(move)) {
                    ordered.push_back(move);
                }
            }
        }
    }

    return ordered;
}

// Helper function performs first negama call and prints stats
int GShogiAgent::negamaxHelper(int alpha, int beta) {
  vector<int> ordered_moves = orderMoves(getBoard().FetchMove(3));

	int best_move_val = INT_MIN;

  // Vector of best moves and their heuristic value
	vector<pair<int, int>> best_moves;

	// for each possible move
	for (int move : ordered_moves) {

    // Skip a move if it has been played withing buffer_size past moves. Avoid infinite games
		if (find(played_buffer.begin(), played_buffer.end(), move) != played_buffer.end()) continue;

    Shogi next = getBoard();
    next.MakeMove(move);

		// find value of that move
		int value = -negamax(next, getDepth() - 1, -beta, -alpha, !getColor());

		if (value == best_move_val || ordered_moves.size() <= buffer_size) {
			best_moves.push_back({move, value});
		} else if (value > best_move_val) {
			best_move_val = value;

      // Get rid of worse moves
			best_moves.clear();
			best_moves.push_back({move, value});
		}

		if (best_move_val > alpha) {
        alpha = best_move_val;
    }

		// Prune and keep track of pruned nodes
		if (alpha >= beta) {
        prune_count++;
        break;
    }
	}

  // Edge casing in case there are no mobes
	if (best_moves.size() == 0) {
		cout << (getColor() ? "SENTE" : "GOTE");
		cout << " forfeit due to having no moves.\n\n";
    return -1;
	}

  // Heuristic evaluates some moves to have same value, so pick one at random
	pair<int, int> best = best_moves[rand() % best_moves.size()];
	played_buffer.push_back(best.first); // add best move to buffer

	// Maintain size of move buffer
	if (played_buffer.size() > buffer_size) {
		played_buffer.erase(played_buffer.begin());
	}

  if (log_stats) {
      printStats(best_move_val, best_moves.size()); // show some data
  }

	return best.first;
}

int GShogiAgent::negamax(Shogi& s, unsigned int depth, int alpha, int beta, bool player) {
	node_count += 1;

	int offset = (player == getColor() ? 1 : -1);

	// If we reach the determined depth, return heuristic score
	if (depth == 0) {
      return offset * heuristic_value(s);
  }

	int best_value = INT_MIN;
  vector<int> orderd_moves = orderMoves(s.FetchMove(3));

	for (int move : orderd_moves) {
    Shogi next = getBoard();
    next.MakeMove(move);

		// Recursive call
		int value = -negamax(next, depth - 1, -beta, -alpha, !player);

		value = max(best_value, value);
		alpha = max(alpha, value);

		if (alpha >= beta) {
        prune_count++;
        break;
    }
	}

	return beta;
}

// Use the evolved heuristic
int GShogiAgent::heuristic_value(Shogi& s) {
    return heuristic.evaluate(s);
}

// Output some stats as we go on, including how many times
// the heuristic evaluated moves to the same score
inline void GShogiAgent::printStats(int score, int equal) {
    cout << "Nodes evaluated: " << node_count << endl;
    cout << "Nodes pruned: " << prune_count << endl;

    if (equal) {
        cout << equal << " moves had same heuristic value, chose randomly" << endl;
    }

    cout << (getColor() ? "GOTE" : "SENTE");
    cout << " chose a move with score " << score << endl;

    node_count = 0; prune_count = 0;
}

unsigned int GShogiAgent::getDepth() { return depth; }
void GShogiAgent::setDepth(unsigned int d) { depth = d; }
