// Legal moves cache 

// Reads a json file of [board, (action, outcome)] pairs into
//  a cpp map 
#include "lmcache.hpp"

// Prettify a given json in_file and save it to out_file
void prettify(string in_file, string out_file) {

    std::ifstream fin(in_file);

    json j;
    fin >> j;

    std::ofstream out(out_file);
    out << std::setw(4) << j << std::endl;
}


void MovesCache::Init(string file) {
    ifstream fin(file);
    
    json j;
    fin >> j;

    for (auto& item : j) {
        
        vector<pair<int, string>>actions;
        for (auto& action : item["actions"]) {
            actions.push_back(action);
        }

        legal_moves[item["board"]] = actions;
    }
}

void MovesCache::Print() {
    for (auto& item : legal_moves) {
        cout << "board: " << item.first.substr(0, 25) << "..." << endl;
        for (auto& action : item.second) {
            cout << "   " << action.first << ", " << action.second.substr(0, 20) << "..." << endl;
        }
        cout << endl;
    } 
}



// Run this main code if you make lmcache. Prettifies the json code and prints
//      the resulting cache
// int main() {
//     // read a JSON file
    
//     string lmcache = "../json/legal_moves_cache_raw.json";
//     string test_in = "../json/test_data_raw.json";
//     string train_in = "../json/train_data_raw.json";

//     // // Uncomment to prettify the raw json
//     prettify(lmcache, "../json/legal_moves_cache.json");
//     prettify(test_in, "../json/test_data.json");
//     prettify(train_in, "../json/train_data.json");


//     // Map of board, (move, resulting_pos)
//     MovesCache cache;
//     cache.Init(lmcache);
//     cache.Print();

//     return 0;
// }







