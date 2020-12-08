#include "features.hpp"
#include <map>
#include <climits>
#include <algorithm>
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
		void set_num_eval(int num_eval);
		int evaluate_organism(vector<int> weights);
		map<string, int> get_evaluation_stats() { return stats; };
		void set_mode(string mode_string);
		string get_mode() { return mode; };
		int get_num_eval() { return n_eval; }
		vector<string> get_feature_labels() { return heuristic.features_vec_labels(); }
		int get_num_features() { return heuristic.num_features(); }
		int get_num_major_features() { return heuristic.num_major_features(); };

	private:
		// Number of positions in the train or test data to evaluate
		int n_eval;

		// Whether to evaluate using train or test data
		string mode;
		const string test_mode = "test";
		const string train_mode = "train";
		const string train_drops = "train_drops";
		vector<string> modes = {test_mode, train_mode, train_drops};
		bool log;
		void log_stats(string board, int move, int grandmaster_move);


		map<string, int> stats;
		MovesCache cache;
		ShogiFeatures heuristic;
		Shogi load_game(string board);
		void init_stats();

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

		// Load legal moves cache, train, and test data at compile time
		const string lm_cache = STR(MOVES_FILE);
		vector<pair<string, int>> sample;
		const vector<pair<string, int>> train_data = loadGames(STR(TRAIN_FILE));
		const vector<pair<string, int>> test_data = loadGames(STR(TEST_FILE));
		int n_train = train_data.size();
		int n_test = test_data.size();
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
