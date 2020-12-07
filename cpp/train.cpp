#include "train.hpp"

#define DEBUG 0

OrganismEvaluator::OrganismEvaluator() : heuristic(SENTE){
	// Load the cache of legal moves into memory
	cache.Init(lm_cache);

	// Set the default number of evaluations to size of train data
	n_eval = n_train;

	// Default mode is train mode
	mode = train_mode;
	sample = train_data;

	// Keep logging to stats object off by default
	log_stats = true;
}

void OrganismEvaluator::set_num_eval(int num_eval) {
	int bound = mode == train_mode ? n_train : n_test;
	if (num_eval <= 0) {
			throw invalid_argument("Positions to evaluate must be non-zero.");
	}
	if (num_eval > bound) {
		string error = "Not enough positions available in " + mode + " data.";
		throw invalid_argument(error);
	}
	n_eval = num_eval;
}

void OrganismEvaluator::set_mode(string mode_string) {
	if (mode_string != train_mode and mode_string != test_mode) {
			throw invalid_argument("Mode not supported");
	}
	mode = mode_string;
	sample = mode == train_mode ? train_data : test_data;
}

Shogi OrganismEvaluator::load_game(string board) {
	// Load the hex board representation and initialize the Shogi object
	Shogi s;
	s.Init();
	s.LoadGame(load_hex_vector(board));

	return s;
}

/* Function to return best move based on heuristic synchronously */
int OrganismEvaluator::select_move(string board, vector<int> weights, int& pos) {

	// Initialize shogi object based on board and best score / move to 0
	Shogi s = load_game(board);
	int best_score = INT_MIN, best_move = 0;

	// Set the perspective for the heuristic evaluation to current player for the input board
  int player = (s.round % 2);
	heuristic.setPlayer(player);

	// Loop though all possible moves reachable from board state
	int hits = 0;
	int p = 0;
	for (auto &action : cache.legal_moves[board]) {

		// Only look at one move in debug mode
		if (DEBUG and p) continue;

		int move = action.first;

		// Initialize new board and make the move
		Shogi result = s;
		result.MakeMove(move);

		// Print the board and the feature vector
		if (DEBUG and !p) {
			cout << "----------------------------------------" << endl;
			result.EasyBoardPrint();
			cout << endl;
			string curr = player == SENTE ? "Sente" : "Gote";
			cout << "Player: " << curr << endl;
		}

		// Key used for the transposition table of {pos, featureVector}
		vector<unsigned char> result_state = result.SaveGame();
		vector<int> fV;

		// Use the feature vector saved in the transposition table if game_state already seen
		if (feature_tt.count(result_state)) {
			fV = feature_tt.at(result_state);
		} else {
			// Update attack map needed in heuristic calculations
			result.FetchMove(1);

			// First time seeing game state, add {pos, featureVector} to transposition table
			fV = heuristic.feature_vec(result);
			feature_tt.insert({result_state, fV});
		}

		if (DEBUG and !p) {
			print_vec(fV);
			cout << "----------------------------------------" << endl;
			cout << endl;
			p = 1;
		}



    // Initialize score with pawn value and accumulate other features with weights
    int score = fV[0] * heuristic.getPawnValue();
    for (int i = 1; i < heuristic.num_features(); i++) {
        score += fV[i] * weights[i];
    }

		// Update highest score
		if (score > best_score) {
				best_score = score;
				best_move = move;
		}

		pos++;
	}

	return best_move;
}

void OrganismEvaluator::init_stats() {
	stats["eval_time_ms"] = 0;
	stats["total_positions"] = 0;
	stats["total_correct"] = 0;
	stats["upgrade_total"] = 0;
	stats["upgrade_correct"] = 0;
	stats["drop_total"] = 0;
	stats["drop_correct"] = 0;
}

int OrganismEvaluator::evaluate_synchronous(vector<int> weights, int& pos) {

	// Loop through all of the training games
	int correct = 0;
	int positions = 0;

	// Compiler directive to make segment parallel, specifies correct/positions as shared
	for (int i = 0; i < n_eval; i++) {
		auto game = sample[i];
		string board = game.first;
    int grandmaster_move = game.second;

		// Select a move using the given weights and set of shogi features
		int move = select_move(board, weights, positions);

		if (log_stats) {
			// Get some stats about the move
			stats["upgrade_total"] += UPGRADED == moveUpgrade(grandmaster_move) ? 1 : 0;
			stats["drop_total"] += PLAYING == movePlaying(grandmaster_move) ? 1 : 0;
		}

		// Compare selection with the choice of the grandmaster
		if (move == grandmaster_move) {
			if (log_stats) {
				stats["upgrade_correct"] += UPGRADED == moveUpgrade(move) ? 1 : 0;
				stats["drop_correct"] += PLAYING == movePlaying(move) ? 1 : 0;
			}
			correct++;
		}
	}

	pos += positions;


	return correct;
}

int OrganismEvaluator::evaluate_parallel(vector<int> weights, int&pos) {
	// Loop through all of the training games
	int correct = 0;
	int positions = 0;

	// Compiler directive to make segment parallel, specifies correct/positions as shared
	#pragma omp parallel for reduction(+:correct,positions)
	for (int i = 0; i < n_eval; i++) {
		auto game = sample[i];
		string board = game.first;
    int grandmaster_move = game.second;

		// Select a move using the given weights and set of shogi features
		int move = select_move(board, weights, positions);

		// Compare selection with the choice of the grandmaster
		if (move == grandmaster_move) {
			correct++;
		}
	}

	pos += positions;
	return correct;
}

int OrganismEvaluator::evaluate_organism(vector<int> weights)
{
	// Loop through all of the training games
	int correct = 0;
	int positions = 0;
	init_stats();

	// Uncomment for timing during featuresTests
	auto start = high_resolution_clock::now();

	if (!tt_full) {
		correct = evaluate_synchronous(weights, positions);
	} else {
		correct = evaluate_parallel(weights, positions);
	}

	if (log_stats) {
		// Add some stats about individual evaluation
		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(stop - start);
		stats["eval_time_ms"] = int(duration.count());
		stats["total_positions"] = positions;
		stats["total_correct"] = correct;
	}

	// After first full run mark the transposition table as full to be used later
	if (!tt_full) {
		tt_full = true;
	}

	// Overall fitness is the square of total number of correct moves
	return (correct * correct);
}


int main() {
    // Used if making featuresTests
    vector<int> weights(20, 1);

		OrganismEvaluator evaluator;
    for (int i = 0; i < 5; i++) {
				cout << "'Generation' " << i << endl;
        int correct1 = evaluator.evaluate_organism(weights);
        cout << "Guessed: " << correct1 << endl;
        /* cout << "Hits: " << hits << endl; */
        /* hits = 0; */
    }

		// Uncommment below for position / move generation testing
		/* cout << "--- Board and Move Tests ---" << endl; */
		/* /1* string replica = "FFFFFFFFFF13FFFFFFFFFFFFFF11FFFFFFFFFFFF0C151DFFFFFFFFFFFFFFFFFFFFFFFFFFFF10FF02FFFFFFFF1413FFFFFF00FF0207FFFF11171210FF0107FFFF16FF10FFFF00010213FFFFFF10FF00030601000000000000010A00000000000000007D"; *1/ */
		/* string board = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0007FFFFFFFFFFFFFF00FF07FFFFFFFFFF000501FFFFFFFFFFFFFF000602FFFFFFFFFF00FFFF03000000000000000000000000000000000000"; */
		/* Shogi r; */
		/* r.Init(); */
		/* r.LoadGame(load_hex_vector(board)); */

		/* r.EasyBoardPrint(); */

		/* vector<int> moveList = r.FetchMove(3); */
		/* sort(moveList.begin(), moveList.end()); */
		/* for (int move : moveList) { */
		/* 	cout << move << endl; */
		/* } */

		/* int chesser2 = r.round & 1; */
		/* cout << chesser2 << endl << endl; */
}

