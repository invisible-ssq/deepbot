#include "MacroPlayer.hpp"
#include <algorithm>

using namespace geode::prelude;

namespace deepbot {

void MacroPlayer::loadMacro(const std::vector<TPSIndependentFrame>& frames) {
    m_frames = frames;
    m_currentIndex = 0;
}

void MacroPlayer::startPlayback() {
    m_playing = true;
    m_currentIndex = 0;
    m_lastProcessedTime = -1.0;
}

void MacroPlayer::stopPlayback() {
    m_playing = false;
}

void MacroPlayer::update(double currentTime) {
    if (!m_playing) return;
    
    // Process only frames we haven't processed yet
    while (m_currentIndex < m_frames.size() &&
           m_frames[m_currentIndex].absoluteTime <= currentTime) {
        
        // Skip if same time as last processed (deduplication)
        if (m_frames[m_currentIndex].absoluteTime == m_lastProcessedTime) {
            m_currentIndex++;
            continue;
        }
        
        const auto& frame = m_frames[m_currentIndex];
        auto* playLayer = PlayLayer::get();
        if (playLayer) {
            int button = frame.button == 2 ? 2 : (frame.button == 3 ? 3 : 1);
            // 3rd param is isPlayer1, so pass !player2
            playLayer->handleButton(frame.down, button, !frame.player2);
        }
        m_lastProcessedTime = frame.absoluteTime;
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
        
        // Use level time in seconds instead of progress
        double levelTime = this->m_gameState.m_levelTime;
        player.update(levelTime);
    }
};

} // namespace deepbot
