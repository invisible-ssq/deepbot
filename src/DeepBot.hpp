#pragma once
#include <vector>
#include <string>
#include "utils/TPSFix.hpp"
#include "recorder/InputRecorder.hpp"
#include "player/MacroPlayer.hpp"
#include "formats/FormatRegistry.hpp"

namespace deepbot {

class DeepBot {
private:
    InputRecorder m_recorder;
    MacroPlayer m_player;
    std::vector<TPSIndependentFrame> m_currentMacro;
    DeepBot() = default;

public:
    static DeepBot& instance() {
        static DeepBot inst;
        return inst;
    }
    void startRecording();
    void stopRecording();
    bool isRecording() const { return m_recorder.isRecording(); }
    void startPlayback();
    void stopPlayback();
    bool isPlaying() const { return m_player.isPlaying(); }
    bool saveToFile(const std::string& path, const std::string& format);
    bool loadFromFile(const std::string& path, const std::string& format);
    bool convertFormat(const std::string& srcPath, const std::string& srcFormat,
                       const std::string& dstPath, const std::string& dstFormat);
    size_t getFrameCount() const { return m_currentMacro.size(); }
    double getDuration() const;
    InputRecorder& getRecorder() { return m_recorder; }
    MacroPlayer& getPlayer() { return m_player; }
    UnifiedReplay toUnified() const;
    void fromUnified(const UnifiedReplay& replay);
};

} // namespace deepbot
