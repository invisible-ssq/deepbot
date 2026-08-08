#pragma once
#include <vector>
#include <string>
#include <cstring>
#include <zlib.h>
#include "../utils/BinaryReader.hpp"
#include "../utils/BinaryWriter.hpp"

namespace deepbot {

// ToastyReplay 3 Format (.ttr3)
// Compressed binary format with sections

class TTR3Format {
public:
    static constexpr const char* MAGIC = "TTR3";
    static constexpr uint16_t VERSION = 1;

    struct Input {
        double timeSeconds;
        uint8_t actionType;
        uint8_t flags;
        uint16_t reserved;
    };

    struct Replay {
        double fps = 240.0;
        std::vector<Input> inputs;
    };

    static std::vector<uint8_t> write(const Replay& replay) {
        // Build header content first
        BinaryWriter headerContent;
        headerContent.writeU64(0x00000000FFFF0003);
        headerContent.writeU64(0);
        headerContent.writeI32(0);
        headerContent.writeStringVar("");
        headerContent.writeStringVar("");
        headerContent.writeF64(replay.fps);
        headerContent.writeF32(0.0f);
        headerContent.writeF32(0.0f);
        headerContent.writeI64(0);
        headerContent.writeU32(0);
        headerContent.writeU8(0);

        uint32_t headerContentSize = static_cast<uint32_t>(headerContent.size());

        // Now build full writer
        BinaryWriter writer;
        writer.writeBytes(reinterpret_cast<const uint8_t*>(MAGIC), 4);
        writer.writeU16(VERSION);
        writer.writeU16(0);  // reserved
        writer.writeU32(0x400);  // flags
        writer.writeU32(headerContentSize);  // header length
        writer.writeBytes(headerContent.data().data(), headerContent.size());

        // Build inputs section
        BinaryWriter inputsSection;
        inputsSection.writeU64(replay.inputs.size());
        for (const auto& input : replay.inputs) {
            inputsSection.writeF64(input.timeSeconds);
            inputsSection.writeU8(input.actionType);
            inputsSection.writeU8(input.flags);
            inputsSection.writeU16(input.reserved);
        }

        // Section table
        writer.writeU16(1);  // section count
        writer.writeU8(1);   // section kind (inputs)
        writer.writeU8(0);
        writer.writeU8(0);
        writer.writeU8(0);
        writer.writeU64(0);  // offset (will be 0 in single-section)
        writer.writeU64(inputsSection.size());

        // Compress inputs
        uLongf compressedSize = compressBound(inputsSection.size());
        std::vector<uint8_t> compressed(compressedSize);
        compress2(compressed.data(), &compressedSize,
            inputsSection.data().data(), inputsSection.size(), Z_DEFAULT_COMPRESSION);
        compressed.resize(compressedSize);
        writer.writeBytes(compressed.data(), compressed.size());

        return writer.intoVec();
    }

    static Replay read(const std::vector<uint8_t>& data) {
        BinaryReader reader(data);
        Replay replay;

        auto magic = reader.readBytes(4);
        if (std::memcmp(magic.data(), MAGIC, 4) != 0) {
            throw std::runtime_error("Invalid TTR3 magic");
        }

        uint16_t version = reader.readU16();
        uint16_t reserved = reader.readU16();
        uint32_t flags = reader.readU32();
        uint32_t headerLen = reader.readU32();

        // Skip header content
        reader.skip(headerLen);

        uint16_t sectionCount = reader.readU16();
        struct Section { uint8_t kind; uint64_t offset, size; };
        std::vector<Section> sections;
        for (uint16_t i = 0; i < sectionCount; i++) {
            Section sec;
            sec.kind = reader.readU8();
            reader.skip(3);
            sec.offset = reader.readU64();
            sec.size = reader.readU64();
            sections.push_back(sec);
        }

        auto payload = reader.readBytes(reader.remaining());
        std::vector<uint8_t> decoded;
        if (flags & (1 << 10)) {
            uLongf uncompressedSize = sections.empty() ? 0 :
                sections.back().offset + sections.back().size;
            decoded.resize(uncompressedSize);
            int result = uncompress(decoded.data(), &uncompressedSize,
                payload.data(), payload.size());
            if (result != Z_OK) {
                throw std::runtime_error("TTR3 decompression failed");
            }
            decoded.resize(uncompressedSize);
        } else {
            decoded = payload;
        }

        for (const auto& sec : sections) {
            if (sec.kind != 1) continue;
            if (sec.offset + sec.size > decoded.size()) {
                throw std::runtime_error("TTR3 section out of bounds");
            }
            BinaryReader secReader(decoded.data() + sec.offset, sec.size);
            uint64_t count = secReader.readU64();
            replay.inputs.reserve(count);
            for (uint64_t i = 0; i < count; i++) {
                Input input;
                input.timeSeconds = secReader.readF64();
                input.actionType = secReader.readU8();
                input.flags = secReader.readU8();
                input.reserved = secReader.readU16();
                replay.inputs.push_back(input);
            }
        }
        return replay;
    }
};

} // namespace deepbot
