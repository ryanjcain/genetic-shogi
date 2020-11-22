#include "features.hpp"
#include <map>
#include <omp.h>


// For timing executions and training times
using namespace std::chrono;

vector<pair<string, int>> loadGames(string in_file);

class OrganismEvaluator {
	public:
		OrganismEvaluator(string moves_cache_file);
		void evaluate(int* weights, int& correct, int& positions);
		int evaluate_synchronous(int* weights, int&pos);
		int evaluate_parallel(int* weights, int&pos);
		int select_move(string board, int* weights, int& pos);
		bool feature_cache_loaded() { return tt_full; };
		void update_tt_status(bool status) { tt_full = status; };

	private:
		MovesCache cache;
		ShogiFeatures heuristic;
		Shogi load_game(string board);

		// Add a transpossition table to store feature vector values
		// NOTE : MAKE THIS AN UNORDERED MAP ASAP!
		map<vector<unsigned char>, vector<int>> feature_tt;
		bool tt_full = false;
};

/* -------------- Load train, test, moves cache into memory --------------- */

/* "Double expansion" used to get / check paths from make file -D arguments */
#define STR1(x)  #x
#define STR(x)  STR1(x)

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

static const string LM_CACHE = STR(MOVES_FILE);
extern const auto TRAIN = loadGames(STR(TRAIN_FILE));
extern const auto TEST = loadGames(STR(TEST_FILE));
