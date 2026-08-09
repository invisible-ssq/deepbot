#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "DeepBot.hpp"
#include "ui/DeepBotUI.hpp"
#include "formats/DeepParser.hpp"

using namespace geode::prelude;

$execute {
    auto formats = deepbot::DeepParser::getSupportedFormats();
    std::string formatList;
    for (const auto& fmt : formats) {
        if (!formatList.empty()) formatList += ", ";
        formatList += fmt;
    }
    log::info("deepbot loaded! Supported formats: {}", formatList);
}

$on_mod(Loaded) {
    auto version = Mod::get()->getVersion();
    log::info("deepbot {}.{}.{} by goodxdeveloper", version.getMajor(), version.getMinor(), version.getPatch());
}

// ===== Хук на инпуты через PlayerObject =====
class $modify(DeepBotPlayerObject, PlayerObject) {
    void pushButton(PlayerButton button) {
        PlayerObject::pushButton(button);
        
        auto* playLayer = PlayLayer::get();
        if (!playLayer) return;
        
        auto& bot = deepbot::DeepBot::instance();
        if (bot.isRecording()) {
            bool player2 = (this == playLayer->m_player2);
            bot.getRecorder().recordInput(true, player2, static_cast<int>(button));
        }
    }
    
    void releaseButton(PlayerButton button) {
        PlayerObject::releaseButton(button);
        
        auto* playLayer = PlayLayer::get();
        if (!playLayer) return;
        
        auto& bot = deepbot::DeepBot::instance();
        if (bot.isRecording()) {
            bool player2 = (this == playLayer->m_player2);
            bot.getRecorder().recordInput(false, player2, static_cast<int>(button));
        }
    }
};

// ===== Хук на позицию =====
class $modify(DeepBotPlayLayer, PlayLayer) {
    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        
        auto& bot = deepbot::DeepBot::instance();
        if (bot.isRecording() && m_player1) {
            auto pos = m_player1->getPosition();
            bot.getRecorder().recordFrame(pos.x, pos.y, m_player1->getRotation(), m_player1->m_yVelocity);
        }
    }
};
