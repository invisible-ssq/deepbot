#include "DeepBotUI.hpp"
#include "../DeepBot.hpp"
#include <Geode/utils/async.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <algorithm>

using namespace geode::prelude;

namespace deepbot {

// ===== Маленькая плавающая кнопка (перетаскиваемая) =====
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
            circle->drawDot(ccp(0, 0), 12, ccc4FFromccc3B({100, 150, 255}));
            bg->addChild(circle);
        }
        bg->setScale(0.9f);

        auto* label = CCLabelBMFont::create("D", "bigFont.fnt");
        label->setScale(0.3f);
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
        if (ccpLength(delta) > 5) {
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

// ===== ТОЛЬКО на паузе в уровне =====
class $modify(PauseLayerDeepBot, PauseLayer) {
    void customSetup() override {
        PauseLayer::customSetup();
        addDeepBotButton(this, "deepbot-pause-menu", "deepbot-pause-button", ccp(35, 35));
    }
};

// ===== Компактное меню =====
bool DeepBotUI::init() {
    if (!FLAlertLayer::init(150)) return false;

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    if (m_mainLayer) {
        m_mainLayer->removeAllChildrenWithCleanup(true);
    }

    auto* bg = CCScale9Sprite::create("GJ_square01.png");
    bg->setContentSize({ 240, 130 });
    bg->setPosition(winSize.width / 2, winSize.height / 2);
    m_mainLayer->addChild(bg);

    auto* title = CCLabelBMFont::create("DeepBot", "bigFont.fnt");
    title->setScale(0.35f);
    title->setPosition(winSize.width / 2, winSize.height / 2 + 50);
    m_mainLayer->addChild(title);

    m_statusLabel = CCLabelBMFont::create("Ready", "goldFont.fnt");
    m_statusLabel->setScale(0.28f);
    m_statusLabel->setPosition(winSize.width / 2, winSize.height / 2 + 32);
    m_mainLayer->addChild(m_statusLabel);

    auto* closeBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
        this,
        menu_selector(DeepBotUI::onClose)
    );
    closeBtn->setScale(0.5f);
    closeBtn->setPosition(winSize.width / 2 + 100, winSize.height / 2 + 50);

    auto* closeMenu = CCMenu::create();
    closeMenu->setPosition(0, 0);
    closeMenu->addChild(closeBtn);
    m_mainLayer->addChild(closeMenu);

    auto* row1 = CCMenu::create();
    row1->setPosition(winSize.width / 2, winSize.height / 2 + 5);

    auto createBtn = [&](const char* text, SEL_MenuHandler handler) {
        auto* spr = ButtonSprite::create(text, 50, true, "bigFont.fnt", "GJ_button_01.png", 20, 0.5f);
        auto* btn = CCMenuItemSpriteExtra::create(spr, this, handler);
        return btn;
    };

    m_recordBtn = createBtn("Rec", menu_selector(DeepBotUI::onRecord));
    m_recordBtn->setPosition(-60, 0);
    row1->addChild(m_recordBtn);

    m_stopBtn = createBtn("Stop", menu_selector(DeepBotUI::onStop));
    m_stopBtn->setPosition(0, 0);
    m_stopBtn->setVisible(false);
    row1->addChild(m_stopBtn);

    m_playBtn = createBtn("Play", menu_selector(DeepBotUI::onPlay));
    m_playBtn->setPosition(60, 0);
    row1->addChild(m_playBtn);

    m_mainLayer->addChild(row1);

    auto* row2 = CCMenu::create();
    row2->setPosition(winSize.width / 2, winSize.height / 2 - 28);

    auto* saveBtn = createBtn("Save", menu_selector(DeepBotUI::onSave));
    saveBtn->setPosition(-45, 0);
    row2->addChild(saveBtn);

    auto* loadBtn = createBtn("Load", menu_selector(DeepBotUI::onLoad));
    loadBtn->setPosition(45, 0);
    row2->addChild(loadBtn);

    m_mainLayer->addChild(row2);

    auto* ver = CCLabelBMFont::create("v1.0.1", "chatFont.fnt");
    ver->setScale(0.3f);
    ver->setPosition(winSize.width / 2, winSize.height / 2 - 52);
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

// ===== Save: автосохранение + меню форматов =====
void DeepBotUI::onSave(CCObject*) {
    auto& bot = DeepBot::instance();
    if (bot.getFrameCount() == 0) {
        updateStatus("Nothing to save!");
        return;
    }

    // Автосохранение в /game/macros/
    auto macroDir = geode::dirs::getGameDir() / "macros";
    std::error_code ec;
    std::filesystem::create_directories(macroDir, ec);

    auto timestamp = std::to_string(std::time(nullptr));
    auto autoPath = macroDir / ("macro_" + timestamp + ".deep");
    bot.saveToFile(autoPath.string(), "deep");

    // Меню выбора формата
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    
    auto* formatMenu = CCMenu::create();
    formatMenu->setPosition(winSize.width / 2, winSize.height / 2);
    formatMenu->setID("deepbot-format-menu");

    auto* fbg = CCScale9Sprite::create("GJ_square01.png");
    fbg->setContentSize({ 200, 180 });
    fbg->setPosition(0, 0);
    formatMenu->addChild(fbg, -1);

    auto* ftitle = CCLabelBMFont::create("Save As", "bigFont.fnt");
    ftitle->setScale(0.35f);
    ftitle->setPosition(0, 70);
    formatMenu->addChild(ftitle);

    auto* finfo = CCLabelBMFont::create("Auto-saved to macros/", "chatFont.fnt");
    finfo->setScale(0.25f);
    finfo->setPosition(0, 55);
    formatMenu->addChild(finfo);

    const char* formats[] = {".deep", ".ttr3", ".gdr", ".gdr2", ".slc", ".xd", ".ybot", ".txt"};
    float y = 30;
    for (int i = 0; i < 8; i++) {
        auto* btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(formats[i], 70, true, "bigFont.fnt", "GJ_button_01.png", 25, 0.45f),
            this,
            menu_selector(DeepBotUI::onSaveFormat)
        );
        btn->setPosition((i % 2 == 0) ? -50 : 50, y);
        btn->setTag(i);
        formatMenu->addChild(btn);
        if (i % 2 == 1) y -= 25;
    }

    auto* customBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Other...", 100, true, "bigFont.fnt", "GJ_button_04.png", 30, 0.5f),
        this,
        menu_selector(DeepBotUI::onSaveCustomFormat)
    );
    customBtn->setPosition(0, -65);
    formatMenu->addChild(customBtn);

    auto* fcloseBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
        this,
        menu_selector(DeepBotUI::onClose)
    );
    fcloseBtn->setScale(0.5f);
    fcloseBtn->setPosition(85, 75);
    formatMenu->addChild(fcloseBtn);

    this->addChild(formatMenu, 2000);
}

void DeepBotUI::onSaveFormat(CCObject* sender) {
    const char* formats[] = {"deep", "ttr3", "gdr", "gdr2", "slc", "xd", "ybot", "txt"};
    int idx = sender->getTag();
    if (idx < 0 || idx > 7) return;

    auto& bot = DeepBot::instance();
    auto macroDir = geode::dirs::getGameDir() / "macros";
    auto timestamp = std::to_string(std::time(nullptr));
    auto path = macroDir / ("macro_" + timestamp + "." + formats[idx]);

    if (bot.saveToFile(path.string(), formats[idx])) {
        updateStatus("Saved ." + std::string(formats[idx]));
    } else {
        updateStatus("Save failed!");
    }

    this->removeChildByID("deepbot-format-menu");
}

void DeepBotUI::onSaveCustomFormat(CCObject*) {
    geode::async::spawn(
        file::pick(file::PickMode::SaveFile, {
            .defaultPath = "macro",
            .filters = {
                { .description = "All files", .files = { "*" }}
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
