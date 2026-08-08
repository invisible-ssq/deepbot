#include "DeepBotUI.hpp"
#include "../DeepBot.hpp"

using namespace geode::prelude;

namespace deepbot {

bool DeepBotUI::setup() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    setTitle("deepbot");
    m_statusLabel = CCLabelBMFont::create("Ready", "bigFont.fnt");
    m_statusLabel->setPosition(winSize.width / 2, winSize.height / 2 + 80);
    m_statusLabel->setScale(0.6f);
    addChild(m_statusLabel);
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
    addChild(m_buttonMenu);
    return true;
}

DeepBotUI* DeepBotUI::create() {
    auto* ret = new DeepBotUI();
    if (ret && ret->init(400, 280)) {
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
    bot.startPlayback();
    updateStatus("Playing...");
}

void DeepBotUI::onSave(CCObject*) {
    auto files = file::pickFile(
        file::PickMode::SaveFile,
        {"deep", "ttr3", "gdr", "gdr2", "slc", "cml", "xd", "ybot", "tcm", "re"}
    );
    if (files.empty()) return;
    auto& bot = DeepBot::instance();
    auto ext = files.extension().string();
    if (ext[0] == '.') ext = ext.substr(1);
    if (bot.saveToFile(files.string(), ext)) {
        updateStatus("Saved to " + files.filename().string());
    } else {
        updateStatus("Save failed!");
    }
}

void DeepBotUI::onLoad(CCObject*) {
    auto files = file::pickFile(
        file::PickMode::OpenFile,
        {"deep", "ttr3", "gdr", "gdr2", "slc", "cml", "xd", "ybot", "tcm", "re",
         "re2", "re3", "re4", "zbf", "mhr", "echo", "txt", "ybf"}
    );
    if (files.empty()) return;
    auto& bot = DeepBot::instance();
    auto ext = files.extension().string();
    if (ext[0] == '.') ext = ext.substr(1);
    if (bot.loadFromFile(files.string(), ext)) {
        updateStatus("Loaded " + std::to_string(bot.getFrameCount()) + " frames");
    } else {
        updateStatus("Load failed!");
    }
}

void DeepBotUI::onConvert(CCObject*) {
    auto* alert = FLAlertLayer::create(
        this,
        "Convert Format",
        "Choose source and target formats",
        "OK",
        nullptr,
        300
    );
    alert->show();
}

void DeepBotUI::onSettings(CCObject*) {
    geode::openSettingsPopup(Mod::get());
}

void DeepBotUI::updateStatus(const std::string& status) {
    m_statusLabel->setString(status.c_str());
}

} // namespace deepbot