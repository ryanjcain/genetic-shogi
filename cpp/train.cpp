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
int OrganismEvaluator::select_synchronous(string board, int* weights) {

	// Initialize shogi object based on board and best score / move to 0
	Shogi s = load_game(board);
	int best_score = 0, best_move = 0;

	// ***** MIGHT NOT NEED ***** But initialize root player
  int player = (s.round & 1);

	// Loop though synchrounously
	for (auto &action : cache.legal_moves[board]) {
		int move = action.first;

		// Initialize new board and make the move
		Shogi result = s;
		result.MakeMove(move);

		// Update highest score and save feature vector (without weight) in transposition table
		int score = heuristic.evaluate(result, weights, player, feature_tt);
		if (score > best_score) {
				best_score = score;
				best_move = move;
		}
	}

	return best_move;
}

/* Function to return best move based on heuristic in parallel */
int OrganismEvaluator::select_parallel(string board, int *weights) {

	// Initialize shogi obj and struct to hold best values during parallel execution
	Shogi s = load_game(board);
	struct Action best_action;
	best_action.score = 0;
	best_action.move = 0;

	// ***** MIGHT NOT NEED ***** But initialize root player
  int player = (s.round & 1);

	// Compiler directive specifies this loop is run in parallel with user struct
	#pragma omp parallel for reduction(max:best_action)
	for (auto &action : cache.legal_moves[board]) {
		int move = action.first;

		// Initialize new board and make the move
		Shogi result = s;
		result.MakeMove(move);

		// Use cached feature vecor instead of calculating
		vector<int> fV = feature_tt.at(result.SaveGame());

		// Calculate heuristic score based on weights
		int score = 0;
		for (int i = 0; i < heuristic.num_features(); i++) {
			score += fV[i] * weights[i];
		}

		/* int score = heuristic.evaluate(result, weights, player, feature_tt); */
		if (score > best_action.score) {
				best_action.score = score;
				best_action.move = move;
		}
	}

	return best_action.move;
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

    for (auto& game : TRAIN) {
    /* for (int i = 0; i < 100; i++) { */
        /* auto game = TRAIN[i]; */
        string board = game.first;
        int grandmaster_move = game.second;
				int selected_move = 0;
        
				// First time running through training positions so cache feature vectors
				if (!evaluator.feature_cache_loaded()) {
					selected_move = evaluator.select_synchronous(board, weights);
				} else {
					selected_move = evaluator.select_parallel(board, weights);
				}

				// Compare selection with the choice of the grandmaster
				if (selected_move == grandmaster_move) {
					correct++;
				}
    }

    // Uncomment for timing during featuresTests
    auto stop = high_resolution_clock::now(); 
    auto duration = duration_cast<milliseconds>(stop - start); 
    /* cout << "Evaluated H() for " << TRAIN.size() << " games, took " << duration.count() << "ms" << endl; */
    /* cout << "Evaluated H() for 100 games, took " << duration.count() << "ms" << endl; */
    
    // Overall fitness is the square of total number of correct moves
    return (correct * correct);
    /* return correct; */
}


int main() {
    // Used if making featuresTests
    int weights[20] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

    for (int i = 0; i < 5; i++) {
        int correct1 = evaluate_organism(weights);
        cout << "Guessed: " << correct1 << endl;
        /* cout << "Hits: " << hits << endl; */
        /* hits = 0; */
    }
}

