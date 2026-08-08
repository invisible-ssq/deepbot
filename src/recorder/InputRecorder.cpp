#include "InputRecorder.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace deepbot {

void InputRecorder::startRecording(double tps, uint32_t seed) {
    m_recording = true;
    m_tps = tps;
    m_seed = seed;
    m_frames.clear();
    m_startTime = getCurrentTime();
}

void InputRecorder::stopRecording() {
    m_recording = false;
}

double InputRecorder::getCurrentTime() {
    return CCDirector::sharedDirector()->getAnimationInterval() * 
           CCDirector::sharedDirector()->getTotalFrames();
}

void InputRecorder::recordInput(bool down, bool player2, int button) {
    if (!m_recording) return;
    
    RecordedFrame frame;
    frame.absoluteTime = getCurrentTime() - m_startTime;
    frame.down = down;
    frame.player2 = player2;
    frame.button = button;
    frame.x = 0;
    frame.y = 0;
    frame.rotation = 0;
    frame.yAccel = 0;
    m_frames.push_back(frame);
}

void InputRecorder::recordFrame(float x, float y, float rotation, float yAccel) {
    if (!m_recording || m_frames.empty()) return;
    
    auto& last = m_frames.back();
    last.x = x;
    last.y = y;
    last.rotation = rotation;
    last.yAccel = yAccel;
}

} // namespace deepbot
