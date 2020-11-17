#include "train.hpp"

/* Struct required to do the reduction in parallel */
struct Action { float score; int move; };
#pragma omp declare reduction(max : struct Action : omp_out = omp_in.score > omp_out.score ? omp_in : omp_out)

OrganismEvaluator::OrganismEvaluator(string moves_cache_file) {
	// Load the cache of legal moves into memory
	cache.Init(moves_cache_file);
}


Shogi OrganismEvaluator::load_game(string board) {
	// Load the hex board representation and initialize the Shogi object
	Shogi s;
	s.Init();
	s.LoadGame(load_hex_vector(board));

	return s;
}

/* Function to return best move based on heuristic synchronously */
int OrganismEvaluator::select_move(string board, int* weights, int& pos) {

	// Initialize shogi object based on board and best score / move to 0
	Shogi s = load_game(board);
	int best_score = 0, best_move = 0;

	// ***** MIGHT NOT NEED ***** But initialize root player
  int player = (s.round & 1);

	// Loop though all possible moves reachable from board state
	int hits = 0;
	for (auto &action : cache.legal_moves[board]) {
		int move = action.first;

		// Initialize new board and make the move
		Shogi result = s;
		result.MakeMove(move);

		// Key used for the transposition table of {pos, featureVector}
		vector<unsigned char> result_state = result.SaveGame();
		vector<int> fV;

		// Use the feature vector saved in the transposition table if game_state already seen
		if (feature_tt.count(result_state)) {
			fV = feature_tt.at(result_state);
		} else {
			// First time seeing game state, add {pos, featureVector} to transposition table
			fV = heuristic.feature_vec(result, player);
			feature_tt.insert({result_state, fV});
		}

		// Calculate score based on given weights
		int score = 0;
		for (int i = 0; i < heuristic.num_features(); i++) {
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

int OrganismEvaluator::evaluate_synchronous(int* weights, int& pos) {

	// Loop through all of the training games
	int correct = 0;
	int positions = 0;

	// Compiler directive to make segment parallel, specifies correct/positions as shared
	for (auto& game : TRAIN) {
	/* for (int i = 0; i < 2000; i++) { */
		/* auto game = TRAIN[i]; */
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

int OrganismEvaluator::evaluate_parallel(int* weights, int&pos) {

	// Loop through all of the training games
	int correct = 0;
	int positions = 0;

	// Compiler directive to make segment parallel, specifies correct/positions as shared
	#pragma omp parallel for reduction(+:correct,positions)
	for (auto& game : TRAIN) {
	/* for (int i = 0; i < 2000; i++) { */
		/* auto game = TRAIN[i]; */
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

// Globals used in every evaluation (should make const)
static OrganismEvaluator evaluator(LM_CACHE);

// Force export of C symbols to avoid C++ name cluttering (Used for python ctypes)
#ifdef __cplusplus
extern "C" int evaluate_organism(int* weights)
#else
char int evaluate_organism(int* weights)
#endif
{
	// Loop through all of the training games
	int correct = 0;
	int positions = 0;

	// Uncomment for timing during featuresTests
	auto start = high_resolution_clock::now();

	if (!evaluator.feature_cache_loaded()) {
		correct = evaluator.evaluate_synchronous(weights, positions);
	} else {
		correct = evaluator.evaluate_parallel(weights, positions);
	}

	// Uncomment for timing during featuresTests
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(stop - start);
	cout << "Evaluated H() for " << TRAIN.size() << " games, took " << duration.count() << "ms" << endl;

	// Remember to mark the cache as being full after first execution
	cout << "Evaluated for " << positions << " positions" << endl;

	if (!evaluator.feature_cache_loaded()) {
		evaluator.update_tt_status(true);
	}

	positions = 0;

	// Overall fitness is the square of total number of correct moves
	return (correct * correct);
	/* return correct; */
}


int main() {
    // Used if making featuresTests
    int weights[20] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

    for (int i = 0; i < 5; i++) {
				cout << "'Generation' " << i << endl;
        int correct1 = evaluate_organism(weights);
        cout << "Guessed: " << correct1 << endl;
        /* cout << "Hits: " << hits << endl; */
        /* hits = 0; */
    }

		// Uncommment below for position / move generation testing
		/* cout << "--- Board and Move Tests ---" << endl; */
		/* /1* string replica = "FFFFFFFFFF13FFFFFFFFFFFFFF11FFFFFFFFFFFF0C151DFFFFFFFFFFFFFFFFFFFFFFFFFFFF10FF02FFFFFFFF1413FFFFFF00FF0207FFFF11171210FF0107FFFF16FF10FFFF00010213FFFFFF10FF00030601000000000000010A00000000000000007D"; *1/ */
		/* string replica = "13FFFF10FF00FF04031214FFFF10FF00FF02FFFF1510FF00FFFFFFFFFFFFFF10FF00FFFFFFFF0111FF151810FFFFFFFFFF10FFFFFF07FFFF1200FFFFFF10FFFF1610FFFFFF0001020BFFFFFFFFFFFFFF06020000010000000101010000000000020063"; */
		/* Shogi r; */
		/* r.Init(); */
		/* r.LoadGame(load_hex_vector(replica)); */

		/* r.EasyBoardPrint(); */

		/* vector<int> moveList = r.FetchMove(3); */
		/* sort(moveList.begin(), moveList.end()); */
		/* for (int move : moveList) { */
		/* 	cout << move << endl; */
		/* } */

		/* int chesser2 = r.round & 1; */
		/* cout << chesser2 << endl << endl; */
}

