#include "shogi.hpp"

int posSuji(int pos);
int posDan(int pos);
int genPos(int suji, int dan);
int genGomakind(int id, int upgrade, int chesser);
int gomakindID(int gomakind);
int gomakindUP(int gomakind);
int gomakindEID(int gomakind);
int gomakindChesser(int gomakind);
int genUPos(int chesser, int gomaid);
int genMove(int prePos, int newPos, int upgrade, int playing);
int movePrepos(int move);
int moveNewpos(int move);
int moveUpgrade(int move);
int movePlaying(int move);
int genWatchup(int blocker, int attacker);
int watchupBlocker(int watchup);
int watchupAttacker(int watchup);
void print_vec(vector<int> vec);
vector<unsigned char> load_hex_vector(string hex);
void printMove(int move);
static string IDPRES[8] = {"Foot", "Sliver", "Cassia", "Chariot", "Flying", "Angle", "King", "Gold"};
