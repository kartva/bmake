#include "util.hpp"
#include "lua_interface.hpp"
#include "server_io.hpp"
#include "nn.hpp"
#include "search.hpp"

#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

void train_model(LuaInterface& lua, const string& weights_path) {
	
}

int main(int argc, char** argv) {
	stringstream ss;
	for (int i=1; i<argc; i++) ss<<argv[i]<<" ";

	cout << "Instructions: " << endl;
	cout << "<lua_path> <weights_path>" << endl;
	cout << "First enter: validate_lua" << endl;
	cout << "Then enter: start_training" << endl;
	cout << "Then enter: query_valid_moves <i> <j> (where <i> and <j> are the coordinates of the piece)" << endl;

	string lua_path; ss >> lua_path;
	string weights_path; ss >> weights_path;

	string line; cin >> line;

	LuaInterface lua(lua_path);
	try {
		lua.validate();
	} catch (LuaException& e) {
		cout<<"Lua error: "<<e.err<<endl;
		return 0;
	}

	cout << "validated" << endl;
	cout.flush();

	cin >> line;

	// loading or training model may take a while

	ifstream weights_input_stream(weights_path);
	if (!weights_input_stream.is_open()) {
		cerr << "Error: Cannot open file: " << weights_path << endl;
		return 1;
	}

	// if the weights file is empty, train the model instead
	// if (weights_input_stream.peek() == ifstream::traits_type::eof()) {
	// 	weights_input_stream.close();
	// 	// train model
	// }

	cout << "loaded" << endl;
	cout.flush();
	
	sendIntroInfo(lua);
	Position pos = lua.initial_position();
	cout.flush();

	// Process commands
	string command;
	while (cin >> command) {
		if (command == "query_valid_moves") {
			Coord c = receiveCoord();
			vec<Move> out;
			lua.valid_moves(out, pos);
			for (Move &m : out) {
				if (m.from == c) {
					sendMove(m);
				}
			}
			cout.flush();
		}
		else if (command == "move_select") {
			Move move = receiveMove();
			// pos.board <- move.board
			memcpy(pos.board, move.board, sizeof(pos.board));				

			pos.next_player++;
			
			// simple strategy: find the first valid move and perform it
			vec<Move> out;
			lua.valid_moves(out, pos);
			if (out.size() == 0) {
				cout << "no valid moves" << endl;
				// TODO: transmit defeat
			} else {
				sendMove(out[0]);
			}
			pos.next_player--;
			cout.flush();
		}
	}
}