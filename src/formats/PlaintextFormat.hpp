#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace deepbot {

// Plaintext Format (.txt)
// Simple text format: frame,down,player2,button per line

class PlaintextFormat {
public:
    struct Input {
        uint32_t frame;
        bool player2;
        bool down;
        uint8_t button;
    };

    struct Replay {
        double fps = 240.0;
        std::vector<Input> inputs;
    };

    static std::vector<uint8_t> write(const Replay& replay) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6);
        oss << "# deepbot plaintext replay\n";
        oss << "# fps: " << replay.fps << "\n";
        oss << "# frame,down,player2,button\n";

        for (const auto& input : replay.inputs) {
            oss << input.frame << ","
                << (input.down ? "1" : "0") << ","
                << (input.player2 ? "1" : "0") << ","
                << (int)input.button << "\n";
        }

        std::string str = oss.str();
        return std::vector<uint8_t>(str.begin(), str.end());
    }

    static Replay read(const std::vector<uint8_t>& data) {
        std::string str(data.begin(), data.end());
        std::istringstream iss(str);
        Replay replay;
        replay.fps = 240.0;

        std::string line;
        while (std::getline(iss, line)) {
            // Skip comments and empty lines
            if (line.empty() || line[0] == '#') {
                // Parse fps from comment
                if (line.find("fps:") != std::string::npos) {
                    size_t pos = line.find("fps:");
                    try {
                        replay.fps = std::stod(line.substr(pos + 4));
                    } catch (...) {
                        // Ignore parse error, use default
                    }
                }
                continue;
            }

            std::istringstream lineStream(line);
            std::string token;
            std::vector<std::string> tokens;

            while (std::getline(lineStream, token, ',')) {
                tokens.push_back(token);
            }

            if (tokens.size() < 2) continue;

            try {
                Input input;
                input.frame = std::stoul(tokens[0]);
                input.down = std::stoi(tokens[1]) != 0;
                input.player2 = (tokens.size() > 2) ? (std::stoi(tokens[2]) != 0) : false;
                input.button = (tokens.size() > 3) ? static_cast<uint8_t>(std::stoi(tokens[3])) : 1;
                DeepParser::normalizeButton(input.button);
                replay.inputs.push_back(input);
            } catch (const std::exception& e) {
                // Skip malformed line
                continue;
            }
        }

        return replay;
    }
};

} // namespace deepbot
