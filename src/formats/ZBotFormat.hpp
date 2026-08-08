#pragma once
#include <vector>
#include <string>
#include <cstring>
#include "../utils/BinaryReader.hpp"
#include "../utils/BinaryWriter.hpp"

namespace deepbot {

class ZBotFormat {
public:
    static constexpr const char* MAGIC = "ZBF";
    static constexpr uint16_t VERSION = 1;

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
        writer.writeBytes(reinterpret_cast<const uint8_t*>(MAGIC), 3);
        writer.writeU16(VERSION);
        writer.writeF64(replay.fps);
        writer.writeU32(static_cast<uint32_t>(replay.inputs.size()));
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
        auto magic = reader.readBytes(3);
        if (std::memcmp(magic.data(), MAGIC, 3) != 0) {
            throw std::runtime_error("Invalid ZBF magic");
        }
        [[maybe_unused]] uint16_t version = reader.readU16();
        replay.fps = reader.readF64();
        uint32_t count = reader.readU32();
        if (count > 10000000) {
            throw std::runtime_error("ZBF: suspicious input count");
        }
        replay.inputs.reserve(count);
        for (uint32_t i = 0; i < count && reader.remaining() >= 7; i++) {
            Input input;
            input.frame = reader.readU32();
            input.down = reader.readU8() != 0;
            input.player2 = reader.readU8() != 0;
            input.button = reader.readU8();
            normalizeButton(input.button);
            replay.inputs.push_back(input);
        }
        return replay;
    }
};

} // namespace deepbot
