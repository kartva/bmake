#pragma once

#include <iostream>
#include <map>
#include "lua_interface.hpp"
#include "util.hpp"

using namespace std;

struct ServerIO {
	vector<int> out;

	void flush() {
		for (int x: out) cout<<x<<" ";
		cout<<endl;
		cout.flush();
		out.clear();
	}

	Coord receive_coord() {
		Coord coord;
		int i, j;
		cin >> i >> j;
		coord.i = i;
		coord.j = j;

		return coord;
	}

	void send_coord(const Coord& coord) {
		out.insert(out.end(), {coord.i, coord.j});
	}

	void send_board(const unsigned char* board, int n, int m) {
		out.insert(out.end(), {n,m});
		for (int i = 0; i < n*m; i++) out.push_back(board[i]);
	}

	void receive_board(unsigned char* board) {
		int n,m; cin>>n>>m;
		for (int i = 0; i < n*m; i++) {
			int val;
			cin >> val;
			board[i] = (unsigned char)val;
		}
	}

	void send_pos(const Position& pos, int n, int m) {
		out.push_back(pos.next_player);
		send_board(pos.board,n,m);
	}

	Position receive_pos() {
		Position pos;
		cin >> pos.next_player;
		receive_board(pos.board);
		return pos;
	}

	void send_move(const Move& move,int n,int m) {
		send_coord(move.from);
		send_coord(move.to);
		send_board(move.board,n,m);
	}

	Move receive_move() {
		Move move;
		move.from = receive_coord();
		move.to = receive_coord();
		receive_board(move.board);
		return move;
	}
};
