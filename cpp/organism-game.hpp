#pragma once
#include "game.hpp"
#include "gshogi-agent.hpp"

// Class for a game between two organisms with given weights used
// to interface with python evolutionary algorithm

class OrganismGame  {

    public:
        // Indvidual 1 always becomes the sente
        OrganismGame(vector<int> indv1, vector<int> indv2, int max_round, int max_search_depth);

        // Return the results of simulating a game between two individuals
        int simulate();

    private:
        int senteColor = 0;
        int goteColor = 1;
        int max_round;
        Agent* sente;
        Agent* gote;
};
