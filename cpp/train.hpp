#include "features.hpp"
#include <map>
#include <omp.h>

#define STR1(x)  #x
#define STR(x)  STR1(x)

// For timing executions and training times
using namespace std::chrono;

vector<pair<string, int>> loadGames(string in_file);

class OrganismEvaluator {
	public:
		OrganismEvaluator();
		void evaluate(vector<int> weights, int& correct, int& positions);
		int evaluate_synchronous(vector<int> weights, int&pos);
		int evaluate_parallel(vector<int> weights, int&pos);
		int select_move(string board, vector<int> weights, int& pos);
		bool feature_cache_loaded() { return tt_full; };
		void update_tt_status(bool status) { tt_full = status; };
		int evaluate_organism(vector<int> weights);

	private:
		MovesCache cache;
		ShogiFeatures heuristic;
		Shogi load_game(string board);

		// Add a transpossition table to store feature vector values
		// NOTE : MAKE THIS AN UNORDERED MAP ASAP!
		map<vector<unsigned char>, vector<int>> feature_tt;
		bool tt_full = false;

		/**
		 *
		 * Function to load json file of training data into program memory.
		 * Json file is created by python script sample.py and saved in the format
		 *          {board_state : grandmaster_move}
		 */
		vector<pair<string, int>> loadGames(string file_name) {
			// Throw error if file doesnt exist
			ifstream fin(file_name);
			if (fin.fail()) {
					cout << "Invalid file: " << file_name << endl;
					exit(-1);
			}

			json j;
			fin >> j;
			vector<pair<string, int>> result;
			for (auto &item : j) {
				pair<string, int> game = {item["board"], item["pmove"]};
				result.push_back(game);
			}
			return result;
		};

		const string LM_CACHE = STR(MOVES_FILE);
		const vector<pair<string, int>> TRAIN = loadGames(STR(TRAIN_FILE));
		const vector<pair<string, int>> TEST = loadGames(STR(TEST_FILE));
};

/* -------------- Load train, test, moves cache into memory --------------- */

/* "Double expansion" used to get / check paths from make file -D arguments */
/* #define STR1(x)  #x */
/* #define STR(x)  STR1(x) */

/**
 *
 * Function to load json file of training data into program memory.
 * Json file is created by python script sample.py and saved in the format
 *          {board_state : grandmaster_move}
 */
/*vector<pair<string, int>> loadGames(string file_name) {*/
/*  // Throw error if file doesnt exist*/
/*  ifstream fin(file_name);*/
/*  if (fin.fail()) {*/
/*      cout << "Invalid file: " << file_name << endl;*/
/*      exit(-1);*/
/*  }*/

/*  json j;*/
/*  fin >> j;*/
/*  vector<pair<string, int>> result;*/
/*  for (auto &item : j) {*/
/*    pair<string, int> game = {item["board"], item["pmove"]};*/
/*    result.push_back(game);*/
/*  }*/
/*  return result;*/
/*};*/

/*static const string LM_CACHE = STR(MOVES_FILE);*/
/*extern const auto TRAIN = loadGames(STR(TRAIN_FILE));*/
/*extern const auto TEST = loadGames(STR(TEST_FILE));*/
