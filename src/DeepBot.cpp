#include "DeepBot.hpp"
#include "formats/DeepParser.hpp"
#include <Geode/Geode.hpp>
#include <fstream>

using namespace geode::prelude;

namespace deepbot {

static double getDefaultTPS() {
    return static_cast<double>(Mod::get()->getSettingValue<int64_t>("default-tps"));
}

void DeepBot::startRecording() {
    auto* playLayer = PlayLayer::get();
    if (!playLayer) return;

    double tps = getDefaultTPS();
    m_recorder.startRecording(tps, playLayer->m_gameState.m_unkRandSeed);
    m_currentMacro.clear();
}

void DeepBot::stopRecording() {
    m_recorder.stopRecording();
    m_currentMacro = m_recorder.getFrames();
}

void DeepBot::startPlayback() {
    if (m_currentMacro.empty()) return;
    m_player.loadMacro(m_currentMacro);
    m_player.startPlayback(getDefaultTPS());
}

void DeepBot::stopPlayback() {
    m_player.stopPlayback();
}

bool DeepBot::saveToFile(const std::string& path, const std::string& format) {
    try {
        auto unified = toUnified();
        auto data = DeepParser::serialize(unified, format);
        std::ofstream file(path, std::ios::binary);
        if (!file) return false;
        file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
        return true;
    } catch (const std::exception& e) {
        log::error("deepbot save failed: {}", e.what());
        return false;
    }
}

bool DeepBot::loadFromFile(const std::string& path, const std::string& format) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;

    std::vector<uint8_t> data(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    try {
        auto unified = DeepParser::parseFormat(format, data);
        fromUnified(unified);
        return true;
    } catch (const std::exception& e) {
        log::error("deepbot load failed: {}", e.what());
        return false;
    }
}

bool DeepBot::convertFormat(const std::string& srcPath, const std::string& srcFormat,
                            const std::string& dstPath, const std::string& dstFormat) {
    if (!loadFromFile(srcPath, srcFormat)) return false;
    return saveToFile(dstPath, dstFormat);
}

double DeepBot::getDuration() const {
    if (m_currentMacro.empty()) return 0.0;
    return m_currentMacro.back().absoluteTime;
}

UnifiedReplay DeepBot::toUnified() const {
    UnifiedReplay replay;
    replay.tps = getDefaultTPS();
    replay.duration = getDuration();
    for (const auto& frame : m_currentMacro) {
        UnifiedInput input;
        input.absoluteTime = frame.absoluteTime;
        input.down = frame.down;
        input.player2 = frame.player2;
        input.button = frame.button;
        input.x = frame.x;
        input.y = frame.y;
        input.rotation = frame.rotation;
        input.yAccel = frame.yAccel;
        replay.inputs.push_back(input);
    }
    return replay;
}

void DeepBot::fromUnified(const UnifiedReplay& replay) {
    m_currentMacro.clear();
    m_currentMacro.reserve(replay.inputs.size());
    for (const auto& input : replay.inputs) {
        TPSIndependentFrame frame;
        frame.absoluteTime = input.absoluteTime;
        frame.down = input.down;
        frame.player2 = input.player2;
        frame.button = input.button;
        frame.x = input.x;
        frame.y = input.y;
        frame.rotation = input.rotation;
        frame.yAccel = input.yAccel;
        m_currentMacro.push_back(frame);
    }
}

} // namespace deepbot