#include <iostream>
#include <cmath>
#include <random>
using namespace std;

const int INPUT_SIZE = 768;
const int HL_SIZE = 1024;

// Fixed network structure.
struct Network {
    // Hidden (accumulator) layer weights and biases.
    float accumulator_weights[INPUT_SIZE][HL_SIZE]; // [INPUT_SIZE x HL_SIZE]
    float accumulator_biases[HL_SIZE];                // [HL_SIZE]

    // Output layer weights and bias.
    float output_weights[HL_SIZE]; // [HL_SIZE]
    float output_bias;             // single value

    // Stored activations from the last forward pass.
    float last_input[INPUT_SIZE];  // input vector
    float hidden[HL_SIZE];         // activated values for hidden layer
    float hidden_zs[HL_SIZE];      // pre-activation values for hidden layer
    float output_z;                // preactivation value for the output neuron
    float output;                  // final output after activation
};

float activation(float in) {
    return tanh(in);
}

float activation_prime(float z) {
    float t = tanh(z);
    return 1.0f - t * t;
}

// -------- Forward Pass --------
// Computes and stores all intermediate values.
float forward(Network &net, const float input[INPUT_SIZE])
{
    // Store the input.
    for (int i = 0; i < INPUT_SIZE; i++) {
        net.last_input[i] = input[i];
    }

    // Compute hidden layer values.
    for (int j = 0; j < HL_SIZE; j++) {
        float z = net.accumulator_biases[j];
        for (int i = 0; i < INPUT_SIZE; i++) {
            z += input[i] * net.accumulator_weights[i][j];
        }
        net.hidden_zs[j] = z;
        net.hidden[j] = activation(z);
    }
    
    // Compute output preactivation: a weighted sum of hidden activations plus bias.
    float out_pre = net.output_bias;
    for (int j = 0; j < HL_SIZE; j++) {
        out_pre += net.hidden[j] * net.output_weights[j];
    }
    net.output_z = out_pre;
    // Apply the activation function to the output preactivation.
    net.output = activation(out_pre);
    return net.output;
}

// -------- Backward Pass --------
// Uses stored values from the forward pass.
// Uses Mean Squared Error (MSE) loss: L = (output - target)^2.
void backward(Network &net, float target, float learning_rate)
{
    // Compute gradient with respect to the output neuron's preactivation.
    // dL/d(output) = 2*(output - target) and since output = activation(z),
    // dL/dz = 2*(output - target)*activation_prime(z)
    float dL_dz = 2.0f * (net.output - target) * activation_prime(net.output_z);

    // Update output layer weights and bias.
    for (int j = 0; j < HL_SIZE; j++) {
        net.output_weights[j] -= learning_rate * (net.hidden[j] * dL_dz);
    }
    net.output_bias -= learning_rate * dL_dz;

    // Backpropagate error into the hidden layer.
    // For each hidden neuron j:
    // dL/d(hidden_z[j]) = dL/dz * output_weights[j] * activation_prime(hidden_zs[j])
    float hidden_errors[HL_SIZE];
    for (int j = 0; j < HL_SIZE; j++) {
        hidden_errors[j] = dL_dz * net.output_weights[j] * activation_prime(net.hidden_zs[j]);
    }

    // Update hidden (accumulator) weights and biases.
    for (int j = 0; j < HL_SIZE; j++) {
        for (int i = 0; i < INPUT_SIZE; i++) {
            net.accumulator_weights[i][j] -= learning_rate * (net.last_input[i] * hidden_errors[j]);
        }
        net.accumulator_biases[j] -= learning_rate * hidden_errors[j];
    }
}

// -------- Example Usage --------
int main() {
    Network net;

    // Initialize random generator.
    mt19937 gen(random_device{}());
    
    // Set up distributions for weight initialization.
    // For hidden layer weights: standard deviation = sqrt(2/(INPUT_SIZE+HL_SIZE))
    normal_distribution<float> dist1(0.0f, sqrt(2.0f / (INPUT_SIZE + HL_SIZE)));
    // For output weights: standard deviation = sqrt(2/(1+HL_SIZE))
    normal_distribution<float> dist2(0.0f, sqrt(2.0f / (1 + HL_SIZE)));
    
    // Initialize hidden layer weights.
    for (int i = 0; i < INPUT_SIZE; i++) {
        for (int j = 0; j < HL_SIZE; j++) {
            net.accumulator_weights[i][j] = dist1(gen);
        }
    }
    // Initialize hidden biases and output weights.
    for (int j = 0; j < HL_SIZE; j++) {
        net.accumulator_biases[j] = 0.0f;
        net.output_weights[j] = dist2(gen);
    }
    net.output_bias = 0.0f;
    
    // Create dummy input vectors.
    float input1[INPUT_SIZE];
    float input2[INPUT_SIZE];
    for (int i = 0; i < INPUT_SIZE; i++) {
        input1[i] = 0.2f;
        input2[i] = 0.1f;
    }
    
    float learning_rate = 0.01f;
    for (int i = 0; i < 1000; i++) {
        float o1 = forward(net, input1);
        backward(net, 0.5f, learning_rate);
        cout << "Iteration " << i << " - Output for input1: " << o1 << endl;

        float o2 = forward(net, input2);
        backward(net, -0.5f, learning_rate);
        cout << "Iteration " << i << " - Output for input2: " << o2 << endl;
    }
    
    return 0;
}
