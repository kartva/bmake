#include "util.hpp"
#include "lua_interface.hpp"
#include "server_io.hpp"

#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

int main(int argc, char** argv) {
	stringstream ss;
	for (int i=1; i<argc; i++) ss<<argv[i]<<" ";

	// Command line arguments:
	// play <lua_path> <weights_path> (play with weights from <weights_path>)
	// train <lua_path> <weights_path> (save weights to <weights_path>)

	string ty; ss >> ty;
	string lua_path; ss >> lua_path;
	string weights_path; ss >> weights_path;

	ifstream lua_input_stream(lua_path);
	if (!lua_input_stream.is_open()) {
		cerr << "Error: Cannot open file: " << lua_path << endl;
		return 1;
	}
	stringstream content; content << lua_input_stream.rdbuf();
	string lua = content.str();

	if (ty == "play") {
		ifstream weights_input_stream(weights_path);
		if (!weights_input_stream.is_open()) {
			cerr << "Error: Cannot open file: " << weights_path << endl;
			return 1;
		}
		
		LuaInterface lua(lua_path);
        intro(lua);
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
	} else if (ty == "train") {
		ofstream weights_input_stream(weights_path);
		if (!weights_input_stream.is_open()) {
			cerr << "Error: Cannot open file: " << weights_path << endl;
			return 1;
		}
	} else {
		cerr << "Usage: play|train <lua_path> <weights_path>" << endl;
		return 1;
	}
}