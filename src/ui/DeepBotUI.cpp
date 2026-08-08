#include "DeepBotUI.hpp"
#include "../DeepBot.hpp"
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/utils/file.hpp>

using namespace geode::prelude;

namespace deepbot {

bool DeepBotUI::setup() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    this->setTitle("deepbot");

    m_statusLabel = CCLabelBMFont::create("Ready", "bigFont.fnt");
    m_statusLabel->setPosition(winSize.width / 2, winSize.height / 2 + 80);
    m_statusLabel->setScale(0.6f);
    this->addChild(m_statusLabel);

    m_buttonMenu = CCMenu::create();
    m_buttonMenu->setPosition(winSize.width / 2, winSize.height / 2);

    auto createBtn = [&](const char* text, SEL_MenuHandler handler, float x, float y) {
        auto* btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(text, 100, true, "bigFont.fnt", "GJ_button_01.png", 40, 0.8f),
            this,
            handler
        );
        btn->setPosition(x, y);
        return btn;
    };

    m_buttonMenu->addChild(createBtn("Record", menu_selector(DeepBotUI::onRecord), -80, 40));
    m_buttonMenu->addChild(createBtn("Stop", menu_selector(DeepBotUI::onStop), 80, 40));
    m_buttonMenu->addChild(createBtn("Play", menu_selector(DeepBotUI::onPlay), -80, -20));
    m_buttonMenu->addChild(createBtn("Save", menu_selector(DeepBotUI::onSave), 80, -20));
    m_buttonMenu->addChild(createBtn("Load", menu_selector(DeepBotUI::onLoad), -80, -80));
    m_buttonMenu->addChild(createBtn("Convert", menu_selector(DeepBotUI::onConvert), 80, -80));

    this->addChild(m_buttonMenu);
    return true;
}

DeepBotUI* DeepBotUI::create() {
    auto* ret = new DeepBotUI();
    if (ret && ret->init(400.f, 280.f)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void DeepBotUI::onRecord(CCObject*) {
    auto& bot = DeepBot::instance();
    if (bot.isRecording()) return;

    auto* playLayer = PlayLayer::get();
    if (!playLayer) {
        updateStatus("Not in level!");
        return;
    }

    bot.startRecording();
    updateStatus("Recording...");
}

void DeepBotUI::onStop(CCObject*) {
    auto& bot = DeepBot::instance();
    if (!bot.isRecording()) return;

    bot.stopRecording();
    updateStatus("Stopped. " + std::to_string(bot.getFrameCount()) + " frames");
}

void DeepBotUI::onPlay(CCObject*) {
    auto& bot = DeepBot::instance();
    if (bot.isPlaying()) return;

    if (bot.getFrameCount() == 0) {
        updateStatus("No macro loaded!");
        return;
    }

    bot.startPlayback();
    updateStatus("Playing...");
}

void DeepBotUI::onSave(CCObject*) {
    auto result = file::pick(file::PickMode::SaveFile, {
        .filters = {
            { .description = "deepbot / supported macros", .files = {
                "*.deep", "*.ttr3", "*.gdr", "*.gdr2", "*.slc",
                "*.xd", "*.ybot", "*.tcm", "*.re", "*.zbf", "*.mhr", "*.echo", "*.txt"
            }}
        }
    });

    if (result.isErr()) return;
    auto path = result.unwrap();

    auto& bot = DeepBot::instance();
    std::string ext = path.extension().string();
    if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);

    if (ext.empty()) {
        ext = "deep";
        path.replace_extension(".deep");
    }

    if (bot.saveToFile(path.string(), ext)) {
        updateStatus("Saved to " + path.filename().string());
    } else {
        updateStatus("Save failed!");
    }
}

void DeepBotUI::onLoad(CCObject*) {
    auto result = file::pick(file::PickMode::OpenFile, {
        .filters = {
            { .description = "Macro files", .files = {
                "*.deep", "*.ttr3", "*.gdr", "*.gdr2", "*.slc", "*.cml",
                "*.xd", "*.ybot", "*.ybf", "*.tcm", "*.re", "*.re2", "*.re3", "*.re4",
                "*.zbf", "*.mhr", "*.echo", "*.txt"
            }}
        }
    });

    if (result.isErr()) return;
    auto path = result.unwrap();

    auto& bot = DeepBot::instance();
    std::string ext = path.extension().string();
    if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);

    if (bot.loadFromFile(path.string(), ext)) {
        updateStatus("Loaded " + std::to_string(bot.getFrameCount()) + " frames");
    } else {
        updateStatus("Load failed!");
    }
}

void DeepBotUI::onConvert(CCObject*) {
    FLAlertLayer::create(
        "Convert",
        "Load a macro first, then use Save and choose the target format.",
        "OK"
    )->show();
}

void DeepBotUI::onSettings(CCObject*) {
    geode::openSettingsPopup(Mod::get());
}

void DeepBotUI::updateStatus(const std::string& status) {
    if (m_statusLabel) {
        m_statusLabel->setString(status.c_str());
    }
}

// Pause menu button (respects "show-button" setting)
class $modify(PauseLayerDeepBot, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto showButton = Mod::get()->getSettingValue<bool>("show-button");
        if (!showButton) return;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto* btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("deepbot", 80, true, "bigFont.fnt", "GJ_button_01.png", 30, 1.0f),
            this,
            menu_selector(PauseLayerDeepBot::onDeepBot)
        );

        auto* menu = CCMenu::create();
        menu->addChild(btn);
        menu->setPosition(winSize.width - 50.f, winSize.height - 50.f);
        this->addChild(menu);
    }

    void onDeepBot(CCObject*) {
        DeepBotUI::create()->show();
    }
};

} // namespace deepbot