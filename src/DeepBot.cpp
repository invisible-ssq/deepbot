#include "DeepBot.hpp"
#include "formats/DeepParser.hpp"
#include <fstream>
#include <ctime>

using namespace geode::prelude;

namespace deepbot {

static double getDefaultTPS() {
    return Mod::get()->getSettingValue<double>("default-tps");
}

void DeepBot::startRecording() {
    auto* playLayer = PlayLayer::get();
    if (!playLayer) return;

    double tps = getDefaultTPS();
    uint32_t seed = 0;

    #if defined(GEODE_IS_WINDOWS)
    seed = playLayer->m_gameState.m_unkRandSeed;
    #else
    seed = 0;
    #endif

    m_recorder.startRecording(tps, seed);
    m_currentMacro.clear();
}

void DeepBot::stopRecording() {
    m_recorder.stopRecording();
    m_currentMacro = m_recorder.getFrames();
}

void DeepBot::startPlayback() {
    if (m_currentMacro.empty()) return;
    m_player.loadMacro(m_currentMacro);
    m_player.startPlayback();
}

void DeepBot::stopPlayback() {
    m_player.stopPlayback();
}

bool DeepBot::saveToFile(const std::string& path, const std::string& format) {
    try {
        auto unified = toUnified();
        auto result = DeepParser::serialize(unified, format);
        if (result.isErr()) {
            log::error("deepbot save failed: {}", result.unwrapErr());
            return false;
        }
        auto data = result.unwrap();

        std::ofstream file(path, std::ios::binary);
        if (!file) return false;

        file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
        if (!file) return false;
        file.flush();
        return file.good();
    } catch (const std::exception& e) {
        log::error("deepbot save failed: {}", e.what());
        return false;
    }
}

bool DeepBot::loadFromFile(const std::string& path, const std::string& format) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) return false;

    auto size = file.tellg();
    if (size <= 0 || size > static_cast<std::streamsize>(1024 * 1024 * 100)) {
        return false;
    }
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(data.data()), size)) return false;

    try {
        std::string fmt = format;
        if (fmt.empty()) {
            fmt = DeepParser::detectFormat(path, data);
        }
        if (fmt.empty()) {
            log::error("deepbot: could not detect format for {}", path);
            return false;
        }

        auto result = DeepParser::parseFormat(fmt, data);
        if (result.isErr()) {
            log::error("deepbot load failed: {}", result.unwrapErr());
            return false;
        }
        fromUnified(result.unwrap());
        return true;
    } catch (const std::exception& e) {
        log::error("deepbot load failed: {}", e.what());
        return false;
    }
}

bool DeepBot::convertFormat(const std::string& srcPath, const std::string& srcFormat,
                            const std::string& dstPath, const std::string& dstFormat) {
    auto oldMacro = m_currentMacro;
    if (!loadFromFile(srcPath, srcFormat)) {
        m_currentMacro = oldMacro;
        return false;
    }
    bool ok = saveToFile(dstPath, dstFormat);
    if (!ok) m_currentMacro = oldMacro;
    return ok;
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
