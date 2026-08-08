#include "DeepBotUI.hpp"
#include "../DeepBot.hpp"
#include <algorithm>

using namespace geode::prelude;

namespace deepbot {

class DeepBotButton : public CCMenuItemSpriteExtra {
public:
    static DeepBotButton* create() {
        auto* btn = new DeepBotButton();
        if (btn && btn->init()) {
            btn->autorelease();
            return btn;
        }
        delete btn;
        return nullptr;
    }

    bool init() override {
        auto* bg = CCSprite::create("GJ_button_01.png");
        if (!bg) {
            bg = CCSprite::create();
            auto* circle = CCDrawNode::create();
            circle->drawDot(ccp(0, 0), 25, ccc4FFromccc3B({100, 150, 255}));
            bg->addChild(circle);
        }
        bg->setScale(2.5f);

        auto* label = CCLabelBMFont::create("DB", "bigFont.fnt");
        label->setScale(0.4f);
        label->setPosition(bg->getContentSize() / 2);
        bg->addChild(label);

        if (!CCMenuItemSpriteExtra::init(bg, nullptr, this, menu_selector(DeepBotButton::onClick))) {
            return false;
        }

        m_draggable = true;
        m_dragging = false;
        m_dragStartPos = ccp(0, 0);
        m_dragStartTouch = ccp(0, 0);

        return true;
    }

    void onClick(CCObject*) {
        if (!m_dragging) {
            DeepBotUI::create()->show();
        }
    }

    void selected() override {
        CCMenuItemSpriteExtra::selected();
        m_dragging = false;
    }

    void unselected() override {
        CCMenuItemSpriteExtra::unselected();
        m_dragging = false;
    }

    void activate() override {
        if (!m_dragging) {
            CCMenuItemSpriteExtra::activate();
        }
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
        if (!isVisible() || !m_bEnabled) return false;
        
        auto pos = convertToNodeSpace(touch->getLocation());
        auto rect = CCRect(0, 0, getContentSize().width, getContentSize().height);
        
        if (rect.containsPoint(pos)) {
            m_dragging = false;
            m_dragStartPos = getPosition();
            m_dragStartTouch = touch->getLocation();
            selected();
            return true;
        }
        return false;
    }

    void ccTouchMoved(CCTouch* touch, CCEvent* event) override {
        auto delta = ccpSub(touch->getLocation(), m_dragStartTouch);
        if (ccpLength(delta) > 10) {
            m_dragging = true;
        }
        if (m_dragging) {
            auto newPos = ccpAdd(m_dragStartPos, delta);
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            auto halfW = getContentSize().width * getScale() / 2;
            auto halfH = getContentSize().height * getScale() / 2;
            newPos.x = std::clamp(newPos.x, halfW, winSize.width - halfW);
            newPos.y = std::clamp(newPos.y, halfH, winSize.height - halfH);
            setPosition(newPos);
        }
    }

    void ccTouchEnded(CCTouch* touch, CCEvent* event) override {
        unselected();
        if (!m_dragging) {
            activate();
        }
        m_dragging = false;
    }

    void ccTouchCancelled(CCTouch* touch, CCEvent* event) override {
        unselected();
        m_dragging = false;
    }

private:
    bool m_draggable = true;
    bool m_dragging = false;
    CCPoint m_dragStartPos;
    CCPoint m_dragStartTouch;
};

class $modify(MenuLayerDeepBot, MenuLayer) {
    bool init() override {
        if (!MenuLayer::init()) return false;

        if (!Mod::get()->isEnabled()) return true;
        if (!Mod::get()->getSettingValue<bool>("show-button")) return true;

        if (this->getChildByID("deepbot-menu"_spr)) return true;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        
        auto* menu = CCMenu::create();
        menu->setPosition(0, 0);
        menu->setID("deepbot-menu"_spr);
        
        auto* btn = DeepBotButton::create();
        btn->setPosition(winSize.width - 50, winSize.height - 100);
        btn->setID("deepbot-button"_spr);
        menu->addChild(btn);
        
        this->addChild(menu, 100);
        return true;
    }
};

class $modify(PauseLayerDeepBot, PauseLayer) {
    void customSetup() override {
        PauseLayer::customSetup();

        if (!Mod::get()->isEnabled()) return;
        if (!Mod::get()->getSettingValue<bool>("show-button")) return;

        if (this->getChildByID("deepbot-pause-menu"_spr)) return;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        
        auto* menu = CCMenu::create();
        menu->setPosition(0, 0);
        menu->setID("deepbot-pause-menu"_spr);
        
        auto* btn = DeepBotButton::create();
        btn->setPosition(winSize.width - 50, winSize.height - 50);
        btn->setID("deepbot-pause-button"_spr);
        menu->addChild(btn);
        
        this->addChild(menu, 100);
    }
};

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
    if (ret && ret->initAnchored(400.f, 280.f)) {
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
    if (bot.isRecording()) {
        updateStatus("Stop recording first!");
        return;
    }
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
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

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
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

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

} // namespace deepbot
