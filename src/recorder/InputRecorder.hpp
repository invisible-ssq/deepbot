#pragma once
#include <vector>
#include <cstdint>
#include "../utils/TPSFix.hpp"

namespace deepbot {

class InputRecorder {
public:
    struct RecordedFrame {
        double absoluteTime;
        bool down;
        bool player2;
        int button;
        float x, y;
        float rotation;
        float yAccel;
    };

    void startRecording(double tps, uint32_t seed);
    void stopRecording();
    bool isRecording() const { return m_recording; }
    void recordInput(bool down, bool player2, int button);
    void recordFrame(float x, float y, float rotation, float yAccel);
    const std::vector<RecordedFrame>& getFrames() const { return m_frames; }

private:
    bool m_recording = false;
    double m_tps = 240.0;
    double m_startTime = 0.0;
    uint32_t m_seed = 0;
    std::vector<RecordedFrame> m_frames;
    double getCurrentTime();
};

} // namespace deepbot
