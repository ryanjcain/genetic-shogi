#include "features.hpp"
#include <omp.h>

ShogiFeatures::ShogiFeatures(int player) {

    // Set the color (perspective) for the heuristic to be evaluated
    this->player = player;

    // Initialize weights to null if not provided
    weights = {};

    // Initialize a piece map, convert from int encoding to string
    piece_map = {
        {{PAWN, NORMAL}, "p"},
        {{LANCE, NORMAL}, "l"},
        {{KNIGHT, NORMAL}, "n"},
        {{SILVER, NORMAL}, "s"},
        {{GOLD, NORMAL}, "g"},
        {{BISHOP, NORMAL}, "b"},
        {{ROOK, NORMAL}, "r"},
        {{PAWN, UPGRADED}, "+p"},
        {{LANCE, UPGRADED}, "+l"},
        {{KNIGHT, UPGRADED}, "+n"},
        {{SILVER, UPGRADED}, "+s"},
        {{BISHOP, UPGRADED}, "+b"},
        {{ROOK, UPGRADED}, "+r"},
    };

    // Initialize the map of castle formations with hex board representations
    black_castles = {
        {"left_mino", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0007FFFFFFFFFFFFFF00FF07FFFFFFFFFF000501FFFFFFFFFFFFFF000602FFFFFFFFFF00FFFF03000000000000000000000000000000000000"},
        {"gold_fortress", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFF0007FFFFFFFFFFFFFF000107FFFFFFFFFFFFFF000602FFFFFFFFFFFF00FF03000000000000000000000000000000000000"},
        {"helmet", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0007FFFFFFFFFFFFFF0006FFFFFFFFFFFF000107FFFFFFFFFFFFFF00FF02FFFFFFFFFF00FFFF03000000000000000000000000000000000000"},
        {"crab", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FF07FFFFFFFFFFFFFF000106FFFFFFFFFF00FF07FFFFFFFFFFFFFF00FF02FFFFFFFFFFFF00FF03000000000000000000000000000000000000"},
        {"bonanza", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0007FFFFFFFFFFFFFF0007FFFFFFFFFFFF000106FFFFFFFFFFFFFF00FF02FFFFFFFFFFFF00FF03000000000000000000000000000000000000"},
        {"snowroof", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000107FFFFFFFFFFFF0001FF06FFFFFFFFFF00FF07FFFFFFFFFFFFFF00FF02FFFFFFFFFFFF00FF03000000000000000000000000000000000000"},
        {"silver_horns_snowroof", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFF0001FFFFFFFFFFFFFFFF0007FFFFFFFFFFFF0001FF06FFFFFFFFFF00FF07FFFFFFFFFFFFFF00FF02FFFFFFFFFFFF00FF03000000000000000000000000000000000000"},
        {"right_king_1", "FFFFFFFFFF00FFFF03FFFFFFFF00FFFFFF04FFFFFFFFFF000206FFFFFFFFFFFF0001FFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"right_king_2", "FFFFFFFF00FFFFFF03FFFFFFFF00FFFFFF04FFFFFFFFFF000206FFFFFFFFFFFF000107FFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"right_king_3", "FFFFFFFFFF00FFFF03FFFFFFFF00FFFFFF04FFFFFFFFFF0002FFFFFFFFFFFFFF000106FFFFFFFFFFFFFF0007FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"central_house", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0007FFFFFFFFFFFFFF0001FFFFFFFFFFFFFF0006FFFFFFFFFFFFFF0001FFFFFFFFFFFFFF0007FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"nakahara", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0001FFFFFFFFFFFFFF00FF07FFFFFFFFFFFF00FF06FFFFFFFFFF00FF0701FFFFFFFFFFFF00FF02FFFFFFFFFFFF00FF03000000000000000000000000000000000000"},
        {"duck", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FF07FFFFFFFFFFFF0001FFFFFFFFFFFFFF0006FFFFFFFFFFFFFF0001FFFFFFFFFFFFFF00FF07FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"paperweight", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FF07FFFFFFFFFFFFFF0007FFFFFFFFFFFF000206FFFFFFFFFFFFFF0001FFFFFFFFFFFF00FF03FF000000000000000000000000000000000000"},
        {"truck", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFF000107FFFFFFFFFFFF000107FFFFFFFFFFFF00FF06FFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"boat_pawn", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FF07FFFFFFFFFFFFFF00FF07FFFFFFFFFF00FF0601FFFFFFFFFFFF000502FFFFFFFFFF00FFFF03000000000000000000000000000000000000"},
        {"daughter_inside_box", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000707FFFFFFFFFF00FF0601FFFFFFFFFFFF000502FFFFFFFFFF00FFFF03000000000000000000000000000000000000"},
        {"diamond", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000107FFFFFFFFFFFFFF000107FFFFFFFFFF00FF06FFFFFFFFFFFFFF000502FFFFFFFFFF00FFFF03000000000000000000000000000000000000"},
        {"strawberry", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0007FFFFFFFFFFFFFF0006FFFFFFFFFFFF00FF0701FFFFFFFFFFFF000502FFFFFFFFFF00FFFF03000000000000000000000000000000000000"},
        {"yonenaga", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFF0007FFFFFFFFFFFFFF00FF07FFFFFFFFFFFF00010502FFFFFFFFFF00FF0603000000000000000000000000000000000000"},
        {"elmo", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0001FFFFFFFFFFFF00FF0607FFFFFFFFFFFF000502FFFFFFFFFF00FFFF03000000000000000000000000000000000000"},
        {"elmo_gold", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF07FFFFFFFFFFFF0001FFFFFFFFFFFF00FF0607FFFFFFFFFFFF000502FFFFFFFFFF00FFFF03000000000000000000000000000000000000"},
        {"silver_elephant_eye", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0001FFFFFFFFFFFFFF000601FFFFFFFFFFFF0002FFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"gold_elephant_eye", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0001FFFFFFFFFFFFFF000607FFFFFFFFFFFF0002FFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"kushikatsu", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFF00FF0707FFFFFFFFFFFF000102FFFFFFFFFFFF000603000000000000000000000000000000000000"},
        {"anaguma", "FFFFFFFFFFFF000306FFFFFFFFFFFF000102FFFFFFFFFFFF000707FFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"mino", "FFFFFFFFFF00FFFF03FFFFFFFFFFFF000602FFFFFFFFFFFF0001FFFFFFFFFFFFFF00FF07FFFFFFFFFFFF0007FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"silver_crown", "FFFFFFFFFF00FFFF03FFFFFFFFFF000106FFFFFFFFFFFF000207FFFFFFFFFFFF0007FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"wall", "FFFFFFFFFFFF00FF03FFFFFFFFFFFF00FF02FFFFFFFFFFFF0006FFFFFFFFFFFFFF000107FFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"gold_mino", "FFFFFFFFFFFF00FF03FFFFFFFFFFFF000602FFFFFFFFFFFF0007FFFFFFFFFFFFFF0001FFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"three_move", "FFFFFFFFFFFF00FF03FFFFFFFFFFFF00FF02FFFFFFFFFFFF000601FFFFFFFFFFFF0007FFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"rapid_castle", "FFFFFFFFFFFF00FF03FFFFFFFFFFFF000602FFFFFFFFFFFF000701FFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"flatfish", "FFFFFFFFFF00FFFF03FFFFFFFFFFFF000602FFFFFFFFFFFF0001FFFFFFFFFFFFFF00FF07FFFFFFFFFFFF00FF07FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"millenium_1", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFF0007FFFFFFFFFFFFFF00020701FFFFFFFFFFFF00FF06FFFFFFFFFFFF00FF03000000000000000000000000000000000000"},
        {"millenium_2", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFF0007FFFFFFFFFFFFFF00020701FFFFFFFFFFFF000106FFFFFFFFFFFF00FF03000000000000000000000000000000000000"},
        {"millenium_3", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFF0007FFFFFFFFFFFFFF00020701FFFFFFFFFF00FF0106FFFFFFFFFF00FFFF03000000000000000000000000000000000000"},
        {"millenium_4", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFF0007FFFFFFFFFFFFFF00020701FFFFFFFFFF0001FF06FFFFFFFFFFFF00FF03000000000000000000000000000000000000"},
        {"millenium_5", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0001FFFFFFFFFFFF00020707FFFFFFFFFFFF000106FFFFFFFFFFFF00FF03000000000000000000000000000000000000"},
        {"millenium_6", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0001FFFFFFFFFFFFFF000701FFFFFFFFFFFF00FF0706FFFFFFFFFF00FF0502FFFFFFFFFFFF00FF03000000000000000000000000000000000000"},
        {"gold_excelsior", "FFFFFFFFFF00FFFF03FFFFFFFFFFFF000102FFFFFFFFFFFF0006FFFFFFFFFFFFFF0007FFFFFFFFFFFFFF0007FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"aerokin", "FFFFFFFFFF000603FFFFFFFFFFFF0001FFFFFFFFFFFFFF000207FFFFFFFFFFFF0007FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"aerial_tower", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000102FFFFFFFFFFFF000607FFFFFFFFFFFF00FFFFFF03000000000000000000000000000000000000"},
        {"fourth_edge_king", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0001FFFFFFFFFFFFFF000702FFFFFFFFFFFF0007FFFFFFFFFFFFFF0006FFFF03000000000000000000000000000000000000"}
    };

    white_castles = {
        {"left_mino_white", "13FFFF10FFFFFFFFFF121610FFFFFFFFFFFFFF111510FFFFFFFFFF17FF10FFFFFFFFFFFFFF1710FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"gold_fortress_white", "13FF10FFFFFFFFFFFF121610FFFFFFFFFFFFFF171110FFFFFFFFFFFFFF1710FFFFFFFFFFFFFFFF10FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"helmet_white", "13FFFF10FFFFFFFFFF12FF10FFFFFFFFFFFFFF171110FFFFFFFFFFFF1610FFFFFFFFFFFFFF1710FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"crab_white", "13FF10FFFFFFFFFFFF12FF10FFFFFFFFFFFFFF17FF10FFFFFFFFFF161110FFFFFFFFFFFFFF17FF10FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"bonanza_white", "13FF10FFFFFFFFFFFF12FF10FFFFFFFFFFFFFF161110FFFFFFFFFFFF1710FFFFFFFFFFFFFF1710FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"snowroof_white", "13FF10FFFFFFFFFFFF12FF10FFFFFFFFFFFFFF17FF10FFFFFFFFFF16FF1110FFFFFFFFFFFF171110FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"silver_horns_snowroof_white", "13FF10FFFFFFFFFFFF12FF10FFFFFFFFFFFFFF17FF10FFFFFFFFFF16FF1110FFFFFFFFFFFF1710FFFFFFFFFFFFFFFF1110FFFFFFFFFFFFFFFF10FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"right_king_1_white", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF10FFFFFFFFFFFFFF1110FFFFFFFFFFFF161210FFFFFFFFFF14FFFFFF10FFFFFFFF13FFFF10FFFFFFFFFF000000000000000000000000000000000000"},
        {"right_king_2_white", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF10FFFFFFFFFFFF171110FFFFFFFFFFFF161210FFFFFFFFFF14FFFFFF10FFFFFFFF13FFFFFF10FFFFFFFF000000000000000000000000000000000000"},
        {"right_king_3_white", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF1710FFFFFFFFFFFFFF161110FFFFFFFFFFFFFF1210FFFFFFFFFF14FFFFFF10FFFFFFFF13FFFF10FFFFFFFFFF000000000000000000000000000000000000"},
        {"central_house_white", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF1710FFFFFFFFFFFFFF1110FFFFFFFFFFFFFF1610FFFFFFFFFFFFFF1110FFFFFFFFFFFFFF1710FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"nakahara_white", "13FF10FFFFFFFFFFFF12FF10FFFFFFFFFFFF1117FF10FFFFFFFFFF16FF10FFFFFFFFFFFF17FF10FFFFFFFFFFFFFF1110FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"duck_white", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF17FF10FFFFFFFFFFFFFF1110FFFFFFFFFFFFFF1610FFFFFFFFFFFFFF1110FFFFFFFFFFFF17FF10FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"paperweight_white", "FF13FF10FFFFFFFFFFFF1110FFFFFFFFFFFFFF161210FFFFFFFFFFFF1710FFFFFFFFFFFFFF17FF10FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"truck_white", "FFFFFFFFFFFFFFFFFFFFFF10FFFFFFFFFFFFFF16FF10FFFFFFFFFFFF171110FFFFFFFFFFFF171110FFFFFFFFFFFFFFFF10FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"boat_pawn_white", "13FFFF10FFFFFFFFFF121510FFFFFFFFFFFF1116FF10FFFFFFFFFF17FF10FFFFFFFFFFFFFF17FF10FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"daughter_inside_box_white", "13FFFF10FFFFFFFFFF121510FFFFFFFFFFFF1116FF10FFFFFFFFFF171710FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"diamond_white", "13FFFF10FFFFFFFFFF121510FFFFFFFFFFFFFF16FF10FFFFFFFFFF171110FFFFFFFFFFFFFF171110FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"strawberry_white", "13FFFF10FFFFFFFFFF121510FFFFFFFFFFFF1117FF10FFFFFFFFFFFF1610FFFFFFFFFFFFFF1710FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"yonenaga_white", "1316FF10FFFFFFFFFF12151110FFFFFFFFFFFF17FF10FFFFFFFFFFFFFF1710FFFFFFFFFFFFFFFF10FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"elmo_white", "13FFFF10FFFFFFFFFF121510FFFFFFFFFFFF1716FF10FFFFFFFFFFFF1110FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"elmo_gold_white", "13FFFF10FFFFFFFFFF121510FFFFFFFFFFFF1716FF10FFFFFFFFFFFF1110FFFFFFFFFFFF17FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"silver_elephant_eye_white", "FFFFFFFFFFFFFFFFFFFFFFFF10FFFFFFFFFFFFFF1210FFFFFFFFFFFF111610FFFFFFFFFFFFFF1110FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"gold_elephant_eye_white", "FFFFFFFFFFFFFFFFFFFFFFFF10FFFFFFFFFFFFFF1210FFFFFFFFFFFF171610FFFFFFFFFFFFFF1110FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"kushikatsu_white", "131610FFFFFFFFFFFF121110FFFFFFFFFFFF1717FF10FFFFFFFFFFFFFF10FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"anaguma_white", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF10FFFFFFFFFFFF171710FFFFFFFFFFFF121110FFFFFFFFFFFF161310FFFFFFFFFFFF000000000000000000000000000000000000"},
        {"mino_white", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF1710FFFFFFFFFFFF17FF10FFFFFFFFFFFFFF1110FFFFFFFFFFFF121610FFFFFFFFFFFF13FFFF10FFFFFFFFFF000000000000000000000000000000000000"},
        {"silver_crown_white", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF1710FFFFFFFFFFFF171210FFFFFFFFFFFF161110FFFFFFFFFF13FFFF10FFFFFFFFFF000000000000000000000000000000000000"},
        {"wall_white", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF10FFFFFFFFFFFF171110FFFFFFFFFFFFFF1610FFFFFFFFFFFF12FF10FFFFFFFFFFFF13FF10FFFFFFFFFFFF000000000000000000000000000000000000"},
        {"gold_mino_white", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF10FFFFFFFFFFFFFF1110FFFFFFFFFFFFFF1710FFFFFFFFFFFF121610FFFFFFFFFFFF13FF10FFFFFFFFFFFF000000000000000000000000000000000000"},
        {"three_move_white", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF10FFFFFFFFFFFFFF1710FFFFFFFFFFFF111610FFFFFFFFFFFF12FF10FFFFFFFFFFFF13FF10FFFFFFFFFFFF000000000000000000000000000000000000"},
        {"rapid_castle_white", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF10FFFFFFFFFFFF111710FFFFFFFFFFFF121610FFFFFFFFFFFF13FF10FFFFFFFFFFFF000000000000000000000000000000000000"},
        {"flatfish_white", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF17FF10FFFFFFFFFFFF17FF10FFFFFFFFFFFFFF1110FFFFFFFFFFFF121610FFFFFFFFFFFF13FFFF10FFFFFFFFFF000000000000000000000000000000000000"},
        {"millenium_1_white", "13FF10FFFFFFFFFFFF16FF10FFFFFFFFFFFF11171210FFFFFFFFFFFFFF1710FFFFFFFFFFFFFFFF10FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"millenium_2_white", "13FF10FFFFFFFFFFFF161110FFFFFFFFFFFF11171210FFFFFFFFFFFFFF1710FFFFFFFFFFFFFFFF10FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"millenium_3_white", "13FFFF10FFFFFFFFFF1611FF10FFFFFFFFFF11171210FFFFFFFFFFFFFF1710FFFFFFFFFFFFFFFF10FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"millenium_4_white", "13FF10FFFFFFFFFFFF16FF1110FFFFFFFFFF11171210FFFFFFFFFFFFFF1710FFFFFFFFFFFFFFFF10FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"millenium_5_white", "13FF10FFFFFFFFFFFF161110FFFFFFFFFFFF17171210FFFFFFFFFFFF1110FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"millenium_6_white", "13FF10FFFFFFFFFFFF1215FF10FFFFFFFFFF1617FF10FFFFFFFFFFFF111710FFFFFFFFFFFFFF1110FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"gold_excelsior_white", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF1710FFFFFFFFFFFFFF1710FFFFFFFFFFFFFF1610FFFFFFFFFFFF121110FFFFFFFFFFFF13FFFF10FFFFFFFFFF000000000000000000000000000000000000"},
        {"aerokin_white", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF1710FFFFFFFFFFFF171210FFFFFFFFFFFFFF1110FFFFFFFFFFFF131610FFFFFFFFFF000000000000000000000000000000000000"},
        {"aerial_tower_white", "13FFFFFF10FFFFFFFFFFFF171610FFFFFFFFFFFF121110FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"},
        {"fourth_edge_king_white", "13FFFF1610FFFFFFFFFFFFFF1710FFFFFFFFFFFF121710FFFFFFFFFFFFFF1110FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000000000000000"}
    };

    // Initialize the vectors storing the upper and lower camps on the board
    //      Doing this manually because the board ordering is strange:
    //
    //          72 63 54 45 36 27 18 09 00
    //          73 64 55 46 37 28 19 10 01
    //          74 65 56 47 38 29 20 11 02
    //          75 66 57 48 39 30 21 12 03
    //          76 67 58 49 40 31 22 13 04
    //          77 68 59 50 41 32 23 14 05
    //          78 69 60 51 42 33 24 15 06
    //          79 70 61 52 43 34 25 16 07
    //          80 71 62 53 44 35 26 17 08

    gote_camp = {72, 63, 54, 45, 36, 27, 18,  9, 0,
                 73, 64, 55, 46, 37, 28, 19, 10, 1,
                 74, 65, 56, 47, 38, 29, 20, 11, 2};

    sente_camp = {78, 69, 60, 51, 42, 33, 24, 15, 6,
                  79, 70, 61, 52, 43, 34, 25, 16, 7,
                  80, 71, 62, 53, 44, 35, 26, 17, 8};

    fifth_rank = {76, 67, 58, 49, 40, 31, 22, 13, 04};

    CASTLE_THRESHOLD = 100;

    n_major_features = 0;

    pawn_count = 0;
    pawn_index = 0;
    pawn_value = 100;
    group_promotions = false;
    in_hand_bonus = true;
    link_material = true;

    // Default initialize the feature vector
    init_features();

    n_features = features.size();
}

void ShogiFeatures::add_feature(string name, bool major, string link="") {
    features[name] = 0;
    n_major_features += major ? 1 : 0;
    feature_order.push_back(name);

    // Link the feature to the other
    if (!link.empty()) {
        if (link == "PAWN_VALUE") {
            feature_links[name] = -1;
        } else {
            // Otherwise add the link to the other feature
            auto itr = find(feature_order.begin(), feature_order.end(), link);
            if (itr != feature_order.end()) {
                int index = std::distance(feature_order.begin(), itr);
                feature_links[name] = index;
            } else {
                string error = "Cannot link " + link + ": does not exist";
                throw invalid_argument(error);
            }
        }
    }
}

void ShogiFeatures::init_features() {
    // Initialize major features (longer bit width) first
    add_feature("LANCE_VALUE", true);
    add_feature("KNIGHT_VALUE", true);
    add_feature("SILVER_VALUE", true);
    add_feature("BISHOP_VALUE", true);
    add_feature("ROOK_VALUE", true);

    // Add pieces according to configuration
    if (group_promotions) {
        add_feature("GOLD_AND_EQV_VALUE", true);
    } else {
        add_feature("GOLD_VALUE", true);
        add_feature("PROMOTED_PAWN_BONUS", true, "PAWN_VALUE");
        add_feature("PROMOTED_LANCE_BONUS", true, "LANCE_VALUE");
        add_feature("PROMOTED_KNIGHT_BONUS", true, "KNIGHT_VALUE");
        add_feature("PROMOTED_SILVER_BONUS", true, "SILVER_VALUE");
    }
    add_feature("PROMOTED_BISHOP_BONUS", true, "BISHOP_VALUE");
    add_feature("PROMOTED_ROOK_BONUS", true, "ROOK_VALUE");

    // Initialize individual values for pieces in hand based on config
    if (in_hand_bonus) {
        add_feature("PAWN_IN_HAND_BONUS", true, "PAWN_VALUE");
        add_feature("LANCE_IN_HAND_BONUS", true, "LANCE_VALUE");
        add_feature("KNIGHT_IN_HAND_BONUS", true, "KNIGHT_VALUE");
        add_feature("SILVER_IN_HAND_BONUS", true, "SILVER_VALUE");
        add_feature("BISHOP_IN_HAND_BONUS", true, "BISHOP_VALUE");
        add_feature("ROOK_IN_HAND_BONUS", true, "ROOK_VALUE");
        add_feature("GOLD_IN_HAND_BONUS", true, "GOLD_VALUE");
    } else {
        add_feature("PIECES_IN_HAND", false);
    }

    add_feature("PLAYER_KING_DEFENDERS", false);
    add_feature("PLAYER_KING_ESCAPE_ROUTES", false);
    add_feature("PLAYER_KING_THREAT_PENALTY", false);
    add_feature("IN_CAMP_VULNERABILITY_PENALTY", false);
    add_feature("OUT_CAMP_ATTACK", false);
    add_feature("CASTLE_FORMATION", false);
    add_feature("GOLD_AHEAD_SILVER_PENALTY", false);
    add_feature("GOLD_ADJACENT_ROOK_PENALTY", false);
    add_feature("BOXED_IN_BISHOP_PENALTY", false);
    add_feature("PIECE_AHEAD_OF_PAWN_PENALTY", false);
    add_feature("BISHOP_HEAD_PROTECTED", false);
    add_feature("RECLINING_SILVER", false);
    add_feature("CLAIMED_FILES", false);
    add_feature("ADJACENT_SILVERS", false);
    add_feature("ADJACENT_GOLDS", false);
    add_feature("ROOK_ENEMY_CAMP", false);
    add_feature("ROOK_ATTACK_KING_FILE", false);
    add_feature("ROOK_ATTACK_KING_ADJ_FILE", false);
    add_feature("ROOK_ATTACK_KING_ADJ_FILE_9821", false);
    add_feature("ROOK_OPEN_FILE", false);
    add_feature("ROOK_SEMI_OPEN_FILE", false);
    add_feature("BISHOP_MOBILITY", false);
    add_feature("ROOK_MOBILITY", false);
    add_feature("BLOCKED_FLOW", false);
    add_feature("AGGRESSION_BALANCE", false);
    add_feature("ENEMY_KING_ATTACKS", false);
    add_feature("ENEMY_KING_ATTACKS_SAFE", false);
    add_feature("TOTAL_ATTACKING", false);
    add_feature("DISTANCE_TO_KINGS", false);
}

void ShogiFeatures::load_features(Shogi& s) {
    // Individual feature calculations
    material(s);
    material_in_hand(s);
    king_safety(s);
    controlled_squares(s);
    castle(s);
    gold_ahead_silver_penalty(s);
    gold_adjacent_rook_penalty(s);
    boxed_in_bishop_penalty(s);
    piece_ahead_of_pawns_penalty(s);
    bishop_head_protected(s);
    reclining_silver(s);
    claimed_files(s);
    adjacent_silvers(s);
    adjacent_golds(s);
    bishop_mobility(s);
    rook_mobility(s);
    rook_enemy_camp(s);
    rook_attack_king_file(s);
    rook_attack_king_adj_file(s);
    rook_attack_king_adj_file_9821(s);
    rook_open_semi_open_file(s);
    blocked_flow(s);
    aggression_balance(s);
    king_attack(s);
    total_attacking(s);
    distance_to_kings(s);
}

vector<int> ShogiFeatures::generate_feature_vec_raw(Shogi s) {
    // Set / Reset feature vector and pawn count to 0
    pawn_count = 0;

    // Initialize position cache
    for (string piece : piece_strings) {
        piece_pos[piece].first = {};
        piece_pos[piece].second = {};
    }

    // Calcualte all feature values and save them to internal featues map
    load_features(s);

    vector<int> feature_vec;
    for (auto& name : feature_order) {
        feature_vec.push_back(features[name]);
    }
    return feature_vec;
}

int ShogiFeatures::evaluate_feature_vec(vector<int>& fV, vector<int>& weights) {
    if (fV.size() > n_features or weights.size() > n_features or fV.size() != weights.size()) {
        string error = "Expected fV and weights to be size of N features";
        throw invalid_argument(error);
    }

    // Initialize score with pawn value and accumulate other features with weights
    int score = pawn_count * pawn_value;
    /* for (int i = 0; i < n_features; i++) { */
    /*     score += fV[i] * weights[i]; */
    /* } */

    for (int i = 0; i < n_features; i++) {
        // See if the feature is linked to another weight
        if (feature_links.count(feature_order[i])) {
            int linked_index = feature_links[feature_order[i]];
            int linked_weight = linked_index == -1 ? pawn_value : weights[linked_index];
            score += fV[i] * (linked_weight + weights[i]);
        } else {
            score += fV[i] * weights[i];
        }
    }

    return score;
}

// Read evolution.py to see explenation of these features
int ShogiFeatures::evaluate(Shogi s) {
    /* Evaluate the shogi position s from the perspective of root player (maximizer) */

    // Feature vector
    vector<int> fV = generate_feature_vec_raw(s);
    return evaluate_feature_vec(fV, weights);
}

// VISUALLY CHECKED
void ShogiFeatures::material(Shogi& s) {
    // Counts the number of pieces the current player has and returns them in the
    //      order of piece_strings class variable with counts of pieces that count
    //      as gold at the end.

    // First val in pair is current player count, second is opponent piece count
    map<string, pair<int, int>> piece_counts = {
        {"p", {0, 0}},  {"l", {0, 0}},  {"n", {0, 0}},  {"s", {0, 0}},  {"g", {0, 0}},
        {"b", {0, 0}},  {"r", {0, 0}},  {"+p", {0, 0}}, {"+l", {0, 0}}, {"+n", {0, 0}},
        {"+s", {0, 0}}, {"+b", {0, 0}}, {"+r", {0, 0}}
    };

    // Iterate through all 40 pieces available (i == piece number)
    for (int i = 0; i < 40; i++) {
        // Skip if the piece is in the hand
        if (s.gomaPos[i] == -1) continue;

        // Get possetion for that piece and if it is upgraded or not
        int piece_type = s.gomaKind[i];
        int id = gomakindID(piece_type);
        int upgrade = gomakindUP(piece_type);

        // Get string representation and add to appropriate count map and piece position cache
        string piece = piece_map[{id, upgrade}];
        if (id != KING) {
            if (gomakindChesser(piece_type) == player) {
                piece_counts[piece].first++;
                piece_pos[piece].first.push_back(s.gomaPos[i]);
            } else {
                piece_counts[piece].second++;
                piece_pos[piece].second.push_back(s.gomaPos[i]);
            }
        }
    }

    // Add the relevant pieces to the feature vector
    int gold_count = 0;
    for (auto& piece : piece_strings) {

        // Difference between player one and player two counts
        int diff = piece_counts[piece].first - piece_counts[piece].second;

        // Tally up all the pieces that move same as a gold if param specified
        if (piece == pawn) {
            pawn_count = diff;
        }
        else if (group_promotions and move_as_gold.count(piece)) {
            gold_count += diff;
        } else {
            /* string name = piece_strings_to_full[piece] + "_VALUE"; */
            string name = piece_strings_to_full[piece];
            string type = name.substr(0, name.find("_"));
            name += type == "PROMOTED" ? "_BONUS" : "_VALUE";

            features[name] = diff;
        }
    }

    // Add diff in gold pieces if we are grouping
    if (group_promotions) {
        string name = "GOLD_AND_EQV_VALUE";
        features[name] = gold_count;
    }
}

// VISUALLY CHECKED
void ShogiFeatures::king_safety(Shogi& s) {
    // NOTE : Maybe make return the ratio instead of raw cout (threats / defenders) etc
    // or (defenders / 8) or (defense / escape) or something else?
    // Set of features that represent the overall safety of the king

    // Get all of the valid squares surrounding the king
    int opponent = (player ^ 1);
    int king_pos = (player == SENTE) ?
                    s.gomaPos[s.SENTEKINGNUM] :
                    s.gomaPos[s.GOTEKINGNUM];

    // Find adjacent squares
    vector<int> adjacent = find_adjacent(king_pos);
    adjacent.push_back(king_pos);

    // Initialize our features
    int defenders = 0;               /* Same as 'thickness' */
    int escape_routes = 0;
    int threats = 0;

    // Loop through adj. sqrs and determine thickness, escape routes, threats
    for (auto& pos : adjacent) {
        // Skip if the position off the left or right edge of board
        if (pos != -1) {

            // Escape routes
            if (s.board[pos] == -1)
                escape_routes++;

            // Check both single and multi square moves
            threats += s.boardFixedAttacking[opponent][pos].size();
            threats += s.boardFlowAttacking[opponent][pos].size();

            defenders += s.boardFixedAttacking[player][pos].size();
            defenders += s.boardFlowAttacking[player][pos].size();
        }
    }

    // Add to our feature vector
    features["PLAYER_KING_DEFENDERS"] = defenders;
    features["PLAYER_KING_ESCAPE_ROUTES"] = escape_routes;
    features["PLAYER_KING_THREAT_PENALTY"] = -1 * threats;
}

void ShogiFeatures::material_in_hand(Shogi& s) {

    if (!in_hand_bonus) {
        total_pieces_in_hand(s);
        return;
    }

    int opponent = player ^ 1;

    vector<int> player_hand;
    vector<int> oppnent_hand;
    player_hand.reserve(in_hand_order.size());
    oppnent_hand.reserve(in_hand_order.size());


    // 0-7 are Sente piece in hand queues, 8-15 are Gote
    for (int i = 0; i < 8; i++) {
        int I = i + player * 8;
        player_hand[i] = s.gomaTable[I].size();
    }
    for (int i = 0; i < 8; i++) {
        int I = i + opponent * 8;
        oppnent_hand[i] = s.gomaTable[I].size();
    }

    for (size_t i = 0; i < in_hand_order.size(); i++) {
        string name = piece_strings_to_full[in_hand_order[i]] + "_IN_HAND_BONUS";
        features[name] = player_hand[i] - oppnent_hand[i];
    }
}

// VISUALLY CHECKED
void ShogiFeatures::total_pieces_in_hand(Shogi& s) {

    // int player = (s.round & 1);
    int piece_cnt = 0;

    // 0-7 are Sente piece in hand queues, 8-15 are Gote
    for (int i = 0; i < 8; i++) {
        int I = i + player * 8;
        piece_cnt += s.gomaTable[I].size();
    }

    features["PIECES_IN_HAND"] = piece_cnt;
}

// VISUALLY CHECKED
void ShogiFeatures::controlled_squares(Shogi& s) {

    // Initialize opponent based on perspective of the heuristic
    int opponent = (player ^ 1);

    // Initialize the camps based on the player
    vector<int>& home_camp = (player == SENTE) ? sente_camp : gote_camp;
    vector<int>& oppn_camp = (player == SENTE) ? gote_camp  : sente_camp;

    // Find "in-camp-vulnerability", num of squares in home camp that are more
    // attacked than defended
    int vulnerable = 0;
    for (int pos : home_camp) {
        int piece = s.board[pos];
        if (piece == -1) continue; /* skip if empty */

        if (s.boardChesser[pos] == player) {
            int attackers = 0;
            int defenders = 0;

            attackers += s.boardFixedAttacking[opponent][pos].size();
            attackers += s.boardFlowAttacking[opponent][pos].size();
            defenders += s.boardFixedAttacking[player][pos].size();
            defenders += s.boardFlowAttacking[player][pos].size();

            if (attackers > defenders) {
                /* if (print) cout << "Vulnerable: " << pos << endl; */
                vulnerable++;
            }
        }
    }


    // Maybe include in camp freedom as another feature?
    /* int in_camp_freedom = 0; */
    /* for (int pos : home_camp) { */
    /*     int piece = s.board[pos]; */
    /*     if (piece == -1 /1* empty square *1/ */
    /*         && s.boardFixedAttacking[opponent][pos].size() == 0 */
    /*         && s.boardFlowAttacking[opponent][pos].size() == 0) { */
    /*             in_camp_freedom++; */
    /*         } */
    /* } */

    // Find "out-camp-attack", num of squares in opp camp that are
    //  current player is attacking more than the opponent is defending
    int attacking = 0;
    for (int pos : oppn_camp) {
        int piece = s.board[pos];
        if (piece == -1) continue; /* skip if empty */

        if (s.boardChesser[pos] == opponent) {
            int attackers = 0;
            int defenders = 0;

            attackers += s.boardFixedAttacking[player][pos].size();
            attackers += s.boardFlowAttacking[player][pos].size();
            defenders += s.boardFixedAttacking[opponent][pos].size();
            defenders += s.boardFlowAttacking[opponent][pos].size();

            if (attackers > defenders) {
                /* if (print) cout << "Attacking pos: " << pos << endl; */
                attacking++;
            }
        }
    }

    // Add results to the feature vector
    features["IN_CAMP_VULNERABILITY_PENALTY"] = -1 * vulnerable;
    features["OUT_CAMP_ATTACK"] = attacking;

    /* if (print and (attacking > 0 or vulnerable > 0)) { */
    /*   s.EasyBoardPrint(); */
    /*   cout << "perspective: " << player << endl; */
    /*   cout << "Attacking: " << attacking << endl; */
    /*   cout << "Vulnerable: " << vulnerable << endl; */
    /*   print_vec(home_camp_occupied); */
    /*   cout << endl; */
    /* } */

    /* print = false; */
}

// VISUALLY CHECKED
void ShogiFeatures::castle(Shogi& s) {

    map<string, string>& castles = (player == SENTE) ?
                                    black_castles : white_castles;

    int king_pos = (player == SENTE) ?
                s.gomaPos[s.SENTEKINGNUM] :
                s.gomaPos[s.GOTEKINGNUM];

    // Determine if we are within the threshold of at least one proper castle
    int closest_match = 0;
    /* string name = ""; */
    /* Shogi best_castle; */
    /* int best_castle_size = 0; */
    for (auto& entry : castles) {
        auto board = entry.second;

        // Ineffecient, but initialize a shogi object of the castle
        Shogi c;
        c.Init();
        c.LoadGame(load_hex_vector(board));

        int castle_king = (player == SENTE) ?
            c.gomaPos[c.SENTEKINGNUM] :
            c.gomaPos[c.GOTEKINGNUM];


        // Only consider a potential castle formation if player's king in correct spot
        if (king_pos != castle_king) continue;

        // See if other pieces are within the threshold
        int correct = 0;
        int total_castle_pieces = 0;
        for (int pos = 0; pos < 81; pos++) {

            // Skip if castle square empty
            if (c.board[pos] == -1) continue;
            total_castle_pieces++;

            // Ignore king position in count since checked above
            if (pos == castle_king) continue;

            // Skip if castle has a piece at square but player's is empty
            if (s.board[pos] == -1) continue;

            // Otherwise player has a piece in same square as castle, check if same piece
            int castle_gomaNum = c.board[pos];
            int player_gomaNum = s.board[pos];
            int castle_gomaKind = c.gomaKind[castle_gomaNum];
            int player_gomaKind = s.gomaKind[player_gomaNum];

            if (player_gomaKind == castle_gomaKind)
                correct++;
        }

        // See if current castle better than best so far
        if (correct > closest_match) {
            closest_match = correct;
            /* name = entry.first; */
            /* best_castle_size = total_castle_pieces; */
        }
    }

    /* if (closest_match > 0 and print) { */
    /*     s.EasyBoardPrint(); */
    /*     cout << "--- " << name << " ---" << endl; */
    /*     Shogi c; */
    /*     c.Init(); */
    /*     c.LoadGame(load_hex_vector(castles[name])); */
    /*     c.EasyBoardPrint(); */

    /*     cout << "Correct: " << closest_match << endl; */
    /*     cout << "Castle Size: " << best_castle_size << endl; */
    /* } */

    // Add the number of matching pieces in the closest castle formation to feature vector
    features["CASTLE_FORMATION"] =  closest_match;
}

// Penalty features for bad shape
void ShogiFeatures::gold_ahead_silver_penalty(Shogi& s) {

    // Check each of the player's silver pieces
    int count = 0;
    for (int pos : piece_pos["s"].first) {

        // Get piece above the silver
        vector<int> adjacent = find_adjacent(pos);
        int head_pos = adjacent[top];

        if (head_pos != -1) {
            int head_kind = s.gomaKind[s.board[head_pos]];
            int id = gomakindID(head_kind);

            if (id == GOLD and s.boardChesser[head_pos] == player) {
                count += 1;
            }
        }
    }

    features["GOLD_AHEAD_SILVER_PENALTY"] = -1 * count;
}

void ShogiFeatures::gold_adjacent_rook_penalty(Shogi& s) {

    // Also check for promoted rooks
    vector<int> all_rooks = piece_pos["r"].first;
    all_rooks.insert(all_rooks.end(), piece_pos["+r"].first.begin(), piece_pos["+r"].first.end());

    int count = 0;
    for (int pos : all_rooks) {

      vector<int> adj = find_adjacent(pos);

      int left_g = (adj[left] != -1 and s.boardChesser[adj[left]] == player and
              gomakindID(s.gomaKind[s.board[adj[left]]]) == GOLD) ? 1 : 0;
      int right_g = (adj[right] != -1 and s.boardChesser[adj[right]] == player and
              gomakindID(s.gomaKind[s.board[adj[right]]]) == GOLD) ? 1 : 0;
      int bot_g = (adj[bot] != -1 and s.boardChesser[adj[bot]] == player and
              gomakindID(s.gomaKind[s.board[adj[bot]]]) == GOLD) ? 1 : 0;
      int top_g = (adj[top] != -1 and s.boardChesser[adj[top]] == player and
              gomakindID(s.gomaKind[s.board[adj[top]]]) == GOLD) ? 1 : 0;

      count += left_g + right_g + bot_g + top_g;
    }

    features["GOLD_ADJACENT_ROOK_PENALTY"] = -1 * count;
}

void ShogiFeatures::boxed_in_bishop_penalty(Shogi& s) {

    // Also check for promoted rooks
    vector<int> all_bishops = piece_pos["b"].first;
    all_bishops.insert(all_bishops.end(), piece_pos["+b"].first.begin(), piece_pos["+b"].first.end());

    int boxed_corners = 0;
    for (int pos : all_bishops) {

        vector<int> adj = find_adjacent(pos);
        int top_l_corner = (adj[topL] != -1 and s.board[adj[topL]] != -1 and
                s.boardChesser[adj[topL]] == player) ? 1 : 0;
        int top_r_corner = (adj[topR] != -1 and s.board[adj[topR]] != -1 and
                s.boardChesser[adj[topR]] == player) ? 1 : 0;
        int bot_l_corner = (adj[botL] != -1 and s.board[adj[botL]] != -1 and
                s.boardChesser[adj[botL]] == player) ? 1 : 0;
        int bot_r_corner = (adj[botR] != -1 and s.board[adj[botR]] != -1 and
                s.boardChesser[adj[botR]] == player) ? 1 : 0;

        boxed_corners = top_l_corner + top_r_corner + bot_l_corner + bot_r_corner;
    }

    features["BOXED_IN_BISHOP_PENALTY"] = -1 * boxed_corners;
}

void ShogiFeatures::piece_ahead_of_pawns_penalty(Shogi& s) {
    vector<int> pawns = piece_pos["p"].first;

    int ahead_of_pawn_count = 0;
    for (int pos : pawns) {
        vector<int> adj = find_adjacent(pos);

        // As long as it is not a silver since reclining is good
        if (adj[top] != -1 and s.boardChesser[adj[top]] == player
                and gomakindID(s.gomaKind[s.board[adj[top]]]) != SILVER) {
            ahead_of_pawn_count += 1;
        }
    }

    features["PIECE_AHEAD_OF_PAWN_PENALTY"] = -1 * ahead_of_pawn_count;
}

// Features for GOOD shape
void ShogiFeatures::bishop_head_protected(Shogi& s) {
    vector<int> bishops = piece_pos["b"].first;

    int heads_protected = 0;
    for (int pos : bishops) {
        vector<int> adj = find_adjacent(pos);

        if (adj[top] != -1) {
            // Check if head is being defended
            if (s.boardFixedAttacking[player][pos].size()) {
                heads_protected += 1;
            } else if (s.boardFlowAttacking[player][pos].size()){
                heads_protected += 1;
            }
        }
    }

    features["BISHOP_HEAD_PROTECTED"] = heads_protected;
}

void ShogiFeatures::reclining_silver(Shogi& s) {
    vector<int> silvers = piece_pos["s"].first;

    int reclining = 0;
    for (int pos : silvers) {
        vector<int> adj = find_adjacent(pos);

        int pawn_right = (adj[right] != -1 and s.boardChesser[adj[right]] == player and
                gomakindID(s.gomaKind[s.board[adj[right]]]) == PAWN) ? 1 : 0;
        int pawn_left = (adj[left] != -1 and s.boardChesser[adj[left]] == player and
                gomakindID(s.gomaKind[s.board[adj[left]]]) == PAWN) ? 1 : 0;
        int pawn_bot = (adj[bot] != -1 and s.boardChesser[adj[bot]] == player and
                gomakindID(s.gomaKind[s.board[adj[bot]]]) == PAWN) ? 1 : 0;

        // Count both orientations fo the chair shape
        if (pawn_bot and pawn_right) {
            reclining += 1;
        } else if (pawn_bot and pawn_left) {
            reclining += 1;
        }
    }


    features["RECLINING_SILVER"] = reclining;
}

void ShogiFeatures::claimed_files(Shogi& s) {
    vector<int> pawns = piece_pos["p"].first;

    int claimed = 0;
    for (int pos : fifth_rank) {
        int piece = s.board[pos];
        if (piece == -1) continue; /* skip if empty */

        // Check if piece on fifth rank is current player's pawn
        if (s.boardChesser[pos] == player and gomakindID(s.gomaKind[piece]) == PAWN) {

            // Ensure that the square is defended
            if (s.boardFixedAttacking[player][pos].size() or s.boardFlowAttacking[player][pos].size()) {
                claimed++;
            }
        }
    }

    features["CLAIMED_FILES"] = claimed;
}

void ShogiFeatures::adjacent_silvers(Shogi& s) {
    features["ADJACENT_SILVERS"] = count_adj_pairs("s", s);
}

void ShogiFeatures::adjacent_golds(Shogi& s) {
    features["ADJACENT_GOLDS"] = count_adj_pairs("g", s);
}

void ShogiFeatures::rook_enemy_camp(Shogi& s) {
    vector<int> all_rooks = piece_pos["r"].first;
    all_rooks.insert(all_rooks.end(), piece_pos["+r"].first.begin(), piece_pos["+r"].first.end());

    int count = 0;
    for (int pos : all_rooks) {
        int file = posDan(pos);
        count += (player == SENTE and file < 4) ? 1 : 0;
        count += (player == GOTE and file > 6) ? 1 : 0;
    }

    features["ROOK_ENEMY_CAMP"] = count;
}

void ShogiFeatures::rook_attack_king_file(Shogi& s) {
    vector<int> all_rooks = piece_pos["r"].first;
    all_rooks.insert(all_rooks.end(), piece_pos["+r"].first.begin(), piece_pos["+r"].first.end());
    int oppn_king = (player == SENTE) ? s.gomaPos[s.GOTEKINGNUM] : s.gomaPos[s.SENTEKINGNUM];

    int count = 0;
    for (int pos : all_rooks) {
        count += posSuji(pos) == posSuji(oppn_king) ? 1 : 0;
    }

    features["ROOK_ATTACK_KING_FILE"] = count;
}

void ShogiFeatures::rook_attack_king_adj_file(Shogi& s) {
    vector<int> all_rooks = piece_pos["r"].first;
    all_rooks.insert(all_rooks.end(), piece_pos["+r"].first.begin(), piece_pos["+r"].first.end());
    int oppn_king = (player == SENTE) ? s.gomaPos[s.GOTEKINGNUM] : s.gomaPos[s.SENTEKINGNUM];

    int count = 0;
    for (int pos : all_rooks) {
        int diff = posSuji(pos) - posSuji(oppn_king);
        count += abs(diff) == 1 ? 1 : 0;
    }

    features["ROOK_ATTACK_KING_ADJ_FILE"] = count;
}

void ShogiFeatures::rook_attack_king_adj_file_9821(Shogi& s) {
    vector<int> all_rooks = piece_pos["r"].first;
    all_rooks.insert(all_rooks.end(), piece_pos["+r"].first.begin(), piece_pos["+r"].first.end());
    int oppn_king = (player == SENTE) ? s.gomaPos[s.GOTEKINGNUM] : s.gomaPos[s.SENTEKINGNUM];
    int king_suji = posSuji(oppn_king);

    int count = 0;
    for (int pos : all_rooks) {
        int diff = posSuji(pos) - king_suji;

        // See if king boxed in on left side
        if (king_suji == 9 or king_suji == 8) {
            count += king_suji - posSuji(pos) == 1 ? 1 : 0;
        }
        // See if king boxed in on the right side
        else if (king_suji == 2 or king_suji == 1) {
            count += posSuji(pos) - king_suji == 1 ? 1 : 0;
        }
    }

    features["ROOK_ATTACK_KING_ADJ_FILE_9821"] = count;
}

void ShogiFeatures::rook_open_semi_open_file(Shogi& s) {
    vector<int> all_rooks = piece_pos["r"].first;
    all_rooks.insert(all_rooks.end(), piece_pos["+r"].first.begin(), piece_pos["+r"].first.end());

    int open_count = 0, semi_open = 0, owned = 0;
    for (int rook : all_rooks) {
        int on_file = 0;
        int rook_file = posSuji(rook);

        // See if that file is completely open or has only 1 piece on it
        for (int dan = 1; dan <= 9; dan++) {
            int pos = genPos(rook_file, dan);
            if (s.board[pos] != -1 and pos != rook) {
                on_file += 1;

                // Keep track if it is player's piece
                if (s.boardChesser[pos] == player) {
                    owned += 1;
                }
            }
        }

        open_count += on_file == 0 ? 1 : 0;
        semi_open += (on_file == 1 and !owned) ? 1 : 0;
    }

    features["ROOK_OPEN_FILE"] = open_count;
    features["ROOK_SEMI_OPEN_FILE"] = semi_open;
}

void ShogiFeatures::bishop_mobility(Shogi& s) {
    vector<int> squares = find_flow_moves("b", s);
    int safe = count_safe_squares(squares, s);
    features["BISHOP_MOBILITY"] = safe;
}

void ShogiFeatures::rook_mobility(Shogi& s) {
    vector<int> squares = find_flow_moves("r", s);
    int safe = count_safe_squares(squares, s);
    features["ROOK_MOBILITY"] = safe;
}

void ShogiFeatures::blocked_flow(Shogi& s) {
    // Count how much of the opponent's flow player is blocking, should help with drop pieces

    int opponent = player ^ 1;

    int blocked = 0;
    for (int pos = 0; pos < 81; pos++) {
        blocked += s.boardBFlowAttacking[opponent][pos].size();
    }

    features["BLOCKED_FLOW"] = blocked;
}


void ShogiFeatures::aggression_balance(Shogi& s) {
    double player_agro = 0;
    double oppn_agro = 0;
    for (int i = 0; i < 40; i++) {
        if (s.gomaPos[i] == -1) continue;
        if (gomakindChesser(s.gomaKind[i]) == player) {
            player_agro += (10 - posDan(s.gomaPos[i])) / 9;
        } else {
            oppn_agro += (10 - posDan(s.gomaPos[i])) / 9;
        }
    }

    features["AGGRESSION_BALANCE"] = (int)(player_agro - oppn_agro);
}


void ShogiFeatures::king_attack(Shogi& s) {
    int opponent = (player ^ 1);
    int enemy_king = (player == SENTE) ?
                    s.gomaPos[s.GOTEKINGNUM] :
                    s.gomaPos[s.SENTEKINGNUM];

    // Look at all of player's pieces attacking opponent's king
    // Look at safe attacks on the king and on the surrounding squares

    int num_attacks = 0, num_safe_attacks = 0;
    vector<int> adjacent = find_adjacent(enemy_king);
    adjacent.push_back(enemy_king);

    // Look enemy king position and surrounding squares
    for (int pos : adjacent) {
        // Look at all of the static attacks on that position and see if safe
        for (int piece : s.boardFixedAttacking[player][enemy_king]) {
            int attacker = watchupAttacker(piece);
            int gomakind = s.gomaKind[attacker];
            int attack_pos = s.gomaPos[attacker];

            // Safe if we have backup, another friendly covering that square
            int safe = s.boardFixedAttacking[player][attack_pos].size() +
                       s.boardFlowAttacking[player][attack_pos].size();

            num_safe_attacks = safe ? 1 : 0;
            num_attacks += 1;
        }
        // Look at all of the long range attacks on that position and see if safe
        for (int piece : s.boardFlowAttacking[player][enemy_king]) {
            int attacker = watchupAttacker(piece);
            int gomakind = s.gomaKind[attacker];
            int attack_pos = s.gomaPos[attacker];

            // Safe if we have backup, another friendly covering that square
            int safe = s.boardFixedAttacking[player][attack_pos].size() +
                       s.boardFlowAttacking[player][attack_pos].size();

            num_safe_attacks = safe ? 1 : 0;
            num_attacks += 1;
        }
    }

    features["ENEMY_KING_ATTACKS"] = num_attacks;
    features["ENEMY_KING_ATTACKS_SAFE"] = num_safe_attacks;
}

void ShogiFeatures::total_attacking(Shogi& s) {
    int opponent = (player ^ 1);
    int player_squares = 0, opponent_squares = 0;
    for (int pos = 0; pos < 81; pos++) {
        if (s.boardFixedAttacking[player][pos].size() !=0) {
            // Count number of pieces attacking that square
            for (int piece : s.boardFixedAttacking[player][pos]) {
                int attacker = watchupAttacker(piece);
                int gomakind = gomakindID(s.gomaKind[attacker]);
                player_squares += 1;
            }
        }
        if (s.boardFlowAttacking[player][pos].size() !=0) {
            // Count number of pieces attacking that square
            for (int piece : s.boardFixedAttacking[player][pos]) {
                int attacker = watchupAttacker(piece);
                int gomakind = gomakindID(s.gomaKind[attacker]);
                player_squares += 1;
            }
        }
        if (s.boardFixedAttacking[opponent][pos].size() !=0) {
            // Count number of pieces attacking that square
            for (int piece : s.boardFixedAttacking[player][pos]) {
                int attacker = watchupAttacker(piece);
                int gomakind = gomakindID(s.gomaKind[attacker]);
                opponent_squares += 1;
            }
        }
        if (s.boardFlowAttacking[opponent][pos].size() !=0) {
            // Count number of pieces attacking that square
            for (int piece : s.boardFixedAttacking[player][pos]) {
                int attacker = watchupAttacker(piece);
                int gomakind = gomakindID(s.gomaKind[attacker]);
                opponent_squares += 1;
            }
        }
    }

    features["TOTAL_ATTACKING"] = player_squares - opponent_squares;
}

void ShogiFeatures::distance_to_kings(Shogi& s) {
    // Measure the general distance each piece has to go to the enemy king
    int opponent = (player ^ 1);
    int player_king = (player == SENTE) ?
                    s.gomaPos[s.SENTEKINGNUM] :
                    s.gomaPos[s.GOTEKINGNUM];
    int enemy_king = (player == SENTE) ?
                    s.gomaPos[s.GOTEKINGNUM] :
                    s.gomaPos[s.SENTEKINGNUM];


    int player_dist = 0, opponent_dist = 0;
    // Look at how far each piece is away from opponnet king
    for (int i = 0; i < 40; i++) {
        if (s.gomaPos[i] == -1) continue;
        if (gomakindChesser(s.gomaKind[i]) == player) {
            player_dist += distance(s.gomaPos[i], enemy_king);
        } else {
            opponent_dist += distance(s.gomaPos[i], player_king);
        }
    }

    features["DISTANCE_TO_KINGS"] = player_dist - opponent_dist;
}

/* Helper functions */
int ShogiFeatures::distance(int posA, int posB){
	int asuji = posSuji(posA);
	int adan = posDan(posA);
	int bsuji = posSuji(posB);
	int bdan = posDan(posB);
	return ((asuji - adan)*(asuji - adan) + (bsuji - bdan)*(bsuji - bdan)) / 10;
}

vector<int> ShogiFeatures::find_adjacent(int pos) {
    // Inialize the vector
    vector<int> adjacent(8, -1);


    // Add adacent squares above pos as long as not in topmost row
    if (pos % 9 != 0) {
        int top_pos = pos - 1;
        int top_r_pos = top_pos - 9;
        int top_l_pos = top_pos + 9;

        adjacent[top] = top_pos;
        adjacent[topL] = in_bounds(top_l_pos) ? top_l_pos : -1;
        adjacent[topR] = in_bounds(top_r_pos) ? top_r_pos : -1;
    }

    // Add adjacent pieces below the king as long as not in bottom most row
    if (pos % 9 != 8) {
        int bot_pos = pos + 1;
        int bot_r_pos = bot_pos - 9;
        int bot_l_pos = bot_pos + 9;

        adjacent[bot] = bot_pos;
        adjacent[botL] = in_bounds(bot_l_pos) ? bot_l_pos : -1;
        adjacent[botR] = in_bounds(bot_r_pos) ? bot_r_pos : -1;
    }

    // Add the left and right pieces if in bounds
    adjacent[left] = in_bounds(pos + 9) ? pos + 9 : -1;
    adjacent[right] = in_bounds(pos - 9) ? pos - 9 : -1;

    return adjacent;
}

int ShogiFeatures::count_adj_pairs(string piece_type, Shogi& s) {
    vector<int> pieces = piece_pos[piece_type].first;

    // Map to insure not coutned twice
    map<int, int> seen;
    int adj_pair = 0;
    for (int pos : pieces) {

        // Skip if we already counted this position as adjacent to another silver
        if (seen.count(pos)) continue;

        vector<int> adj = find_adjacent(pos);

        int piece_left = (adj[left] != -1 and s.boardChesser[adj[left]] == player and
                gomakindID(s.gomaKind[adj[left]]) == SILVER) ? 1 : 0;
        int piece_right = (adj[right] != -1 and s.boardChesser[adj[right]] == player and
                gomakindID(s.gomaKind[adj[right]]) == SILVER) ? 1 : 0;

        if (piece_left) {
            adj_pair += 1;
            seen[adj[left]] = 1;
        } else if (piece_right) {
            adj_pair += 1;
            seen[adj[right]] = 1;
        }
    }

    return adj_pair;
}

void ShogiFeatures::print_piece_map() {
    string curr = player == SENTE ? "Sente" : "Gote";
    string opp = player == SENTE ? "Gote" : "Sente";
    for (auto& entry : piece_pos) {
        cout << "--- " << entry.first << " ---" << endl;
        cout << "     " << curr << ": ";
        print_vec(entry.second.first);
        cout << "     " << opp << ": ";
        print_vec(entry.second.second);
    }
}

int ShogiFeatures::count_safe_squares(vector<int> squares, Shogi& s)  {
    int opp = player ^ 1;
    vector<int> safe = {};
    for (int pos : squares) {
        if (!s.boardFixedAttacking[opp][pos].size() and !s.boardFlowAttacking[opp][pos].size()) {
            safe.push_back(pos);
        }
    }

    return safe.size();
}

vector<int> ShogiFeatures::find_flow_moves(string piece_type, Shogi& s) {

    string up_piece = "+" + piece_type;
    vector<int> pieces = piece_pos[piece_type].first;
    pieces.insert(pieces.end(), piece_pos[up_piece].first.begin(), piece_pos[up_piece].first.end());

    vector<int> squares = {};
    for (int pos : pieces) {

        // Attempt at getting rook movement
        int piece_num = s.board[pos];
        int gomaKind = s.gomaKind[piece_num];

        // Eid is the id for the kind of movement the piece can make
        int up = gomakindUP(gomaKind);
        int eid = gomakindEID(gomaKind);
        int owner = gomakindChesser(gomaKind);
		    int danReverse = (owner == SENTE) ? 1 : -1;

        for (int v = 0; v < movingDlength[eid]; v++) {
            int prePos = pos;
            int preSuji = posSuji(prePos);
            int preDan = posDan(prePos);

            // 0 meanns it is a corner move
            if (movingD[eid][v] == 0) {
                int newSuji = preSuji + sujiD[eid][v];
                int newDan = preDan + danD[eid][v] * danReverse;
                int newPos = genPos(newSuji, newDan);
                if(newPos == -1 or s.boardChesser[newPos] == owner) continue;

                squares.push_back(newPos);
            }
            // 1 means upgraded direction (left, right, up, down) for bishop, (topL, topR, botL, botR) for rook
            else if (movingD[eid][v] == 1){

                // Move in each direction until fall off the board
				        int step = 1;
                bool expanding = true;
                bool previous_capture = false;
                while (true) {
                  int newSuji = preSuji + sujiD[eid][v] * step;
                  int newDan = preDan + danD[eid][v] * danReverse * step;
                  int newPos = genPos(newSuji, newDan);

                  // Stop if new position off the board or blocked by friendly piece
                  if (newPos == -1 or s.boardChesser[newPos] == owner) break;

                  // Add square
                  squares.push_back(newPos);

                  // Stop advance if there was an enemy piece on that square
                  if (s.boardChesser[newPos] != -1) break;

                  step++;
                }
            }
        }
    }
    return squares;
}

