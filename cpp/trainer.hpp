#pragma once
#include "search2.hpp"
#include "nn.hpp"
#include <random>

class Trainer {
    Network net;
    std::string weights_path;
    int games_played = 0;
    const int games_per_iteration = 1;
    float learning_rate = 0.0001f;

    struct GameRecord {
        vec<Position> positions;
        vec<int> scores;  // Scores from NNUE/whatever else at time of position evaluation
        PosType outcome;  // Final game result
    };
    
    vec<GameRecord> game_history;
    // const float discount_factor = 0.99f;

public:
    Trainer(std::string weights_path): weights_path(weights_path) {
        // Initialize network with random weights if no file exists
        std::ifstream in(weights_path, std::ios::binary);
        if (in.good()) {
            net.load(in);
        } else {
            initializeRandomWeights();
        }
    }

    void train(LuaInterface& lua) {
        while (games_played < games_per_iteration) {
            selfPlay(lua);
            games_played++;
            
            if (games_played % 10 == 0) {
                saveWeights();
            }
        }
    }

private:
    void initializeRandomWeights() {
        std::mt19937 gen(std::random_device{}());
        std::normal_distribution<float> dist(0.0f, 0.1f);
        
        for (int i = 0; i < INPUT_SIZE; i++)
            for (int j = 0; j < HL_SIZE; j++)
                net.hidden_weights[i][j] = dist(gen);
        
        for (int i = 0; i < HL_SIZE; i++) {
            net.hidden_biases[i] = 0.0f;
            net.output_weights[i] = dist(gen);
        }
        net.output_bias = 0.0f;
    }

    void printBoardStateHumanReadable(unsigned char board[64]) {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                int x = i*8 + j;
                if (board[x] == 0) {
                    std::cout << "  ";
                } else {
                    auto piece = board[x];
                    if (piece <= 6) {
                        std::cout << ".";
                    } else {
                        std::cout << "+";
                        piece -= 6;
                    }
                    std::cout << piece_names[piece];
                }
            }
            std::cout << std::endl;
        }
    }

    void selfPlay(LuaInterface& lua) {
        Position pos = lua.initial_position();
        // Searcher searcher(6, 8, 8, 4, 4, pos);
        // searcher.nnue = net;

        Searcher searcher(32, 8, 8, 1000, 0, "../lua-scripts/chess/chess2.lua");

        GameRecord record;
        
        while (true) {
            auto pos_type = lua.get_pos_type(pos);
            auto pos_type_str = pos_type == PosType::Win ? "Win" : 
                pos_type == PosType::Loss ? "Loss" : 
                pos_type == PosType::Draw ? "Draw" : "Other";

            std::cout << "Pos type: " << pos_type_str << std::endl;
            if (pos_type != PosType::Other) {
                record.outcome = pos_type;
                break;
            }

            // vec<Move> moves;

            // lua.valid_moves(moves, pos);

            // if (moves.size() == 0) {
            //     std::cout << "No moves found!\n";
            //     break;
            // }

            // Move move = moves[rand() % moves.size()];

            // Move move = moves[1];
            
            auto out = searcher.search(pos);

            std::cout << "before search\n";
            
            Searcher::SearchOut sout = searcher.search(pos);
            std::cout << "Search done. Possible moves: " << sout.possible.size() << '\n';
            std::cout << "Move index: " << sout.move_i << '\n';
            // if (!move) break;
            if (sout.move_i == -1) {
                std::cout << "No moves found!\n";
                break;
            }

            // sout.move_i %= sout.possible.size();

            Move move = sout.possible[sout.move_i];
            
            record.positions.push_back(pos);
            record.scores.push_back(searcher.score(pos));

            printBoardStateHumanReadable(move.board);
            
            memcpy(pos.board, move.board, sizeof(pos.board));
            pos.next_player ^= 1;
        }

        // Store game record
        game_history.push_back(record);

        // Train on this game using TD(λ) learning
        float target = getOutcomeValue(record.outcome);
        
        for (int i = record.positions.size() - 1; i >= 0; i--) {
            bool input[INPUT_SIZE] = {false};
            convertPosToNN(record.positions[i], input);
            
            // Current position's value
            float current = forward(net, input) * 100.0f;
            
            // TD target is mix of actual outcome and next position's value
            float next_value = (i < record.positions.size() - 1) ? 
                record.scores[i+1] : target;
            float td_target = (current + learning_rate * (next_value - current)) / 100.0f;
            
            // Train network toward TD target
            backward(net, td_target, learning_rate);
        }
    }

    float getOutcomeValue(PosType outcome) {
        switch(outcome) {
            case PosType::Win: return WINNING;
            case PosType::Loss: return LOSING;
            case PosType::Draw: return 0;
            default: return 0;
        }
    }

    void convertPosToNN(const Position& pos, bool input[INPUT_SIZE]) {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                int x = i*8 + j;
                if (pos.board[x]) {
                    int piece_idx = (pos.board[x]-1)*64 + i*8 + j;
                    input[piece_idx] = true;
                }
            }
        }
    }

    void saveWeights() {
        std::ofstream out(weights_path, std::ios::binary);
        net.save(out);
    }
};
