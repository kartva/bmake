#pragma once

#ifndef BUILD_DEBUG
#include <mimalloc.h>
#include <mimalloc-new-delete.h>
#endif

#ifndef BUILD_DEBUG
#include <gtl/phmap.hpp>
#include <gtl/btree.hpp>
#include <gtl/vector.hpp>

template<class T>
using vec = gtl::vector<T, mi_stl_allocator<T>>;
#else
#include <vector>
template<class T>
using vec = std::vector<T>;
#endif

struct Coord {
	unsigned char i,j;
};

struct Piece {
	unsigned char type;
	Coord c;
	bool being_added;
};

// A position is a board with pieces on it.
// It represents a state of the game.
struct Position {
	vec<Piece> board;
};

// A move consists of:
// A piece at a coordinate being moved to another coordinate.
// A list of pieces being added or removed.
// Handle the case when from == to.
struct Move {
	Coord from, to;
	vec<Piece> add_remove;
};
