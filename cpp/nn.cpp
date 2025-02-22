#include "nn.hpp"  // Assumes nn.hpp defines INPUT_SIZE, HL_SIZE, and struct Network

#include <iostream>
#include <chrono>
#include <cmath>
#include <random>

using namespace std;

// float activation(float in) {
//     return tanh(in);
// }

// float activation_prime(float z) {
//     float t = tanh(z);
//     return 1.0f - t * t;
// }

float activation(float in) {
    if (in < -1) {
        return -1;
    } else if (in > 1) {
        return 1;
    } else {
        return in;
    }
}

float activation_prime(float z) {
    if (z < -1 || z > 1) {
        return 0;
    } else {
        return 1;
    }
}

int used_indices[INPUT_SIZE];

float forward(Network &net, const bool input[INPUT_SIZE]) {
    for (int i = 0; i < INPUT_SIZE; i++) {
        net.last_input[i] = input[i];
    }

    int used_indices_size = 0;
    for (int i = 0; i < INPUT_SIZE; i++) {
        if (input[i]) {
            used_indices[used_indices_size++] = i;
        }
    }

    // Compute hidden layer values. This is the costly portion.
    for (int j = 0; j < HL_SIZE; j++) {
        float z = net.hidden_biases[j];
        for (int k = 0; k < used_indices_size; k++) {
            int i = used_indices[k];
            z += net.hidden_weights[i][j];
        }
        net.hidden_zs[j] = z;
        net.hidden[j] = activation(z);
    }
    
    net.output_z = net.output_bias;
    for (int j = 0; j < HL_SIZE; j++) {
        net.output_z += net.hidden[j] * net.output_weights[j];
    }

    net.output = net.output_z; // no activation function at the output
    return net.output;
}

// assumes forward or forward_flipidx was called before
float forward_flipidx(Network &net, const int idx) {
    bool curr_value = net.last_input[idx];

    // Compute hidden layer values.
    for (int j = 0; j < HL_SIZE; j++) {
        float z = net.hidden_zs[j];
        // update it based on the flipped bit.

        if (curr_value) { // if on, turn off. if off, turn on.
            z -= net.hidden_weights[idx][j];
        } else {
            z += net.hidden_weights[idx][j];
        }

        net.hidden_zs[j] = z;
        net.hidden[j] = activation(z);
    }
    
    net.output_z = net.output_bias;
    for (int j = 0; j < HL_SIZE; j++) {
        net.output_z += net.hidden[j] * net.output_weights[j];
    }

    net.output = net.output_z; // no activation function at the output
    return net.output;
}


void backward(Network &net, float target, float learning_rate) {
    // Since there is no activation on the output,
    // the gradient is simply:
    // dL/d(out) = 2 * (output - target)
    float dL_dout = 2.0f * (net.output - target);

    // Update output layer weights and bias.
    for (int j = 0; j < HL_SIZE; j++) {
        net.output_weights[j] -= learning_rate * (net.hidden[j] * dL_dout);
    }
    net.output_bias -= learning_rate * dL_dout;

    // Backpropagate error into the hidden layer.
    // For each hidden neuron j, we compute:
    // dL/d(hidden_z[j]) = dL/d(out) * output_weights[j] * activation_prime(hidden_zs[j])
    float hidden_errors[HL_SIZE];
    for (int j = 0; j < HL_SIZE; j++) {
        hidden_errors[j] = dL_dout * net.output_weights[j] * activation_prime(net.hidden_zs[j]);
    }

    // Update hidden layer (accumulator) weights and biases.
    for (int j = 0; j < HL_SIZE; j++) {
        for (int i = 0; i < INPUT_SIZE; i++) {
            net.hidden_weights[i][j] -= learning_rate * (net.last_input[i] * hidden_errors[j]);
        }
        net.hidden_biases[j] -= learning_rate * hidden_errors[j];
    }
}


void benchmark_time_forward_vs_forward_flipidx() {
    Network net;

    mt19937 gen(random_device{}());

    normal_distribution<float> dist1(0.0f, sqrt(2.0f / (INPUT_SIZE + HL_SIZE)));
    normal_distribution<float> dist2(0.0f, sqrt(2.0f / (1 + HL_SIZE)));
    
    for (int i = 0; i < INPUT_SIZE; i++) {
        for (int j = 0; j < HL_SIZE; j++) {
            net.hidden_weights[i][j] = dist1(gen);
        }
    }

    for (int j = 0; j < HL_SIZE; j++) {
        net.hidden_biases[j] = 0.0f;
        net.output_weights[j] = dist2(gen);
    }

    net.output_bias = 0.0f;

    bool input1[INPUT_SIZE];
    bool input2[INPUT_SIZE];
    for (int i = 0; i < INPUT_SIZE; i++) {
        input1[i] = i % 2 == 0 || i % 3 == 0; // 1
        input2[i] = i % 2 == 1; // 0
    }

    int n = 1000;
    
    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < n; i++) {
        forward(net, input1);
    }

    auto mid = chrono::high_resolution_clock::now();

    for (int i = 0; i < n; i++) {
        forward_flipidx(net, 0);
    }

    auto end = chrono::high_resolution_clock::now();

    cout << "Time for forward: " << chrono::duration_cast<chrono::microseconds>(mid - start).count() << " microseconds\n";
    cout << "Time for forward_flipidx: " << chrono::duration_cast<chrono::microseconds>(end - mid).count() << " microseconds\n";

    exit(0);
}

int main() {

    benchmark_time_forward_vs_forward_flipidx();

    Network net;

    mt19937 gen(random_device{}());

    normal_distribution<float> dist1(0.0f, sqrt(2.0f / (INPUT_SIZE + HL_SIZE)));
    normal_distribution<float> dist2(0.0f, sqrt(2.0f / (1 + HL_SIZE)));
    
    for (int i = 0; i < INPUT_SIZE; i++) {
        for (int j = 0; j < HL_SIZE; j++) {
            net.hidden_weights[i][j] = dist1(gen);
        }
    }

    for (int j = 0; j < HL_SIZE; j++) {
        net.hidden_biases[j] = 0.0f;
        net.output_weights[j] = dist2(gen);
    }

    net.output_bias = 0.0f;

    bool input1[INPUT_SIZE];
    bool input2[INPUT_SIZE];
    for (int i = 0; i < INPUT_SIZE; i++) {
        // input1[i] = i % 2 == 0 || i % 3 == 0; // 1
        // input2[i] = i % 2 == 1; // 0

        input1[i] = i % 2 == 0;
        input2[i] = i % 2 == 1;
    }

    // input2[0] = 0;
    
    float learning_rate = 0.0001f;
    for (int i = 0; i < 200; i++) {
        float o1 = forward(net, input1);
        backward(net, -50.0f, learning_rate);
        cout << "Iteration " << i << " - Output for input1: " << o1 << '\n';

        float o2 = forward(net, input2);
        backward(net, 49.0f, learning_rate);
        cout << "Iteration " << i << " - Output for input2: " << o2 << '\n';
    }

    float o1 = forward(net, input1);
    cout << "Output for input1: " << o1 << '\n';

    float o2;
    // flip every bit
    for (int i = 0; i < INPUT_SIZE; i++) {
        o2 = forward_flipidx(net, i);
    }
    cout << "Output for input2 from flip: " << o2 << '\n';

    o1 = forward(net, input2);
    cout << "Output for input2 orig: " << o1 << '\n';
    
    return 0;
}