#pragma once
#include <vector>
#include <cstdint>
#include <Geode/Geode.hpp>
#include "../utils/TPSFix.hpp"

namespace deepbot {

class InputRecorder {
private:
    bool m_recording = false;
    std::vector<TPSIndependentFrame> m_frames;
    double m_startTime = 0.0;
    double m_currentTPS = 240.0;
    uint32_t m_seed = 0;
    bool m_lastP1Down = false;
    bool m_lastP2Down = false;

public:
    void startRecording(double tps, uint32_t seed);
    void stopRecording();
    bool isRecording() const { return m_recording; }
    void recordInput(bool down, bool player2, uint8_t button,
                     float x, float y, float rot, float yAccel);
    std::vector<TPSIndependentFrame> getFrames() const { return m_frames; }
    void clear() { m_frames.clear(); }
    double getDuration() const;
    void setTPS(double tps) { m_currentTPS = tps; }
};

} // namespace deepbot