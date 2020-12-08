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
        ShogiFeatures(int player, vector<int> weights) : ShogiFeatures(player) { this->weights = weights; };
        /* ShogiFeatures(string cache_file); */
        vector<int> gote_camp;
        vector<int> sente_camp;
        vector<int> fifth_rank;
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
        vector<int> feature_vec_raw(Shogi s) { return this->generate_feature_vec_raw(s); };
        vector<string> features_vec_labels() { return feature_order; }
        int evaluate_feature_vec(vector<int>& fV, vector<int>& weights);

        /* int evaluate(Shogi s, int* test_weights, int root_player, \ */
        /*     map<vector<unsigned char>, vector<int>>& tt, int& hits); */
        // int evaluate(Shogi s, vector<int>& weights, int root_player);
        int num_features() { return n_features; }
        int num_major_features() { return n_major_features; }
        int getPawnCount() { return pawn_count; }
        int getPawnValue() { return pawn_value; }
        int getPlayer() { return player; }
        void setPlayer(int newPlayer) { player = newPlayer; }
        void setPrint(int p) { print = p; }

    private:
        int print;
        int player;
        vector<int> weights;
        int n_major_features;
        int n_features;
        int pawn_index;
        int pawn_count;
        int pawn_value;

        bool group_promotions;
        bool in_hand_bonus;
        bool link_promotions_and_in_hand;

        void init_features();
        map<string, int> features;

        // Keep track of the names and indexes of features in the feature vector
        // Major determines if it is a major feature and deserves larger bit width
        void add_feature(string name, int value, bool major);

        // Preserve the order of initialization as it is used later to split major and minor bit widths
        vector<string> feature_order;


        string pawn = "p";
        vector<string> piece_strings = {"p", "l", "n", "s", "b", "r", "+b", "+r", "g",
            "+p", "+l", "+n", "+s"};
        map<string, string> piece_strings_to_full = {{"p", "PAWN"}, {"l", "LANCE"}, {"n", "KNIGHT"},
            {"s", "SILVER"}, {"b", "BISHOP"}, {"r", "ROOK"}, {"+b", "PROMOTED_BISHOP"}, {"+r", "PROMOTED_ROOK"},
            {"g", "GOLD"}, {"+p", "PROMOTED_PAWN"}, {"+l", "PROMOTED_LANCE"}, {"+n", "PROMOTED_KNIGHT"},
            {"+s", "PROMOTED_SILVER"}};

        vector<string> in_hand_order = {"p", "l", "n", "s", "g", "b", "r"};

        // Cache the position(s) [0-81] of each kind of piece on the board since used many times in board shape feature
        // Key is a piece type string (as in piece_strings above), value is a pair of vectors
        // Pair.first  is the board locations of the given piece type for PLAYER
        // Pair.second is the board locations of the given piece type for OPPONENT
        // An example entry could be: {"+p", {<74, 81>, <12>}}
        // If player is white this means they have two upgraded pawns on square 74 and 81, while black has one
        // upgraded pawn on square 12
        map<string, pair<vector<int>, vector<int>>> piece_pos;

        // Pieces that move the same as gold
        map<string, int> move_as_gold = {{"g", 1}, {"+n", 1}, {"+s", 1}, {"+l", 1}, {"+p", 1}};


        /*
         * Return a vector containing squares distance 1 away from pos indexed 0-8,
         * Out of bound values are -1. So for pos 40 there are 8 valid adjacent squares and the result would be
         *
         *  [ 48  39  30                 [ 0  1  2
         *    49  XX  31     indexed as:   3     4
         *    50  41  32 ]                 5  6  7 ]
         *
         *  Edge case example with only 3 valid adjacent squares
         *
         *   [ -1  79  70                 [ 0  1  2
         *     -1  XX  71     indexed as:   3     4
         *     -1  -1  -1 ]                 5  6  7 ]
        */
        vector<int> find_adjacent(int pos);

        /*
         * Constants and other thresholds used by some of the features.
         */

        /* Number of 4 corner squares to consider when checking if bishop boxed in */
        int bishop_box_penalty = 3;

        /* For readability when coding with find_adjacent output */
        int left = 3, right = 4;
        int topL = 0, top = 1, topR = 2;
        int botL = 5, bot = 6, botR = 7;

        // Helper Functions
        bool in_bounds(int pos) { return (0 <= pos and pos <= 80); }
        int count_adj_pairs(string piece_type, Shogi& s);
        vector<int> find_flow_moves(string piece_type, Shogi& s);
        int count_safe_squares(vector<int> squares, Shogi& s);
        int distance(int posA, int posB);

        vector<int> generate_feature_vec_raw(Shogi s);
        void load_features(Shogi& s);

        // Feature functions
        void material(Shogi& s);
        void material_in_hand(Shogi& s);
        void king_safety(Shogi& s);
        void total_pieces_in_hand(Shogi& s);
        void controlled_squares(Shogi& s);
        void castle(Shogi& s);

        // Penalty features for bad shape
        void gold_ahead_silver_penalty(Shogi& s);
        void gold_adjacent_rook_penalty(Shogi& s);
        // Small penalty if bishop boxed in on at least bishop_box_penalty sides
        void boxed_in_bishop_penalty(Shogi& s);
        void piece_ahead_of_pawns_penalty(Shogi& s);

        // Features for GOOD shape
        void bishop_head_protected(Shogi& s);
        void reclining_silver(Shogi& s);
        void claimed_files(Shogi& s);
        void adjacent_silvers(Shogi& s);
        void adjacent_golds(Shogi& s);
        void rook_enemy_camp(Shogi& s);
        void rook_attack_king_file(Shogi& s);
        void rook_attack_king_adj_file(Shogi& s);
        void rook_attack_king_adj_file_9821(Shogi& s);
        void rook_open_semi_open_file(Shogi& s);
        void bishop_mobility(Shogi& s);
        void rook_mobility(Shogi& s);
        void aggression_balance(Shogi& s);

        // Try to help with drops, count the number of pieces that are blocking flow of an enemy
        void blocked_flow(Shogi& s);

        void king_attack(Shogi& s);
        void distance_to_kings(Shogi& s);

        // General difference in the number of squares each player is attacking. Should give a good
        // Idea about spacinf of a players pieces.
        void total_attacking(Shogi& s);

        // Used for visual debugging
        void print_piece_map();


        // Used to decide moves, coppied from shogi.cpp because shogi.cpp clobers global namespace
        int movingD[14][8] = {
          {0}, {0, 0, 0, 0, 0}, {0, 0}, {1}, {1, 1, 1, 1},
          {1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0},
          {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0},
          {0, 0, 0, 0, 1, 1, 1, 1}, {0, 0, 0, 0, 1, 1, 1, 1}
        };

        int movingDlength[14] = {
          1, 5, 2, 1, 4, 4, 8, 6, 6, 6, 6, 6, 8, 8
        };

        int danD[14][8] = {
          {-1}, {-1, -1, -1, 1, 1}, {-2, -2}, {-1}, {-1, 0, 1, 0}, {-1, -1, 1, 1},
          {-1, -1, -1, 1, 1, 1, 0, 0}, {-1, -1, -1, 0, 0, 1}, {-1, -1, -1, 0, 0, 1},
          {-1, -1, -1, 0, 0, 1}, {-1, -1, -1, 0, 0, 1}, {-1, -1, -1, 0, 0, 1},
          {-1, -1, 1, 1, -1, 0, 1, 0}, {-1, 0, 1, 0, -1, -1, 1, 1}
        };

        int sujiD[14][8] = {
          {0}, {-1, 0, 1, -1, 1}, {-1, 1}, {0}, {0, -1, 0, 1}, {1, -1, 1, -1},
          {1, 0, -1, 1, 0, -1, 1, -1}, {1, 0, -1, 1, -1, 0}, {1, 0, -1, 1, -1, 0},
          {1, 0, -1, 1, -1, 0}, {1, 0, -1, 1, -1, 0}, {1, 0, -1, 1, -1, 0},
          {1, -1, 1, -1, 0, 1, 0, -1}, {0, 1, 0, -1, 1, -1, 1, -1}
        };
};

