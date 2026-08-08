#include "MacroPlayer.hpp"
#include <algorithm>

using namespace geode::prelude;

namespace deepbot {

void MacroPlayer::loadMacro(const std::vector<TPSIndependentFrame>& frames) {
    m_frames = frames;
    m_currentIndex = 0;
    m_lastProcessedIndex = -1;
}

void MacroPlayer::startPlayback() {
    m_playing = true;
    m_currentIndex = 0;
    m_lastProcessedIndex = -1;
}

void MacroPlayer::stopPlayback() {
    m_playing = false;
}

void MacroPlayer::update(double currentTime) {
    if (!m_playing) return;

    // Process all frames up to current time
    while (m_currentIndex < m_frames.size() &&
           m_frames[m_currentIndex].absoluteTime <= currentTime) {

        // Skip if already processed (dedup by index, not time)
        if (static_cast<int>(m_currentIndex) == m_lastProcessedIndex) {
            m_currentIndex++;
            continue;
        }

        const auto& frame = m_frames[m_currentIndex];
        auto* playLayer = PlayLayer::get();
        if (playLayer) {
            int button = frame.button == 2 ? 2 : (frame.button == 3 ? 3 : 1);
            // 3rd param: isPlayer1 (true = P1, false = P2)
            playLayer->handleButton(frame.down, button, !frame.player2);
        }
        m_lastProcessedIndex = static_cast<int>(m_currentIndex);
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

        // Use level time in seconds
        double levelTime = this->m_gameState.m_levelTime;
        player.update(levelTime);
    }

    void resetLevel() {
        auto& player = DeepBot::instance().getPlayer();
        if (player.isPlaying()) {
            player.stopPlayback();
        }
        PlayLayer::resetLevel();
    }
};

} // namespace deepbot
