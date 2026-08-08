#include "MacroPlayer.hpp"
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

namespace deepbot {

void MacroPlayer::loadMacro(const std::vector<TPSIndependentFrame>& frames) {
    m_frames = frames;
    m_currentIndex = 0;
}

void MacroPlayer::startPlayback(double tps) {
    m_playing = true;
    m_currentTPS = tps;
    m_startTime = 0.0;
    m_currentIndex = 0;
}

void MacroPlayer::stopPlayback() {
    m_playing = false;
}

void MacroPlayer::update(double currentTime) {
    if (!m_playing) return;
    while (m_currentIndex < m_frames.size() &&
           m_frames[m_currentIndex].absoluteTime <= currentTime) {
        const auto& frame = m_frames[m_currentIndex];
        auto* playLayer = PlayLayer::get();
        if (playLayer) {
            int button = frame.button == 2 ? 2 : (frame.button == 3 ? 3 : 1);
            playLayer->handleButton(frame.down, button, frame.player2);
        }
        m_currentIndex++;
    }
    if (m_currentIndex >= m_frames.size()) {
        m_playing = false;
    }
}

bool MacroPlayer::hasNextFrame() const {
    return m_currentIndex < m_frames.size();
}

TPSIndependentFrame MacroPlayer::getNextFrame() const {
    return m_frames[m_currentIndex];
}

void MacroPlayer::advance() {
    if (m_currentIndex < m_frames.size()) m_currentIndex++;
}

class $modify(PlayLayerPlayer, PlayLayer) {
    void update(float dt) {
        PlayLayer::update(dt);
        auto& player = DeepBot::instance().getPlayer();
        if (!player.isPlaying()) return;
        player.update(m_gameState.m_currentProgress);
    }
};

} // namespace deepbot