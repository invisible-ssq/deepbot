#pragma once
#include <vector>
#include <cstddef>
#include <cstdint>
#include "../utils/TPSFix.hpp"

namespace deepbot {

class MacroPlayer {
private:
    bool m_playing = false;
    std::vector<TPSIndependentFrame> m_frames;
    size_t m_currentIndex = 0;
    int m_lastProcessedIndex = -1;

public:
    void loadMacro(const std::vector<TPSIndependentFrame>& frames);
    void startPlayback();
    void stopPlayback();
    bool isPlaying() const { return m_playing; }
    void update(double currentTime);
    bool hasNextFrame() const;
    TPSIndependentFrame getNextFrame() const;
    void advance();
};

} // namespace deepbot
