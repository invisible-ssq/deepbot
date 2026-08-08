#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <zlib.h>
#include "../utils/BinaryReader.hpp"
#include "../utils/BinaryWriter.hpp"

namespace deepbot {

// xdBot Format (.xd)
// Header: "XD" (2 bytes)
// Version: uint16
// Metadata + compressed inputs

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
        inputsData.writeU32(replay.inputs.size());
        for (const auto& input : replay.inputs) {
            uint32_t packed = (input.frame << 3)
                            | ((input.button & 3) << 1)
                            | (input.player2 ? 0x80000000 : 0)
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
        do {
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
            input.frame = packed >> 3;
            input.player2 = (packed & 0x80000000) != 0;
            input.down = (packed & 1) != 0;
            input.button = (packed >> 1) & 3;
            if (input.button == 0) input.button = 1;
            replay.inputs.push_back(input);
        }

        return replay;
    }
};

} // namespace deepbot