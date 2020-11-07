#include "helper.hpp"

int posSuji(int pos){
	return (pos / 9) + 1;
}

int posDan(int pos){
	return (pos % 9) + 1;
}

int genPos(int suji, int dan){
	return (suji > 9 or suji < 1 or dan > 9 or dan < 1) ?
	-1 : (suji - 1) * 9 + (dan - 1);
}

int genGomakind(int id, int upgrade, int chesser){
	return id + upgrade*8 + chesser*16;
}

int gomakindID(int gomakind){
	return gomakind & 7;
}

int gomakindUP(int gomakind){
	return ((gomakind & 8) >> 3);
}

int gomakindEID(int gomakind){
	return gomakind & 15;
}

int gomakindChesser(int gomakind){
	return ((gomakind & 16) >> 4);
}

int genUPos(int chesser, int gomaid){
	return chesser * 8 + gomaid;
}

int genMove(int prePos, int newPos, int upgrade, int playing){
	return (prePos*81 + newPos)*4 + upgrade*2 + playing;
}

int movePrepos(int move){
	return (move / 4) / 81;
}

int moveNewpos(int move){
	return (move / 4) % 81;
}

int moveUpgrade(int move){
	return ((move & 2) >> 1);
}

int movePlaying(int move){
	return (move & 1);
}

int genWatchup(int blocker, int attacker){
	return (blocker+1)*40 + attacker;
}

int watchupBlocker(int watchup){
	return (watchup / 40) - 1;
}

int watchupAttacker(int watchup){
	return watchup % 40;
}

void print_vec(vector<int> vec) {
    cout << "[ ";
    for (auto& item : vec ){
        cout << item << ", ";
    }
    cout << "]" << endl;
}

vector<unsigned char> load_hex_vector(string hex) {
  vector<unsigned char> c;
  c.reserve(hex.length() / 2);
  for (int i = 0; i < hex.length(); i += 2) {
    char v = 0;
    v += (hex[i] - (hex[i] >= 'A' ? ('A' - 10) : ('0'))) * 16;
    v += (hex[i + 1] - (hex[i + 1] >= 'A' ? ('A' - 10) : ('0')));
    c.push_back(v);
  }
  return c;
}

void printMove(int move){
	int prePos = movePrepos(move);
	int newPos = moveNewpos(move);
	int upgrade = moveUpgrade(move);
	int playing = movePlaying(move);
	int preSuji = posSuji(prePos);
	int preDan = posDan(prePos);
	int newSuji = posSuji(newPos);
	int newDan = posDan(newPos);

	if(!playing){
		cout << "(" << preSuji << ", " << preDan 
		<< ") => (" << newSuji << ", " << newDan << ")";
		if(upgrade) cout << " upgrade";
		cout << "\n";
	}else{
		cout << "Playing " << IDPRES[prePos] << " at (" << newSuji << ", " << newDan << ")\n";
	}
	
}
