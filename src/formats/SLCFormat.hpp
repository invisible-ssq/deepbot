#pragma once
#include <vector>
#include <cstdint>
#include "../utils/BinaryReader.hpp"
#include "../utils/BinaryWriter.hpp"

namespace deepbot {

class SLCFormat {
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
        uint64_t seed = 0;
    };

    static std::vector<uint8_t> write(const Replay& replay) {
        BinaryWriter writer;
        writer.writeF64(replay.fps);
        writer.writeU32(replay.inputs.size());
        for (const auto& input : replay.inputs) {
            uint32_t packed = (input.frame << 4)
                            | ((input.button & 3) << 1)
                            | ((input.player2 ? 1 : 0) << 3)
                            | (input.down ? 1 : 0);
            writer.writeU32(packed);
        }
        writer.writeU64(replay.seed);
        return writer.intoVec();
    }

    static Replay read(const std::vector<uint8_t>& data) {
        BinaryReader reader(data);
        Replay replay;
        if (data.size() >= 4) {
            if (data[0] == 0x53 && data[1] == 0x49 && data[2] == 0x4C && data[3] == 0x4C) {
                throw std::runtime_error("Use SLC2/SLC3 parser for this file");
            }
            if (data.size() >= 8 && std::memcmp(data.data(), "SLC3RPLY", 8) == 0) {
                throw std::runtime_error("Use SLC3 parser for this file");
            }
        }
        replay.fps = reader.readF64();
        uint32_t count = reader.readU32();
        replay.inputs.reserve(count);
        for (uint32_t i = 0; i < count; i++) {
            uint32_t packed = reader.readU32();
            Input input;
            input.frame = packed >> 4;
            input.player2 = (packed & 0x8) != 0;
            input.down = (packed & 0x1) != 0;
            input.button = (packed >> 1) & 3;
            if (input.button == 0) input.button = 1;
            replay.inputs.push_back(input);
        }
        if (!reader.eof()) {
            replay.seed = reader.readU64();
        }
        return replay;
    }
};

} // namespace deepbot