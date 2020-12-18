#include "train.hpp"
#include "json.hpp"
#include <string>

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
	log = true;
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
	if (find(modes.begin(), modes.end(), mode_string) == modes.end()) {
			throw invalid_argument("Mode not supported");
	}
	// Careful with tt cache when switching between modes
	feature_tt.clear();
	tt_full = false;
	sample = mode == train_mode ? train_data : test_data;
	mode = mode_string;
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

	if (DEBUG) {
		cout << "----------------------------------------" << endl;
		string curr = player == SENTE ? "Sente" : "Gote";
		cout << "Player: " << curr << endl;
		cout << "Starting position:" << endl;
		s.EasyBoardPrint();
	}

	// Loop though all possible moves reachable from board state
	int hits = 0;
	for (auto &action : cache.legal_moves[board]) {

		int move = action.first;

		// Initialize new board and make the move
		Shogi result = s;
		result.MakeMove(move);

		// Print the board in debug mode
		/* if (DEBUG) { */
		/* 	cout << "----------------------------------------" << endl; */
		/* 	string curr = player == SENTE ? "Sente" : "Gote"; */
		/* 	cout << "Player: " << curr << endl; */
		/* 	cout << "Starting position:" << endl; */
		/* 	s.EasyBoardPrint(); */
		/* 	cout << "Looking at first available move: "; */
		/* 	printMove(move); */
		/* 	cout << "Result:" << endl; */
		/* 	result.EasyBoardPrint(); */
		/* 	cout << endl; */
		/* } */

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
			fV = heuristic.feature_vec_raw(result);
			feature_tt.insert({result_state, fV});
		}

    // Initialize score with pawn value and accumulate other features with weights
		int score = heuristic.evaluate_feature_vec(fV, weights);

		// Print the raw feature vector in debug mode
		/* if (DEBUG) { */
		/* 	print_vec(fV); */
		/* 	cout << "Score: " << score << endl; */
		/* 	cout << endl; */

		/* 	// Break from the loop after looking at one move */
		/* 	/1* break; *1/ */
		/* } */



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
	stats["total_positions_evaluated"] = 0;
	stats["total_correct"] = 0;
	stats["total_positions"] = 0;

	stats["gm_drop_total"] = 0;
	stats["gm_up_total"] = 0;
	stats["h_drop_total"] = 0;
	stats["h_missed_drops"] = 0;
	stats["h_missed_ups"] = 0;
	stats["h_up_total"] = 0;

	// Stats for upgrades
	stats["h_up_same_square_same_piece"] = 0;
	stats["h_up_same_square_diff_piece"] = 0;
	stats["h_up_diff_square_diff_piece"] = 0;
	stats["h_up_diff_square_same_piece"] = 0;

	// Stats for drops
	stats["h_drop_same_square_same_piece"] = 0;
	stats["h_drop_same_square_diff_piece"] = 0;
	stats["h_drop_diff_square_diff_piece"] = 0;
	stats["h_drop_diff_square_same_piece"] = 0;
	
	// Stats for regular moves
	stats["h_drop_when_gm_normal"] = 0;
	stats["h_up_when_gm_normal"] = 0;
	stats["h_move_same_square_same_piece"] = 0;
	stats["h_move_same_square_diff_piece"] = 0;
	stats["h_move_diff_square_diff_piece"] = 0;
	stats["h_move_diff_square_same_piece"] = 0;
	stats["h_drop_same_square_as_gm_normal"] = 0;
	stats["h_up_when_gm_normal_same_square_same_piece"] = 0;
	stats["h_up_when_gm_normal_same_square_diff_piece"] = 0;
	stats["h_up_when_gm_normal_diff_square_same_piece"] = 0;
	stats["h_up_when_gm_normal_diff_square_diff_piece"] = 0;
}

void OrganismEvaluator::log_stats(string board, int move, int grandmaster_move, vector<int> weights) {
	
	// Initialize board in order to check for piece correctness etc
	Shogi s = load_game(board);
	
	// Resulting board of grand master move move
	Shogi gm = s;
	gm.MakeMove(grandmaster_move);

	// Resulting board of heuristic move
	Shogi h = s;
	h.MakeMove(move);

	int h_new_pos = moveNewpos(move);
	int gm_new_pos = moveNewpos(grandmaster_move);
	int h_pre_pos = movePrepos(move);
	int gm_pre_pos = movePrepos(grandmaster_move);
	int h_goma_kind = h.gomaKind[h.board[h_new_pos]];
	int gm_goma_kind = gm.gomaKind[gm.board[gm_new_pos]];

	// Type of piece that was moved, ie. pawn, promoted bishop, etc
	int h_piece_type = gomakindEID(h_goma_kind);
	int gm_piece_type = gomakindEID(gm_goma_kind);


	if (log) {
			/* /1* // Print out the board and the drop move if in debug mode *1/ */
			if (DEBUG and mode == train_drops) {
				vector<int> fv = heuristic.feature_vec_raw(gm);
				int gm_score = heuristic.evaluate_feature_vec(fv, weights);

				vector<int> fvH = heuristic.feature_vec_raw(h);
				int h_score = heuristic.evaluate_feature_vec(fvH, weights);

  			int player = (s.round % 2);
				string turn = player == SENTE ? "Sente" : "Gote";

				cout << "Grandmaster Drop Move for " << turn << endl;
				cout << "     ";
				printMove(grandmaster_move);
				cout << "     H(p) = " << gm_score << endl;
				cout << "     ";
				cout << "Heuristic chose:  ";
				printMove(move);
				cout << "     H(p) = " << h_score << endl;
			}

			// See if grandmaster played a drop move
			if (PLAYING == movePlaying(grandmaster_move)) {
				stats["gm_drop_total"] += 1;

				if (PLAYING == movePlaying(move)) {
					stats["h_drop_total"] += 1;

					// Heuristic guessed same square and same piece
					if (move == grandmaster_move) {
						stats["h_drop_same_square_same_piece"] += 1;
					} 

					// Heuristic also played a drop move on the same square, but different piece
					else if (h_new_pos == gm_new_pos) {
						stats["h_drop_same_square_diff_piece"] += 1;
					}

					// Heuristic played a drop of same piece on different square
					else if (h_piece_type == gm_piece_type) {
						stats["h_drop_diff_square_same_piece"] += 1;
					}

					// Heuristic played a drop of different piece on different square
					else {
						stats["h_drop_diff_square_diff_piece"] += 1;
					}
				} 

				// Otherwise heuristic didnt drop when it should have
				else {
					stats["h_missed_drops"] += 1;
				}
			}

			// See if grandmaster played a upgrade move
			else if (UPGRADED == moveUpgrade(grandmaster_move)) {
				stats["gm_up_total"] += 1;

				if (UPGRADED == moveUpgrade(move)) {
					stats["h_up_total"] += 1;

					if (move == grandmaster_move) {
						stats["h_up_same_square_same_piece"] += 1;
					}

					// Heuristic moved and upgraded the same piece, but ended on diff square
					else if (h_pre_pos == gm_pre_pos) {
						stats["h_up_diff_square_same_piece"] += 1;
					}
					
					else if (h_new_pos == gm_new_pos) {
						stats["h_up_same_square_diff_piece"] += 1;
					}

					else {
						stats["h_up_diff_square_diff_piece"] += 1;
					}
				}

				// Otherwise heuristic missed an upgrade when it should have
				else {
					stats["h_missed_ups"] += 1;
				}
			} 

			// Otherwise it is just a regular move for gm
			else {
				if (move == grandmaster_move) {
					stats["h_move_same_square_same_piece"] += 1;
				}
				// Heuristic dropped when grandmaster played regular move
				else if (PLAYING == movePlaying(move)) {
					stats["h_drop_when_gm_normal"] += 1;

					if (h_new_pos == gm_new_pos) {
						stats["h_drop_same_square_as_gm_normal"] += 1;
					}
				}
				// Heuristic upgraded when grandmaster played regular move
				else if (UPGRADED == moveUpgrade(move)) {
					stats["h_up_when_gm_normal"] += 1;

					// Heuristic made exact same move, but just chose to upgrade
					if (h_pre_pos == gm_pre_pos and h_new_pos == gm_new_pos) {
						stats["h_up_when_gm_normal_same_square_same_piece"] += 1;
					} else if (h_pre_pos == gm_pre_pos) {
						stats["h_up_when_gm_normal_diff_square_same_piece"] += 1;
					} else if (h_new_pos == gm_new_pos) {
						stats["h_up_when_gm_normal_same_square_diff_piece"] += 1;
					} else {
						stats["h_up_when_gm_normal_diff_square_diff_piece"] += 1;
					}
				}
				else if (h_pre_pos == gm_pre_pos) {
					stats["h_move_diff_square_same_piece"] += 1;
				}
				else if (h_new_pos == gm_new_pos) {
					stats["h_move_same_square_diff_piece"] += 1;
				}
				else {
					stats["h_move_diff_square_diff_piece"] += 1;
				}
			}
	
	// Other stats, just keep track of total n positions 
	stats["total_positions"] += 1;
	}
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

		// Only look at drop moves in drop test mode
		if (mode == train_drops and !movePlaying(grandmaster_move)) continue;

		// Select a move using the given weights and set of shogi features
		int move = select_move(board, weights, positions);

		// Log statistics about the selection
		log_stats(board, move, grandmaster_move, weights);

		// Compare selection with the choice of the grandmaster
		if (move == grandmaster_move) {
			correct++;
		}

		if (DEBUG) {
			cout << "Grandmaster move:" << endl;
			cout << "     ";
			printMove(grandmaster_move);
			cout << "Heuristic chose:  " << endl;
			cout << "     ";
			printMove(move);
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

		// Only look at drop moves in drop test mode
		if (mode == train_drops and !movePlaying(grandmaster_move)) continue;

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

int OrganismEvaluator::evaluate_organism(vector<int> weights) {

	// Error check incase something invalid thrown from python (-1 bc now weight for pawn)
	if (weights.size() != heuristic.num_features()) {
		string error = "Expected " + to_string(heuristic.num_features()) + " weights but " \
									 "passed " + to_string(weights.size());

		throw invalid_argument(error);
	}

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

	if (log) {
		// Add some stats about overall organism evaluation
		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(stop - start);
		stats["eval_time_ms"] = int(duration.count());
		stats["total_positions_evaluated"] = positions;
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

		OrganismEvaluator evaluator;
		evaluator.set_mode("train_drops");

		evaluator.set_num_eval(500);
    /* vector<int> weights(evaluator.get_num_features(), 1); */

		vector<int> weights = {
    100, 340, 209, 675, 530, 886, 787, 970, 297, 313, 366, 188, 42, 649, 618, 824, 649, 980, 382, 855, 228,
    75, 33, 10, 39, 5, 31, 56, 9, 22, 25, 1, 2, 60, 59, 60, 10, 54, 58, 22, 35, 20, 49, 20, 7, 24, 58, 44,
    44, 58, 21, 23, 57, 14, 42, 19, 6, 45, 22, 29, 26, 9, 13, 8, 11, 0, 31, 50, 10, 21, 16, 14, 42, 6 };


		evaluator.evaluate_organism(weights);
		for (auto& e : evaluator.get_evaluation_stats()) {
			cout << e.first << ": " << e.second << endl;
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

