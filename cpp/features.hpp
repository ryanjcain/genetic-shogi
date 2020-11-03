#include "shogi.hpp"
#include "lmcache.hpp"
#include <chrono> 
#include <numeric>
#include <map>

static const string lmcache = "/Users/ryanjcain 1/Desktop/Yale/Spring 2020/Games/shogi/json/legal_moves_cache_raw.json";
static const string train_in = "/Users/ryanjcain 1/Desktop/Yale/Spring 2020/Games/shogi/json/train_data_raw.json";

// For timing execution
using namespace std::chrono; 

vector<pair<string, int>> to_array(string in_file);

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

vector<pair<string, int>> to_array(string in_file) {
    ifstream fin(in_file);

    json j;
    fin >> j;

    vector<pair<string, int>> result;
    for (auto& item : j) {
        pair<string, int>game = {item["board"], item["pmove"]};
        result.push_back(game);
    }
    return result;

};

extern const auto TRAIN = to_array(train_in);
extern const int NUM_FEATURES = 20;
