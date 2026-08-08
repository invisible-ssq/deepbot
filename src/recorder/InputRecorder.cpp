#include "InputRecorder.hpp"
#include "../DeepBot.hpp"
#include <Geode/modify/PlayLayer.hpp>
#include <array>

using namespace geode::prelude;

namespace deepbot {

void InputRecorder::startRecording(double tps, uint32_t seed) {
    m_recording = true;
    m_frames.clear();
    m_currentTPS = tps;
    m_seed = seed;
    m_lastP1Down.fill(false);
    m_lastP2Down.fill(false);
}

void InputRecorder::stopRecording() {
    m_recording = false;
}

void InputRecorder::recordInput(bool down, bool player2, uint8_t button,
    float x, float y, float rot, float yAccel) {

    if (!m_recording) return;

    if (button == 0 || button > 3) button = 1;

    auto& lastDown = player2 ? m_lastP2Down : m_lastP1Down;
    if (button < lastDown.size() && down == lastDown[button]) {
        return;
    }

    auto* playLayer = PlayLayer::get();
    if (!playLayer) return;

    double time = 0.0;
    #if defined(GEODE_IS_WINDOWS)
        time = playLayer->m_gameState.m_levelTime;
    #else
        time = playLayer->m_gameState.m_levelTime;
    #endif

    TPSIndependentFrame frame;
    frame.absoluteTime = time;
    frame.down = down;
    frame.player2 = player2;
    frame.button = button;
    frame.x = x;
    frame.y = y;
    frame.rotation = rot;
    frame.yAccel = yAccel;
    m_frames.push_back(frame);

    if (button < lastDown.size()) {
        lastDown[button] = down;
    }
}

double InputRecorder::getDuration() const {
    if (m_frames.empty()) return 0.0;
    return m_frames.back().absoluteTime;
}

class $modify(PlayLayerRecorder, PlayLayer) {
    void handleButton(bool down, int button, bool player2) {
        PlayLayer::handleButton(down, button, player2);
        auto& recorder = deepbot::DeepBot::instance().getRecorder();
        if (!recorder.isRecording()) return;

        uint8_t btn = 1;
        if (button == 2) btn = 2;
        if (button == 3) btn = 3;

        float x = 0, y = 0, rot = 0, yAccel = 0;
        auto* player = player2 ? m_player2 : m_player1;
        if (player) {
            x = player->m_position.x;
            y = player->m_position.y;
            rot = player->getRotation();
            yAccel = player->m_yVelocity;
        }
        recorder.recordInput(down, player2, btn, x, y, rot, yAccel);
    }
};

} // namespace deepbot
