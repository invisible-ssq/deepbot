#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>

namespace deepbot {

class DeepBotUI : public geode::Popup<> {
protected:
    bool setup() override;

public:
    static DeepBotUI* create();
    void onRecord(CCObject*);
    void onStop(CCObject*);
    void onPlay(CCObject*);
    void onSave(CCObject*);
    void onLoad(CCObject*);
    void onConvert(CCObject*);
    void onSettings(CCObject*);
    void updateStatus(const std::string& status);

private:
    cocos2d::CCLabelBMFont* m_statusLabel = nullptr;
    cocos2d::CCMenu* m_buttonMenu = nullptr;
};

class $modify(PauseLayerDeepBot, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto* btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("deepbot", 80, true, "bigFont.fnt", "GJ_button_01.png", 30, 1.0f),
            this,
            menu_selector(PauseLayerDeepBot::onDeepBot)
        );
        auto* menu = CCMenu::create();
        menu->addChild(btn);
        menu->setPosition(winSize.width - 50, winSize.height - 50);
        addChild(menu);
    }
    void onDeepBot(CCObject*) {
        DeepBotUI::create()->show();
    }
};

} // namespace deepbot