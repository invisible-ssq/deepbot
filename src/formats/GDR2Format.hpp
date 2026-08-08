#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
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
        std::unique_ptr<PhysicsData> physics; // FIXED: was raw pointer
    };

    struct Replay {
        std::string author, description;
        float duration = 0;
        int32_t gameVersion = 0;
        double framerate = 240.0;
        int32_t seed = 0;
        int32_t coins = 0;
        bool ldm = false, platformer = false;
        struct { std::string name; int32_t version; } botInfo
