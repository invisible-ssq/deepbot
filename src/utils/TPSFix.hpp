#pragma once
#include <cstdint>
#include <vector>

namespace deepbot {

struct TPSIndependentFrame {
    double absoluteTime;
    bool down;
    bool player2;
    uint8_t button;
    float x, y, rotation, yAccel;
};

class TPSConverter {
public:
    static uint32_t timeToFrame(double time, double tps) {
        return static_cast<uint32_t>(time * tps);
    }

    static double frameToTime(uint32_t frame, double tps) {
        return static_cast<double>(frame) / tps;
    }

    static std::vector<TPSIndependentFrame> convertTPS(
        const std::vector<TPSIndependentFrame>& inputs,
        double sourceTPS,
        double targetTPS
    ) {
        if (sourceTPS == targetTPS) return inputs;
        
        std::vector<TPSIndependentFrame> result;
        result.reserve(inputs.size());
        
        for (const auto& input : inputs) {
            auto converted = input;
            // Convert: frame = time * sourceTPS, newTime = frame / targetTPS
            uint32_t frame = timeToFrame(input.absoluteTime, sourceTPS);
            converted.absoluteTime = frameToTime(frame, targetTPS);
            result.push_back(converted);
        }
        return result;
    }
};

} // namespace deepbot
