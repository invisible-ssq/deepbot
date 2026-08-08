#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include "../utils/BinaryReader.hpp"
#include "../utils/BinaryWriter.hpp"

namespace deepbot {

class GDR2Format {
public:
    static constexpr const char* MAGIC = "GDR";
    static constexpr uint64_t VERSION = 2;

    struct PhysicsData {
        float xPosition, yPosition, rotation;
        double xVelocity, yVelocity;
    };

    struct Input {
        uint64_t frame;
        uint8_t button;
        bool player2;
        bool down;
        PhysicsData* physics = nullptr;
    };

    struct Replay {
        std::string author, description;
        float duration = 0;
        int32_t gameVersion = 0;
        double framerate = 240.0;
        int32_t seed = 0;
        int32_t coins = 0;
        bool ldm = false, platformer = false;
        struct { std::string name; int32_t version; } botInfo;
        struct { uint32_t id; std::string name; } levelInfo;
        std::vector<Input> inputs;
        std::vector<uint64_t> deaths;
    };

    static std::vector<uint8_t> write(const Replay& replay) {
        BinaryWriter writer;
        writer.writeBytes(reinterpret_cast<const uint8_t*>(MAGIC), 3);
        writer.writeVarU64(VERSION);
        bool hasPhysics = false;
        for (const auto& inp : replay.inputs) {
            if (inp.physics) { hasPhysics = true; break; }
        }
        writer.writeStringVar(hasPhysics ? "Phys" : "");
        writer.writeStringVar(replay.author);
        writer.writeStringVar(replay.description);
        writer.writeF32(replay.duration);
        writer.writeVarU64(replay.gameVersion);
        writer.writeF64(replay.framerate);
        writer.writeVarU64(replay.seed);
        writer.writeVarU64(replay.coins);
        writer.writeBool(replay.ldm);
        writer.writeBool(replay.platformer);
        writer.writeStringVar(replay.botInfo.name);
        writer.writeVarU64(replay.botInfo.version);
        writer.writeVarU64(replay.levelInfo.id);
        writer.writeStringVar(replay.levelInfo.name);
        writer.writeVarU64(0);
        writer.writeVarU64(replay.deaths.size());
        uint64_t prevDeath = 0;
        for (uint64_t death : replay.deaths) {
            writer.writeVarU64(death - prevDeath);
            prevDeath = death;
        }
        size_t p1Count = 0;
        for (const auto& inp : replay.inputs) {
            if (!inp.player2) p1Count++;
        }
        writer.writeVarU64(replay.inputs.size());
        writer.writeVarU64(p1Count);
        uint64_t prevFrame = 0;
        for (const auto& input : replay.inputs) {
            if (input.player2) continue;
            uint64_t delta = input.frame - prevFrame;
            uint64_t packed;
            if (replay.platformer) {
                packed = (delta << 3) | ((input.button & 3) << 1) | (input.down ? 1 : 0);
            } else {
                packed = (delta << 1) | (input.down ? 1 : 0);
            }
            writer.writeVarU64(packed);
            if (hasPhysics && input.physics) {
                BinaryWriter ext;
                ext.writeF32(input.physics->xPosition);
                ext.writeF32(input.physics->yPosition);
                ext.writeF32(input.physics->rotation);
                ext.writeF64(input.physics->xVelocity);
                ext.writeF64(input.physics->yVelocity);
                writer.writeVarU64(ext.size());
                writer.writeBytes(ext.data().data(), ext.size());
            }
            prevFrame = input.frame;
        }
        prevFrame = 0;
        for (const auto& input : replay.inputs) {
            if (!input.player2) continue;
            uint64_t delta = input.frame - prevFrame;
            uint64_t packed;
            if (replay.platformer) {
                packed = (delta << 3) | ((input.button & 3) << 1) | (input.down ? 1 : 0);
            } else {
                packed = (delta << 1) | (input.down ? 1 : 0);
            }
            writer.writeVarU64(packed);
            if (hasPhysics && input.physics) {
                BinaryWriter ext;
                ext.writeF32(input.physics->xPosition);
                ext.writeF32(input.physics->yPosition);
                ext.writeF32(input.physics->rotation);
                ext.writeF64(input.physics->xVelocity);
                ext.writeF64(input.physics->yVelocity);
                writer.writeVarU64(ext.size());
                writer.writeBytes(ext.data().data(), ext.size());
            }
            prevFrame = input.frame;
        }
        return writer.intoVec();
    }

    static Replay read(const std::vector<uint8_t>& data) {
        BinaryReader reader(data);
        Replay replay;
        auto magic = reader.readBytes(3);
        if (std::memcmp(magic.data(), MAGIC, 3) != 0) {
            throw std::runtime_error("Invalid GDR2 magic");
        }
        uint64_t version = reader.readVarU64();
        if (version != VERSION) {
            throw std::runtime_error("Unsupported GDR2 version");
        }
        std::string inputTag = reader.readStringVar();
        bool hasExtension = !inputTag.empty();
        replay.author = reader.readStringVar();
        replay.description = reader.readStringVar();
        replay.duration = reader.readF32();
        replay.gameVersion = reader.readVarU64();
        replay.framerate = reader.readF64();
        replay.seed = reader.readVarU64();
        replay.coins = reader.readVarU64();
        replay.ldm = reader.readBool();
        replay.platformer = reader.readBool();
        replay.botInfo.name = reader.readStringVar();
        replay.botInfo.version = reader.readVarU64();
        replay.levelInfo.id = reader.readVarU64();
        replay.levelInfo.name = reader.readStringVar();
        uint64_t extSize = reader.readVarU64();
        reader.skip(extSize);
        uint64_t deathCount = reader.readVarU64();
        uint64_t prevDeath = 0;
        for (uint64_t i = 0; i < deathCount; i++) {
            prevDeath += reader.readVarU64();
            replay.deaths.push_back(prevDeath);
        }
        uint64_t totalInputs = reader.readVarU64();
        uint64_t p1Inputs = reader.readVarU64();
        uint64_t p1Remaining = p1Inputs;
        uint64_t prevFrame = 0;
        while (reader.remaining() > 0 && replay.inputs.size() < totalInputs) {
            uint64_t packed = reader.readVarU64();
            uint64_t delta;
            uint8_t button;
            if (replay.platformer) {
                delta = packed >> 3;
                button = ((packed >> 1) & 3);
            } else {
                delta = packed >> 1;
                button = 1;
            }
            uint64_t frame = prevFrame + delta;
            bool isP2 = (p1Remaining == 0);
            bool down = (packed & 1) != 0;
            Input input;
            input.frame = frame;
            input.button = button;
            input.player2 = isP2;
            input.down = down;
            if (hasExtension) {
                if (reader.remaining() == 0) break;
                uint64_t extLen = reader.readVarU64();
                if (extLen > 0) {
                    if (extLen > reader.remaining()) break;
                    auto extData = reader.readBytes(extLen);
                    if (inputTag == "Phys") {
                        BinaryReader extReader(extData);
                        input.physics = new PhysicsData();
                        input.physics->xPosition = extReader.readF32();
                        input.physics->yPosition = extReader.readF32();
                        input.physics->rotation = extReader.readF32();
                        input.physics->xVelocity = extReader.readF64();
                        input.physics->yVelocity = extReader.readF64();
                    }
                }
            }
            prevFrame = frame;
            replay.inputs.push_back(input);
            if (p1Remaining > 0) {
                p1Remaining--;
                if (p1Remaining == 0) prevFrame = 0;
            }
        }
        std::sort(replay.inputs.begin(), replay.inputs.end(),
            [](const Input& a, const Input& b) { return a.frame < b.frame; });
        return replay;
    }
};

} // namespace deepbot