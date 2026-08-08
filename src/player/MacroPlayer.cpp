#include "MacroPlayer.hpp"
#include "../DeepBot.hpp"
#include <Geode/modify/PlayLayer.hpp>
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

    while (m_currentIndex < m_frames.size() &&
           m_frames[m_currentIndex].absoluteTime <= currentTime) {

        if (static_cast<int>(m_currentIndex) == m_lastProcessedIndex) {
            m_currentIndex++;
            continue;
        }

        const auto& frame = m_frames[m_currentIndex];
        auto* playLayer = PlayLayer::get();
        if (playLayer) {
            uint8_t btn = frame.button;
            if (btn == 0 || btn > 3) btn = 1;
            int button = (btn == 2) ? 2 : (btn == 3 ? 3 : 1);
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
    if (m_currentIndex < m_frames.size()) {
        return m_frames[m_currentIndex];
    }
    return TPSIndependentFrame{};
}

void MacroPlayer::advance() {
    if (m_currentIndex < m_frames.size()) m_currentIndex++;
}

class $modify(PlayLayerPlayer, PlayLayer) {
    void update(float dt) {
        PlayLayer::update(dt);
        auto& player = deepbot::DeepBot::instance().getPlayer();
        if (!player.isPlaying()) return;

        double levelTime = 0.0;
        #if defined(GEODE_IS_WINDOWS)
            levelTime = this->m_gameState.m_levelTime;
        #else
            levelTime = this->m_gameState.m_levelTime;
        #endif
        player.update(levelTime);
    }

    void resetLevel() {
        auto& player = deepbot::DeepBot::instance().getPlayer();
        if (player.isPlaying()) {
            player.stopPlayback();
        }
        PlayLayer::resetLevel();
    }
};

} // namespace deepbot
