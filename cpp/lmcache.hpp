#pragma once
#include "json.hpp" // Credit to nlohmann/json.hpp
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <iomanip>

// for convenience
using json = nlohmann::json;
using namespace std;

void prettify(string in_file, string out_file);


class MovesCache {
    public:
       unordered_map<string, vector<pair<int, string>>> legal_moves;
       void Init(string file);
       void Print();
};
