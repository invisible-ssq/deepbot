#pragma once
#include <vector>
#include <cstdint>
#include <array>
#include "../utils/TPSFix.hpp"

namespace deepbot {

class InputRecorder {
private:
    bool m_recording = false;
    std::vector<TPSIndependentFrame> m_frames;
    double m_currentTPS = 240.0;
    uint32_t m_seed = 0;
    std::array<bool, 4> m_lastP1Down = {false, false, false, false};
    std::array<bool, 4> m_lastP2Down = {false, false, false, false};

public:
    void startRecording(double tps, uint32_t seed);
    void stopRecording();
    bool isRecording() const { return m_recording; }
    void recordInput(bool down, bool player2, uint8_t button,
        float x, float y, float rot, float yAccel);
    const std::vector<TPSIndependentFrame>& getFrames() const { return m_frames; }
    void clear() { m_frames.clear(); }
    double getDuration() const;
    void setTPS(double tps) { m_currentTPS = tps; }
};

} // namespace deepbot
