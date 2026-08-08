#pragma once
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include "../utils/BinaryReader.hpp"
#include "../utils/BinaryWriter.hpp"

namespace deepbot {

class REFormat {
public:
    struct Input {
        uint32_t frame;
        bool player2;
        bool down;
        uint8_t button;
    };

    struct Replay {
        double fps = 240.0;
        int version = 1;
        std::vector<Input> inputs;
    };

    static int detectVersion(const std::vector<uint8_t>& data) {
        if (data.size() < 4) return 1;
        if (data[0] == 'R' && data[1] == 'E' && data[2] == '4') return 4;
        if (data[0] == 'R' && data[1] == 'E' && data[2] == '3') return 3;
        if (data[0] == 'R' && data[1] == 'E' && data[2] == '2') return 2;
        return 1;
    }

    static std::vector<uint8_t> write(const Replay& replay, int version = 1) {
        BinaryWriter writer;

        if (version >= 2) {
            std::string header = "RE" + std::to_string(version);
            writer.writeBytes(reinterpret_cast<const uint8_t*>(header.c_str()), header.size());
        }

        writer.writeF64(replay.fps);

        if (version >= 3) {
            writer.writeU32(0);
            writer.writeU8(0);
        }

        if (version >= 4) {
            writer.writeStringVar("");
            writer.writeStringVar("");
        }

        auto sorted = replay.inputs;
        std::sort(sorted.begin(), sorted.end(),
            [](const Input& a, const Input& b) { return a.frame < b.frame; });

        std::vector<Input> p1Inputs, p2Inputs;
        for (const auto& inp : sorted) {
            if (inp.player2) p2Inputs.push_back(inp);
            else p1Inputs.push_back(inp);
        }

        if (version >= 2) {
            writer.writeVarU64(p1Inputs.size());
            writer.writeVarU64(p2Inputs.size());
        } else {
            // RE v1: merge all as P1 (P2 data loss)
            writer.writeU32(static_cast<uint32_t>(p1Inputs.size() + p2Inputs.size()));
        }

        uint32_t prevFrame = 0;
        for (const auto& input : p1Inputs) {
            uint32_t delta = input.frame - prevFrame;
            if (version >= 3) {
                uint32_t packed = (delta << 2) | (input.down ? 1 : 0) | (input.button == 2 ? 2 : 0);
                writer.writeVarU64(packed);
            } else {
                uint32_t packed = (delta << 1) | (input.down ? 1 : 0);
                writer.writeU32(packed);
            }
            prevFrame = input.frame;
        }

        prevFrame = 0;
        for (const auto& input : p2Inputs) {
            uint32_t delta = input.frame - prevFrame;
            if (version >= 3) {
                uint32_t packed = (delta << 2) | (input.down ? 1 : 0) | (input.button == 2 ? 2 : 0);
                writer.writeVarU64(packed);
            } else {
                uint32_t packed = (delta << 1) | (input.down ? 1 : 0);
                writer.writeU32(packed);
            }
            prevFrame = input.frame;
        }

        return writer.intoVec();
    }

    static Replay read(const std::vector<uint8_t>& data) {
        int version = detectVersion(data);
        BinaryReader reader(data);
        Replay replay;
        replay.version = version;

        if (version >= 2) {
            reader.skip(3);
        }

        replay.fps = reader.readF64();

        if (version >= 3) {
            reader.skip(4);
            reader.skip(1);
        }

        if (version >= 4) {
            reader.readStringVar();
            reader.readStringVar();
        }

        uint64_t p1Count = 0, p2Count = 0;
        if (version >= 2) {
            p1Count = reader.readVarU64();
            p2Count = reader.readVarU64();
        } else {
            uint32_t total = reader.readU32();
            p1Count = total;
        }

        uint32_t prevFrame = 0;
        for (uint64_t i = 0; i < p1Count && reader.remaining() > 0; i++) {
            uint64_t packed = (version >= 3) ? reader.readVarU64() : reader.readU32();
            uint32_t delta = packed >> (version >= 3 ? 2 : 1);
            bool down = (packed & 1) != 0;
            uint8_t button = 1;
            if (version >= 3 && (packed & 2)) button = 2;
            prevFrame += delta;
            Input input;
            input.frame = prevFrame;
            input.player2 = false;
            input.down = down;
            input.button = button;
            replay.inputs.push_back(input);
        }

        prevFrame = 0;
        for (uint64_t i = 0; i < p2Count && reader.remaining() > 0; i++) {
            uint64_t packed = (version >= 3) ? reader.readVarU64() : reader.readU32();
            uint32_t delta = packed >> (version >= 3 ? 2 : 1);
            bool down = (packed & 1) != 0;
            uint8_t button = 1;
            if (version >= 3 && (packed & 2)) button = 2;
            prevFrame += delta;
            Input input;
            input.frame = prevFrame;
            input.player2 = true;
            input.down = down;
            input.button = button;
            replay.inputs.push_back(input);
        }

        std::sort(replay.inputs.begin(), replay.inputs.end(),
            [](const Input& a, const Input& b) { return a.frame < b.frame; });

        return replay;
    }
};

} // namespace deepbot
