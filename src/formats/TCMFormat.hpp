#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include "../utils/BinaryReader.hpp"
#include "../utils/BinaryWriter.hpp"

namespace deepbot {

// TCBot / TCM Format (.tcm)
// Simple binary: [fps:f64][count:u32][frame:u32][down:u8][p2:u8]...

class TCMFormat {
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
        BinaryWriter writer;
        writer.writeF64(replay.fps);
        writer.writeU32(replay.inputs.size());
        for (const auto& input : replay.inputs) {
            writer.writeU32(input.frame);
            writer.writeU8(input.down ? 1 : 0);
            writer.writeU8(input.player2 ? 1 : 0);
            writer.writeU8(input.button);
        }
        return writer.intoVec();
    }

    static Replay read(const std::vector<uint8_t>& data) {
        BinaryReader reader(data);
        Replay replay;
        replay.fps = reader.readF64();
        uint32_t count = reader.readU32();
        replay.inputs.reserve(count);
        for (uint32_t i = 0; i < count; i++) {
            Input input;
            input.frame = reader.readU32();
            input.down = reader.readU8() != 0;
            input.player2 = reader.readU8() != 0;
            input.button = reader.readU8();
            if (input.button == 0) input.button = 1;
            replay.inputs.push_back(input);
        }
        return replay;
    }
};

} // namespace deepbot