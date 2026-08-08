#pragma once
#include <vector>
#include <string>
#include <cstring>
#include "../utils/BinaryReader.hpp"
#include "../utils/BinaryWriter.hpp"
#include "../utils/TPSFix.hpp"

namespace deepbot {

#pragma pack(push, 1)
struct DeepInput {
    double absoluteTime;
    uint8_t flags;
    float x, y, rotation, yAccel;
};
#pragma pack(pop)

class DeepFormat {
public:
    static constexpr const char* MAGIC = "DEEP";
    static constexpr uint16_t VERSION = 1;
    static constexpr const char* FOOTER = "END!";

    struct Replay {
        std::string author;
        std::string description;
        int32_t levelId = 0;
        std::string levelName;
        double tps = 240.0;
        double duration = 0.0;
        uint32_t gameVersion = 0;
        uint32_t seed = 0;
        std::vector<DeepInput> inputs;
    };

    static std::vector<uint8_t> write(const Replay& replay) {
        BinaryWriter headerWriter;
        headerWriter.writeString(replay.author);
        headerWriter.writeString(replay.description);
        headerWriter.writeI32(replay.levelId);
        headerWriter.writeString(replay.levelName);
        headerWriter.writeF64(replay.tps);
        headerWriter.writeF64(replay.duration);
        headerWriter.writeU32(replay.gameVersion);
        headerWriter.writeU32(replay.seed);
        
        uint32_t headerSize = static_cast<uint32_t>(headerWriter.size());
        
        BinaryWriter writer;
        writer.writeBytes(reinterpret_cast<const uint8_t*>(MAGIC), 4);
        writer.writeU16(VERSION);
        writer.writeU32(0); // flags
        writer.writeU32(headerSize);
        writer.writeBytes(headerWriter.data().data(), headerWriter.size());
        
        writer.writeVarU64(replay.inputs.size());
        for (const auto& input : replay.inputs) {
            writer.writeF64(input.absoluteTime);
            writer.writeU8(input.flags);
            writer.writeF32(input.x);
            writer.writeF32(input.y);
            writer.writeF32(input.rotation);
            writer.writeF32(input.yAccel);
        }
        writer.writeBytes(reinterpret_cast<const uint8_t*>(FOOTER), 4);
        return writer.intoVec();
    }

    static Replay read(const std::vector<uint8_t>& data) {
        BinaryReader reader(data);
        Replay replay;
        auto magic = reader.readBytes(4);
        if (std::memcmp(magic.data(), MAGIC, 4) != 0) {
            throw std::runtime_error("Invalid .deep magic");
        }
        uint16_t version = reader.readU16();
        if (version != VERSION) {
            throw std::runtime_error("Unsupported .deep version");
        }
        uint32_t flags = reader.readU32();
        uint32_t headerSize = reader.readU32();
        
        replay.author = reader.readString();
        replay.description = reader.readString();
        replay.levelId = reader.readI32();
        replay.levelName = reader.readString();
        replay.tps = reader.readF64();
        replay.duration = reader.readF64();
        replay.gameVersion = reader.readU32();
        replay.seed = reader.readU32();
        
        size_t headerEnd = 14 + headerSize;
        if (reader.position() < headerEnd) {
            reader.skip(headerEnd - reader.position());
        }
        
        uint64_t inputCount = reader.readVarU64();
        replay.inputs.reserve(inputCount);
        for (uint64_t i = 0; i < inputCount; i++) {
            DeepInput input;
            input.absoluteTime = reader.readF64();
            input.flags = reader.readU8();
            input.x = reader.readF32();
            input.y = reader.readF32();
            input.rotation = reader.readF32();
            input.yAccel = reader.readF32();
            replay.inputs.push_back(input);
        }
        
        // Skip footer check - not all writers include it
        return replay;
    }
};

} // namespace deepbot
