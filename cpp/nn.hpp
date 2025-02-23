#pragma once
#include <fstream>
#include <iostream>

constexpr int INPUT_SIZE = 768; // 8x8x12 for chess pieces
constexpr int HL_SIZE = 1024;   // Hidden layer size

// Fixed network structure.
struct Network {
	// Hidden (accumulator) layer weights and biases.
    float hidden_weights[INPUT_SIZE][HL_SIZE];
    float hidden_biases[HL_SIZE];
	// Output layer weights and bias.
    float output_weights[HL_SIZE];
    float output_bias;
    
	// Stored activations from the last forward pass.
    bool last_input[INPUT_SIZE];
    float hidden_zs[HL_SIZE];
    float hidden[HL_SIZE];
    float output_z;
    float output;

    void save(std::ofstream& out) {
        out.write((char*)hidden_weights, sizeof(hidden_weights));
        out.write((char*)hidden_biases, sizeof(hidden_biases));
        out.write((char*)output_weights, sizeof(output_weights));
        out.write((char*)&output_bias, sizeof(output_bias));
    }

    void load(std::ifstream& in) {
        in.read((char*)hidden_weights, sizeof(hidden_weights));
        in.read((char*)hidden_biases, sizeof(hidden_biases));
        in.read((char*)output_weights, sizeof(output_weights));
        in.read((char*)&output_bias, sizeof(output_bias));
    }
};

float forward(Network& net, const bool input[INPUT_SIZE]);
float forward_flipidx(Network& net, const int idx);
void backward(Network& net, float target, float learning_rate);