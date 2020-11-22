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

        ShogiFeatures(int player);
        /* ShogiFeatures() { this->weights = NULL; NUM_FEATURES = 20; }; */
        ShogiFeatures(int player, int* weights) : ShogiFeatures(player) { this->weights = weights; };
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
        int evaluate(Shogi s);
        vector<int> feature_vec(Shogi s) { return this->generate_feature_vec(s); };
        /* int evaluate(Shogi s, int* test_weights, int root_player, \ */
        /*     map<vector<unsigned char>, vector<int>>& tt, int& hits); */
        // int evaluate(Shogi s, vector<int>& weights, int root_player);
        int num_features() { return NUM_FEATURES; }
        int getPawnCount() { return pawn_count; }
        int getPawnValue() { return pawn_value; }
        int getPlayer() { return player; }
        void setPlayer(int newPlayer) { player = newPlayer; }
        void setPrint(int p) { print = p; }

    private:
        int print;
        int player;
        int* weights;
        int NUM_FEATURES;
        int pawn_index;
        int pawn_count;
        int pawn_value;
        vector<int> features;
        vector<int> generate_feature_vec(Shogi s);
        void material(Shogi& s);
        void king_safety(Shogi& s);
        void pieces_in_hand(Shogi& s);
        void controlled_squares(Shogi& s);
        void castle(Shogi& s);
        void board_shape(Shogi& s);
};

