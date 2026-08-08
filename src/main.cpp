#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
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

// ===== Хук на инпуты в уровне =====
class $modify(DeepBotPlayLayer, PlayLayer) {
    void handleButton(bool down, int button, bool player1) {
        PlayLayer::handleButton(down, button, player1);
        
        auto& bot = deepbot::DeepBot::instance();
        if (bot.isRecording()) {
            bool player2 = !player1;
            bot.getRecorder().recordInput(down, player2, button);
        }
    }
    
    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        
        auto& bot = deepbot::DeepBot::instance();
        if (bot.isRecording() && m_player1) {
            auto pos = m_player1->getPosition();
            bot.getRecorder().recordFrame(pos.x, pos.y, m_player1->getRotation(), m_player1->m_yVelocity);
        }
    }
};
