#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include "../utils/BinaryReader.hpp"
#include "../utils/BinaryWriter.hpp"

namespace deepbot {

// Echo Format (.echo)
// Binary format with simple structure

class EchoFormat {
public:
    static constexpr const char* MAGIC = "ECHO";
    static constexpr uint16_t VERSION = 1;

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
        writer.writeU16(VERSION);
        writer.writeF64(replay.fps);
        writer.writeVarU64(replay.inputs.size());

        for (const auto& input : replay.inputs) {
            writer.writeVarU64(input.frame);
            uint8_t flags = (input.down ? 1 : 0)
                          | (input.player2 ? 2 : 0)
                          | ((input.button & 3) << 2);
            writer.writeU8(flags);
        }

        return writer.intoVec();
    }

    static Replay read(const std::vector<uint8_t>& data) {
        BinaryReader reader(data);
        Replay replay;

        auto magic = reader.readBytes(4);
        if (std::memcmp(magic.data(), MAGIC, 4) != 0) {
            throw std::runtime_error("Invalid Echo magic");
        }

        uint16_t version = reader.readU16();
        replay.fps = reader.readF64();
        uint64_t count = reader.readVarU64();
        replay.inputs.reserve(count);

        for (uint64_t i = 0; i < count; i++) {
            Input input;
            input.frame = reader.readVarU64();
            uint8_t flags = reader.readU8();
            input.down = flags & 1;
            input.player2 = flags & 2;
            input.button = (flags >> 2) & 3;
            if (input.button == 0) input.button = 1;
            replay.inputs.push_back(input);
        }

        return replay;
    }
};

} // namespace deepbot