#include "util.hpp"
#include "lua_interface.hpp"
#include "server_io.hpp"
#include "search2.hpp"

#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

int main(int argc, char** argv) {
	stringstream ss;
	for (int i=1; i<argc; i++) ss<<argv[i]<<" ";

	// cout << "Instructions: " << endl;
	// cout << "<lua_path> <weights_path>" << endl;
	// cout << "First enter: validate_lua" << endl;
	// cout << "Then enter: start_training" << endl;
	// cout << "Then enter: query_valid_moves <i> <j> (where <i> and <j> are the coordinates of the piece)" << endl;

	string ty; ss>>ty;
	string lua_path; ss >> lua_path;

	if (ty=="validate") {
		cout << "trying validate\n";
		try {
			LuaInterface lua(lua_path);
			lua.validate();
		} catch (LuaException& e) {
			cout<<"Lua error: "<<e.err<<endl;
			return 1;
		} catch (exception& e) {
			cout<<"Other exception"<<endl;
			return 1;
		}
	} else if (ty=="train") {
	} else if (ty=="play") {
		// string weights_path; ss >> weights_path;
		// ifstream weights_input_stream(weights_path);
		// if (!weights_input_stream.is_open()) {
		// 	cerr << "Error: Cannot open file: " << weights_path << endl;
		// 	return 1;
		// }

		ServerIO io;
		int n,m,npty; cin>>n>>m>>npty;

		Searcher search(npty, n, m, 1000, 0, lua_path);
		auto& lua = search.interfaces[0];
		vec<Move> moves;

		while (true) {
			int query_type; cin>>query_type;
			Position pos = io.receive_pos();

			if (query_type==0) {

				int pty;
				switch (lua.get_pos_type(pos)) {
					case PosType::Win: pty=1; break;
					case PosType::Draw: pty=0; break;
					case PosType::Loss: pty=-1; break;
					case PosType::Other: pty=-2; break;
				}

				io.out.push_back(pty);

				moves.clear();
				lua.valid_moves(moves, pos);
				io.out.push_back(moves.size());
				for (auto& move: moves) io.send_move(move,n,m);

			} else if (query_type==1) {

				auto search_out = search.search(pos);
				if (search_out.move_i==-1) return 1;
				io.send_move(search_out.possible[search_out.move_i], m, n);

			}

			io.flush();
		}
	} else {
		cerr<<"unrecognized command "<<ty<<endl;
		return 1;
	}

	// string command;
	// while (cin >> command) {
	// 	if (command == "query_valid_moves") {
	// 		Coord c = receiveCoord();
	// 		vec<Move> out;
	// 		lua.valid_moves(out, pos);
	// 		for (Move &m : out) {
	// 			if (m.from == c) {
	// 				sendMove(m);
	// 			}
	// 		}
	// 		cout.flush();
	// 	} else if (command == "move_select") {
	// 		Move move = receiveMove();
	// 		// pos.board <- move.board
	// 		memcpy(pos.board, move.board, sizeof(pos.board));				

	// 		pos.next_player++;
			
	// 		// simple strategy: find the first valid move and perform it
	// 		vec<Move> out;
	// 		lua.valid_moves(out, pos);
	// 		if (out.size() == 0) {
	// 			cout << "no valid moves" << endl;
	// 			// TODO: transmit defeat
	// 		} else {
	// 			sendMove(out[0]);
	// 		}
	// 		pos.next_player--;
	// 		cout.flush();
	// 	}
	// }
}