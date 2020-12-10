#include "organism-game.hpp"


OrganismGame::OrganismGame(vector<int> indv1, vector<int> indv2, int rounds, int max_search_depth) {
    sente = new GShogiAgent(senteColor, max_search_depth, indv1);
    gote = new GShogiAgent(goteColor, max_search_depth, indv2);
    max_round = rounds;
}

int OrganismGame::simulate() {
    Shogi s;
    s.Init();

    Game g(s, sente, gote);
    return g.play(max_round);
}
