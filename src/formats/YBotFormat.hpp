#pragma once
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include "../utils/BinaryReader.hpp"
#include "../utils/BinaryWriter.hpp"

namespace deepbot {

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

        auto sorted = replay.inputs;
        std::sort(sorted.begin(), sorted.end(),
            [](const Input& a, const Input& b) { return a.frame < b.frame; });

        std::vector<Input> p1Inputs, p2Inputs;
        for (const auto& inp : sorted) {
            if (inp.player2) p2Inputs.push_back(inp);
            else p1Inputs.push_back(inp);
        }

        writer.writeVarU64(p1Inputs.size());
        writer.writeVarU64(p2Inputs.size());

        uint64_t prevFrame = 0;
        for (const auto& input : p1Inputs) {
            uint64_t delta = input.frame - prevFrame;
            uint64_t packed = (delta << 3) | ((input.button & 3) << 1) | (input.down ? 1 : 0);
            writer.writeVarU64(packed);
            prevFrame = input.frame;
        }

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
        if (data.size() < 4) {
            return readYBF(data);
        }
        
        if (std::memcmp(data.data(), MAGIC, 4) != 0) {
            return readYBF(data);
        }

        BinaryReader reader(data);
        Replay replay;

        auto magic = reader.readBytes(4);
        replay.fps = reader.readF64();
        uint64_t p1Count = reader.readVarU64();
        uint64_t p2Count = reader.readVarU64();

        uint64_t prevFrame = 0;
        for (uint64_t i = 0; i < p1Count && reader.remaining() > 0; i++) {
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
            normalizeButton(input.button);
            replay.inputs.push_back(input);
        }

        prevFrame = 0;
        for (uint64_t i = 0; i < p2Count && reader.remaining() > 0; i++) {
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
            normalizeButton(input.button);
            replay.inputs.push_back(input);
        }

        std::sort(replay.inputs.begin(), replay.inputs.end(),
            [](const Input& a, const Input& b) { return a.frame < b.frame; });

        return replay;
    }

    static Replay readYBF(const std::vector<uint8_t>& data) {
        BinaryReader reader(data);
        Replay replay;
        replay.fps = 240.0;

        try {
            replay.fps = reader.readF64();
            uint32_t count = reader.readU32();
            for (uint32_t i = 0; i < count && reader.remaining() >= 5; i++) {
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
