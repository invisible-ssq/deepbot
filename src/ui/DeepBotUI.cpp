#include "DeepBotUI.hpp"
#include "../DeepBot.hpp"
#include <Geode/utils/async.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <algorithm>

using namespace geode::prelude;

namespace deepbot {

// ===== Маленькая плавающая кнопка =====
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
            circle->drawDot(ccp(0, 0), 15, ccc4FFromccc3B({100, 150, 255}));
            bg->addChild(circle);
        }
        bg->setScale(1.2f);

        auto* label = CCLabelBMFont::create("DB", "bigFont.fnt");
        label->setScale(0.25f);
        label->setPosition(bg->getContentSize() / 2);
        bg->addChild(label);

        auto* selectedBg = CCSprite::create("GJ_button_02.png");
        if (!selectedBg) selectedBg = bg;

        if (!CCMenuItemSpriteExtra::init(bg, selectedBg, this, menu_selector(DeepBotButton::onClick))) {
            return false;
        }

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

    bool ccTouchBegan(CCTouch* touch, CCEvent*) {
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

    void ccTouchMoved(CCTouch* touch, CCEvent*) {
        auto delta = ccpSub(touch->getLocation(), m_dragStartTouch);
        if (ccpLength(delta) > 8) {
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

    void ccTouchEnded(CCTouch*, CCEvent*) {
        unselected();
        if (!m_dragging) {
            activate();
        }
        m_dragging = false;
    }

    void ccTouchCancelled(CCTouch*, CCEvent*) {
        unselected();
        m_dragging = false;
    }

private:
    bool m_dragging = false;
    CCPoint m_dragStartPos;
    CCPoint m_dragStartTouch;
};

// ===== Хелпер для добавления кнопки =====
static void addDeepBotButton(CCLayer* layer, const char* menuID, const char* btnID, CCPoint pos) {
    if (layer->getChildByID(menuID)) return;

    auto* menu = CCMenu::create();
    menu->setPosition(0, 0);
    menu->setID(menuID);

    auto* btn = DeepBotButton::create();
    btn->setPosition(pos);
    btn->setID(btnID);
    menu->addChild(btn);

    layer->addChild(menu, 100);
}

// ===== Хуки для кнопки в меню и паузе =====
class $modify(MenuLayerDeepBot, MenuLayer) {
    bool init() override {
        if (!MenuLayer::init()) return false;
        addDeepBotButton(this, "deepbot-menu", "deepbot-button", ccp(40, 40));
        return true;
    }
};

class $modify(PauseLayerDeepBot, PauseLayer) {
    void customSetup() override {
        PauseLayer::customSetup();
        addDeepBotButton(this, "deepbot-pause-menu", "deepbot-pause-button", ccp(40, 40));
    }
};

// ===== Компактное меню =====
bool DeepBotUI::init() {
    if (!FLAlertLayer::init(150)) return false;

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    // Очищаем стандартные элементы FLAlertLayer
    if (m_mainLayer) {
        m_mainLayer->removeAllChildrenWithCleanup(true);
    }

    // Фон — маленький прямоугольник
    auto* bg = CCScale9Sprite::create("GJ_square01.png");
    bg->setContentSize({ 260, 140 });
    bg->setPosition(winSize.width / 2, winSize.height / 2);
    m_mainLayer->addChild(bg);

    // Заголовок
    auto* title = CCLabelBMFont::create("DeepBot", "bigFont.fnt");
    title->setScale(0.4f);
    title->setPosition(winSize.width / 2, winSize.height / 2 + 55);
    m_mainLayer->addChild(title);

    // Статус
    m_statusLabel = CCLabelBMFont::create("Ready", "goldFont.fnt");
    m_statusLabel->setScale(0.3f);
    m_statusLabel->setPosition(winSize.width / 2, winSize.height / 2 + 35);
    m_mainLayer->addChild(m_statusLabel);

    // Кнопка закрытия (X)
    auto* closeBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
        this,
        menu_selector(DeepBotUI::onClose)
    );
    closeBtn->setScale(0.6f);
    closeBtn->setPosition(winSize.width / 2 + 110, winSize.height / 2 + 55);

    auto* closeMenu = CCMenu::create();
    closeMenu->setPosition(0, 0);
    closeMenu->addChild(closeBtn);
    m_mainLayer->addChild(closeMenu);

    // Ряд 1: Rec | Stop | Play
    auto* row1 = CCMenu::create();
    row1->setPosition(winSize.width / 2, winSize.height / 2 + 5);

    auto createBtn = [&](const char* text, ccColor3B color, SEL_MenuHandler handler) {
        auto* spr = ButtonSprite::create(text, 60, true, "bigFont.fnt", "GJ_button_01.png", 25, 0.55f);
        auto* btn = CCMenuItemSpriteExtra::create(spr, this, handler);
        return btn;
    };

    m_recordBtn = createBtn("Rec", {255, 80, 80}, menu_selector(DeepBotUI::onRecord));
    m_recordBtn->setPosition(-70, 0);
    row1->addChild(m_recordBtn);

    m_stopBtn = createBtn("Stop", {255, 200, 80}, menu_selector(DeepBotUI::onStop));
    m_stopBtn->setPosition(0, 0);
    m_stopBtn->setVisible(false);
    row1->addChild(m_stopBtn);

    m_playBtn = createBtn("Play", {80, 255, 80}, menu_selector(DeepBotUI::onPlay));
    m_playBtn->setPosition(70, 0);
    row1->addChild(m_playBtn);

    m_mainLayer->addChild(row1);

    // Ряд 2: Save | Load | Conv
    auto* row2 = CCMenu::create();
    row2->setPosition(winSize.width / 2, winSize.height / 2 - 35);

    auto* saveBtn = createBtn("Save", {150, 150, 255}, menu_selector(DeepBotUI::onSave));
    saveBtn->setPosition(-70, 0);
    row2->addChild(saveBtn);

    auto* loadBtn = createBtn("Load", {150, 255, 150}, menu_selector(DeepBotUI::onLoad));
    loadBtn->setPosition(0, 0);
    row2->addChild(loadBtn);

    auto* convBtn = createBtn("Conv", {255, 200, 100}, menu_selector(DeepBotUI::onConvert));
    convBtn->setPosition(70, 0);
    row2->addChild(convBtn);

    m_mainLayer->addChild(row2);

    // Версия внизу
    auto* ver = CCLabelBMFont::create("v1.0.1", "chatFont.fnt");
    ver->setScale(0.35f);
    ver->setPosition(winSize.width / 2, winSize.height / 2 - 60);
    m_mainLayer->addChild(ver);

    refreshButtons();
    return true;
}

DeepBotUI* DeepBotUI::create() {
    auto* ret = new DeepBotUI();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void DeepBotUI::show() {
    this->setZOrder(1000);
    this->setTouchPriority(-1000);
    FLAlertLayer::show();
}

void DeepBotUI::onClose(CCObject*) {
    this->setKeypadEnabled(false);
    this->removeFromParentAndCleanup(true);
}

void DeepBotUI::refreshButtons() {
    auto& bot = DeepBot::instance();
    if (m_recordBtn) m_recordBtn->setVisible(!bot.isRecording());
    if (m_stopBtn) m_stopBtn->setVisible(bot.isRecording());
}

void DeepBotUI::updateStatus(const std::string& status) {
    if (m_statusLabel) {
        m_statusLabel->setString(status.c_str());
    }
    refreshButtons();
}

// ===== Обработчики =====
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
    updateStatus(std::to_string(bot.getFrameCount()) + " frames");
}

void DeepBotUI::onPlay(CCObject*) {
    auto& bot = DeepBot::instance();
    if (bot.isRecording()) {
        updateStatus("Stop first!");
        return;
    }
    if (bot.isPlaying()) return;
    if (bot.getFrameCount() == 0) {
        updateStatus("No macro!");
        return;
    }

    bot.startPlayback();
    updateStatus("Playing...");
}

void DeepBotUI::onSave(CCObject*) {
    geode::async::spawn(
        file::pick(file::PickMode::SaveFile, {
            .defaultPath = "",
            .filters = {
                { .description = "deepbot macros", .files = { "*.deep" }}
            }
        }),
        [](Result<std::optional<std::filesystem::path>> result) {
            if (result.isErr()) return;
            auto pathOpt = result.unwrap();
            if (!pathOpt.has_value()) return;

            auto& bot = DeepBot::instance();
            std::string ext = pathOpt->extension().string();
            if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
            if (ext.empty()) ext = "deep";

            bot.saveToFile(pathOpt->string(), ext);
        }
    );
}

void DeepBotUI::onLoad(CCObject*) {
    geode::async::spawn(
        file::pick(file::PickMode::OpenFile, {
            .defaultPath = "",
            .filters = {
                { .description = "Macro files", .files = {
                    "*.deep", "*.ttr3", "*.gdr", "*.gdr2", "*.slc", "*.cml",
                    "*.xd", "*.ybot", "*.ybf", "*.tcm", "*.re", "*.re2", "*.re3", "*.re4",
                    "*.zbf", "*.mhr", "*.echo", "*.txt"
                }}
            }
        }),
        [](Result<std::optional<std::filesystem::path>> result) {
            if (result.isErr()) return;
            auto pathOpt = result.unwrap();
            if (!pathOpt.has_value()) return;

            auto& bot = DeepBot::instance();
            std::string ext = pathOpt->extension().string();
            if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            bot.loadFromFile(pathOpt->string(), ext);
        }
    );
}

void DeepBotUI::onConvert(CCObject*) {
    FLAlertLayer::create(
        "Convert",
        "Load a macro first, then use Save and choose the target format.",
        "OK"
    )->show();
}

} // namespace deepbot
