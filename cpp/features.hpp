#include "helper.hpp"
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
        
        ShogiFeatures();
        /* ShogiFeatures() { this->weights = NULL; NUM_FEATURES = 20; }; */
        ShogiFeatures(int* weights) : ShogiFeatures() { this->weights = weights; };
        /* ShogiFeatures(string cache_file); */
        vector<int> gote_camp;
        vector<int> sente_camp;
        /* MovesCache cache; */
        map<pair<int, int>, string> piece_map;
        map<string, string> black_castles;
        map<string, string> white_castles;
        int CASTLE_THRESHOLD;

        // Add a transpossition table to store feature vector values
        /* map<vector<unsigned char>, vector<int>> tt; */
        /* bool tt_full = false; */


        // Multiple methods for evaluate depending on use in training or search
        int evaluate(Shogi s, int root_player);
        int evaluate(Shogi s, int* test_weights, int root_player, \
            map<vector<unsigned char>, vector<int>>& tt, int& hits);
        // int evaluate(Shogi s, vector<int>& weights, int root_player);
        int num_features() { return NUM_FEATURES; }

    private:
        int* weights;
        int NUM_FEATURES;
        void material(Shogi& s, int player, vector<int>& featVec);
        void king_safety(Shogi& s, int player, vector<int>& featVec);
        void pieces_in_hand(Shogi& s, int player, vector<int>& featVec);
        void controlled_squares(Shogi& s, int player, vector<int>& featVec);
        void castle(Shogi& s, int player, vector<int>& featVec);
        void board_shape(Shogi& s, int player, vector<int>& featVec);
};

