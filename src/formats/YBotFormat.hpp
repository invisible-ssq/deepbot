#pragma once
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include "../utils/BinaryReader.hpp"
#include "../utils/BinaryWriter.hpp"

namespace deepbot {

// yBot 2 Format (.ybot)
// Binary format with frame deltas

class YBotFormat {
public:
    static constexpr const char* MAGIC = "YBOT";

    struct Input {
        uint64_t frame;
        bool player2;
        bool down;
        uint8_t button;
    };

    struct Replay {
        double fps = 240.0;
        std::vector<Input> inputs;
    };

    static std::vector<uint8_t> write(const Replay& replay) {
        BinaryWriter writer;
        writer.writeBytes(reinterpret_cast<const uint8_t*>(MAGIC), 4);
        writer.writeF64(replay.fps);

        // Sort inputs
        auto sorted = replay.inputs;
        std::sort(sorted.begin(), sorted.end(),
            [](const Input& a, const Input& b) { return a.frame < b.frame; });

        // Separate P1 and P2
        std::vector<Input> p1Inputs, p2Inputs;
        for (const auto& inp : sorted) {
            if (inp.player2) p2Inputs.push_back(inp);
            else p1Inputs.push_back(inp);
        }

        writer.writeVarU64(p1Inputs.size());
        writer.writeVarU64(p2Inputs.size());

        // Write P1 with delta encoding
        uint64_t prevFrame = 0;
        for (const auto& input : p1Inputs) {
            uint64_t delta = input.frame - prevFrame;
            // Include button info: 2 bits for button, 1 for down
            uint64_t packed = (delta << 3) | ((input.button & 3) << 1) | (input.down ? 1 : 0);
            writer.writeVarU64(packed);
            prevFrame = input.frame;
        }

        // Write P2 with delta encoding
        prevFrame = 0;
        for (const auto& input : p2Inputs) {
            uint64_t delta = input.frame - prevFrame;
            uint64_t packed = (delta << 3) | ((input.button & 3) << 1) | (input.down ? 1 : 0);
            writer.writeVarU64(packed);
            prevFrame = input.frame;
        }

        return writer.intoVec();
    }

    static Replay read(const std::vector<uint8_t>& data) {
        BinaryReader reader(data);
        Replay replay;

        auto magic = reader.readBytes(4);
        if (std::memcmp(magic.data(), MAGIC, 4) != 0) {
            // Try yBot 1 (.ybf) format
            return readYBF(data);
        }

        replay.fps = reader.readF64();
        uint64_t p1Count = reader.readVarU64();
        uint64_t p2Count = reader.readVarU64();

        // Read P1
        uint64_t prevFrame = 0;
        for (uint64_t i = 0; i < p1Count; i++) {
            uint64_t packed = reader.readVarU64();
            uint64_t delta = packed >> 3;
            bool down = (packed & 1) != 0;
            uint8_t button = (packed >> 1) & 3;
            prevFrame += delta;
            Input input;
            input.frame = prevFrame;
            input.player2 = false;
            input.down = down;
            input.button = button;
            DeepParser::normalizeButton(input.button);
            replay.inputs.push_back(input);
        }

        // Read P2
        prevFrame = 0;
        for (uint64_t i = 0; i < p2Count; i++) {
            uint64_t packed = reader.readVarU64();
            uint64_t delta = packed >> 3;
            bool down = (packed & 1) != 0;
            uint8_t button = (packed >> 1) & 3;
            prevFrame += delta;
            Input input;
            input.frame = prevFrame;
            input.player2 = true;
            input.down = down;
            input.button = button;
            DeepParser::normalizeButton(input.button);
            replay.inputs.push_back(input);
        }

        std::sort(replay.inputs.begin(), replay.inputs.end(),
            [](const Input& a, const Input& b) { return a.frame < b.frame; });

        return replay;
    }

    // yBot 1 (.ybf) format - simple text-like binary
    static Replay readYBF(const std::vector<uint8_t>& data) {
        BinaryReader reader(data);
        Replay replay;
        replay.fps = 240.0;

        try {
            replay.fps = reader.readF64();
            uint32_t count = reader.readU32();
            for (uint32_t i = 0; i < count; i++) {
                Input input;
                input.frame = reader.readU32();
                input.down = reader.readBool();
                input.player2 = false;
                input.button = 1;
                replay.inputs.push_back(input);
            }
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Invalid yBot format: ") + e.what());
        }

        return replay;
    }
};

} // namespace deepbot
