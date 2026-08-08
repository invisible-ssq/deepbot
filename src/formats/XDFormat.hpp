#pragma once
#include <vector>
#include <string>
#include <cstring>
#include <zlib.h>
#include "../utils/BinaryReader.hpp"
#include "../utils/BinaryWriter.hpp"

namespace deepbot {

class XDFormat {
public:
    static constexpr const char* MAGIC = "XD";
    static constexpr uint16_t VERSION = 1;

    struct Input {
        uint32_t frame;
        bool player2;
        bool down;
        uint8_t button;
    };

    struct Replay {
        double fps = 240.0;
        std::string levelName;
        std::vector<Input> inputs;
    };

    static std::vector<uint8_t> write(const Replay& replay) {
        BinaryWriter writer;
        writer.writeBytes(reinterpret_cast<const uint8_t*>(MAGIC), 2);
        writer.writeU16(VERSION);
        writer.writeF64(replay.fps);
        writer.writeStringVar(replay.levelName);

        BinaryWriter inputsData;
        inputsData.writeU32(static_cast<uint32_t>(replay.inputs.size()));
        for (const auto& input : replay.inputs) {
            // FIXED: use bit 2 for player2 instead of bit 31 (sign bit)
            if (input.frame > 0x1FFFFFFF) {
                throw std::runtime_error("XD format: frame number too large (max 536,870,911)");
            }
            uint32_t packed = ((input.frame & 0x1FFFFFFF) << 3)
                | ((input.button & 3) << 1)
                | (input.player2 ? 4 : 0)   // bit 2 for player2
                | (input.down ? 1 : 0);
            inputsData.writeU32(packed);
        }

        uLongf compressedSize = compressBound(inputsData.size());
        std::vector<uint8_t> compressed(compressedSize);
        compress2(compressed.data(), &compressedSize,
            inputsData.data().data(), inputsData.size(), Z_BEST_COMPRESSION);
        compressed.resize(compressedSize);

        writer.writeVarU64(compressed.size());
        writer.writeBytes(compressed.data(), compressed.size());
        return writer.intoVec();
    }

    static Replay read(const std::vector<uint8_t>& data) {
        BinaryReader reader(data);
        Replay replay;

        auto magic = reader.readBytes(2);
        if (std::memcmp(magic.data(), MAGIC, 2) != 0) {
            throw std::runtime_error("Invalid XD magic");
        }

        uint16_t version = reader.readU16();
        replay.fps = reader.readF64();
        replay.levelName = reader.readStringVar();

        uint64_t compressedSize = reader.readVarU64();
        auto compressed = reader.readBytes(compressedSize);

        uLongf uncompressedSize = compressedSize * 10;
        std::vector<uint8_t> uncompressed;
        int result;
        int attempts = 0;
        const int MAX_ATTEMPTS = 10;
        do {
            if (attempts >= MAX_ATTEMPTS) {
                throw std::runtime_error("XD decompression failed: too many attempts");
            }
            attempts++;
            uncompressedSize *= 2;
            uncompressed.resize(uncompressedSize);
            result = uncompress(uncompressed.data(), &uncompressedSize,
                compressed.data(), compressed.size());
        } while (result == Z_BUF_ERROR);

        if (result != Z_OK) {
            throw std::runtime_error("XD decompression failed");
        }
        uncompressed.resize(uncompressedSize);

        BinaryReader inputsReader(uncompressed);
        uint32_t count = inputsReader.readU32();
        replay.inputs.reserve(count);

        for (uint32_t i = 0; i < count; i++) {
            uint32_t packed = inputsReader.readU32();
            Input input;
            input.frame = (packed >> 3) & 0x1FFFFFFF;
            input.player2 = (packed & 4) != 0;   // bit 2
            input.down = (packed & 1) != 0;
            input.button = (packed >> 1) & 3;
            DeepParser::normalizeButton(input.button);
            replay.inputs.push_back(input);
        }

        return replay;
    }
};

} // namespace deepbot
