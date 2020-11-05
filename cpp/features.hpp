#include "shogi.hpp"
#include "lmcache.hpp"
#include <chrono> 
#include <numeric>
#include <map>
#include <iostream>

// For timing execution
using namespace std::chrono;

vector<pair<string, int>> loadGames(string in_file);

class ShogiFeatures {
    public:
        
        ShogiFeatures(string cache_file);
        vector<int> gote_camp;
        vector<int> sente_camp;
        MovesCache cache;
        map<pair<int, int>, string> piece_map;
        map<string, string> black_castles;
        map<string, string> white_castles;
        int CASTLE_THRESHOLD;



        int evaluate(Shogi s, int* weights, int root_player);
        // int evaluate(Shogi s, vector<int>& weights, int root_player);

    private:
        void material(Shogi& s, int player, vector<int>& featVec);
        void king_safety(Shogi& s, int player, vector<int>& featVec);
        void pieces_in_hand(Shogi& s, int player, vector<int>& featVec);
        void controlled_squares(Shogi& s, int player, vector<int>& featVec);
        void castle(Shogi& s, int player, vector<int>& featVec);
        void board_shape(Shogi& s, int player, vector<int>& featVec);
};

/* -------------- Load train, test, moves cache into memory --------------- */

/* "Double expansion" used to get correct paths from make file -D arguments */
#define STR1(x)  #x
#define STR(x)  STR1(x)

/**
 * 
 * Function to load json file of training data into program memory.
 * Json file is created by python script sample.py and saved in the format
 *          {board_state : grandmaster_move}
 */
vector<pair<string, int>> loadGames(string file_name) {
  ifstream fin(file_name);
  json j;
  fin >> j;

  vector<pair<string, int>> result;
  for (auto &item : j) {
    pair<string, int> game = {item["board"], item["pmove"]};
    result.push_back(game);
  }
  return result;
};

static const string lmcache = STR(MOVES_FILE);
extern const auto TRAIN = loadGames(STR(TRAIN_FILE));
extern const auto TEST = loadGames(STR(TEST_FILE));
extern const int NUM_FEATURES = 20;
