#pragma once
constexpr int INPUT_SIZE = 768;
constexpr int HL_SIZE = 1024;

// Fixed network structure.
struct Network {
	// Hidden (accumulator) layer weights and biases.
	float hidden_weights[INPUT_SIZE][HL_SIZE]; // [INPUT_SIZE x HL_SIZE]
	float hidden_biases[HL_SIZE];                // [HL_SIZE]

	// Output layer weights and bias.
	float output_weights[HL_SIZE]; // [HL_SIZE]
	float output_bias;             // single value

	// Stored activations from the last forward pass.
	bool last_input[INPUT_SIZE];  // input vector
	float hidden[HL_SIZE];         // activated values for hidden layer
	float hidden_zs[HL_SIZE];      // pre-activation values for hidden layer
	
	float output;                  // final output after activation
	float output_z;                // preactivation value for the output neuron
};