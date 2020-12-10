#pragma once
#include "shogi.hpp"

using namespace std;

class Agent {
	private:
		Shogi s; // the board the player is playing
		bool color; // color of the player
	protected:
		Shogi getBoard();
		void setColor(bool);
	public:
    // Virtual because eventually want to make another class allowing human to
    // interact via command line
		virtual int getMove()=0;
		bool getColor();
		void setBoard(Shogi board);
};
