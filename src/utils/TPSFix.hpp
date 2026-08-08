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
        return frame / tps;
    }

    static std::vector<TPSIndependentFrame> convertTPS(
        const std::vector<TPSIndependentFrame>& inputs,
        double sourceTPS,
        double targetTPS
    ) {
        std::vector<TPSIndependentFrame> result;
        result.reserve(inputs.size());
        for (const auto& input : inputs) {
            auto converted = input;
            result.push_back(converted);
        }
        return result;
    }
};

} // namespace deepbot