#include "util.hpp"
#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

int main(int argc, char** argv) {
	stringstream ss;
	for (int i=0; i<argc; i++) ss<<argv[i]<<" ";

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
	} else if (ty == "train") {
		ofstream weights_input_stream(weights_path);
		if (!weights_input_stream.is_open()) {
			cerr << "Error: Cannot open file: " << weights_path << endl;
			return 1;
		}
	}
}