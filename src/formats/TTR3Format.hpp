#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <zlib.h>
#include "../utils/BinaryReader.hpp"
#include "../utils/BinaryWriter.hpp"

namespace deepbot {

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
        double duration = 0.0;
        std::vector<Input> inputs;
    };

    static std::vector<uint8_t> write(const Replay& replay) {
        BinaryWriter writer;
        writer.writeBytes(reinterpret_cast<const uint8_t*>(MAGIC), 4);
        writer.writeU16(VERSION);
        writer.writeU16(0);
        writer.writeU32(0x400);
        uint32_t headerLenPos = writer.size();
        writer.writeU32(0);
        writer.writeU64(0x00000000FFFF0003);
        writer.writeU64(0);
        writer.writeI32(0);
        writer.writeStringVar("");
        writer.writeStringVar("");
        writer.writeF64(replay.fps);
        writer.writeF32(0.0f);
        writer.writeF32(0.0f);
        writer.writeI64(0);
        writer.writeU32(0);
        writer.writeU8(0);
        uint32_t headerLen = writer.size();
        std::memcpy(writer.data().data() + headerLenPos, &headerLen, 4);

        BinaryWriter inputsSection;
        inputsSection.writeU64(replay.inputs.size());
        for (const auto& input : replay.inputs) {
            inputsSection.writeF64(input.timeSeconds);
            inputsSection.writeU8(input.actionType);
            inputsSection.writeU8(input.flags);
            inputsSection.writeU16(input.reserved);
        }

        writer.writeU16(1);
        writer.writeU8(1);
        writer.writeU8(0);
        writer.writeU8(0);
        writer.writeU8(0);
        writer.writeU64(0);
        writer.writeU64(inputsSection.size());

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
        reader.skip(8);
        reader.skip(8);
        reader.skip(4);
        reader.readStringVar();
        reader.readStringVar();
        replay.fps = reader.readF64();
        reader.skip(4 + 4 + 8 + 4 + 1);
        reader.seek(headerLen);
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
            uncompress(decoded.data(), &uncompressedSize, payload.data(), payload.size());
        } else {
            decoded = payload;
        }
        for (const auto& sec : sections) {
            if (sec.kind != 1) continue;
            BinaryWriter secWriter;
            secWriter.writeBytes(decoded.data() + sec.offset, sec.size);
            BinaryReader secReader(secWriter.data());
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