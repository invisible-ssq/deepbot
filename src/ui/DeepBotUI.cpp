#include "DeepBotUI.hpp"
#include "../DeepBot.hpp"
#include <Geode/utils/async.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <algorithm>

using namespace geode::prelude;

namespace deepbot {

// ===== Кнопка D (перетаскиваемая) =====
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
        bg->setScale(0.85f);

        auto* label = CCLabelBMFont::create("D", "bigFont.fnt");
        label->setScale(0.28f);
        label->setPosition(bg->getContentSize() / 2);
        bg->addChild(label);

        auto* selectedBg = CCSprite::create("GJ_button_02.png");
        if (!selectedBg) selectedBg = bg;

        if (!CCMenuItemSpriteExtra::init(bg, selectedBg, this, menu_selector(DeepBotButton::onClick))) {
            return false;
        }

        m_dragging = false;
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
        if (ccpLength(delta) > 5) m_dragging = true;
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
        if (!m_dragging) activate();
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

class $modify(PauseLayerDeepBot, PauseLayer) {
    void customSetup() override {
        PauseLayer::customSetup();
        addDeepBotButton(this, "deepbot-pause-menu", "deepbot-pause-button", ccp(35, 35));
    }
};

// ===== DeepBotUI =====
bool DeepBotUI::init() {
    if (!CCLayerColor::initWithColor(ccc4(0, 0, 0, 150))) return false;

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    float cx = winSize.width / 2;
    float cy = winSize.height / 2;

    auto* bg = CCScale9Sprite::create("GJ_square01.png");
    bg->setContentSize({ 420, 280 });
    bg->setPosition(cx, cy);
    bg->setColor({ 90, 70, 160 });
    this->addChild(bg);

    auto* title = CCLabelBMFont::create("DeepBot", "bigFont.fnt");
    title->setScale(0.5f);
    title->setPosition(cx, cy + 120);
    this->addChild(title);

    auto* closeMenu = CCMenu::create();
    closeMenu->setPosition(0, 0);
    auto* closeBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
        this,
        menu_selector(DeepBotUI::onClose)
    );
    closeBtn->setScale(0.55f);
    closeBtn->setPosition(cx + 190, cy + 120);
    closeMenu->addChild(closeBtn);
    this->addChild(closeMenu);

    // Вкладки
    m_tabMenu = CCMenu::create();
    m_tabMenu->setPosition(cx, cy + 90);

    auto createTab = [&](const char* text, SEL_MenuHandler handler, float x, bool active) {
        auto* spr = ButtonSprite::create(text, 80, true, "bigFont.fnt", active ? "GJ_button_02.png" : "GJ_button_01.png", 30, 0.55f);
        auto* btn = CCMenuItemSpriteExtra::create(spr, this, handler);
        btn->setPosition(x, 0);
        return btn;
    };

    auto* macroTab = createTab("Macro", menu_selector(DeepBotUI::onTabMacro), -80, true);
    macroTab->setTag(0);
    m_tabMenu->addChild(macroTab);

    auto* settingsTab = createTab("Settings", menu_selector(DeepBotUI::onTabSettings), 80, false);
    settingsTab->setTag(1);
    m_tabMenu->addChild(settingsTab);

    this->addChild(m_tabMenu);

    // ===== Macro Tab =====
    m_macroMenu = CCMenu::create();
    m_macroMenu->setPosition(cx, cy + 10);

    m_actionsLabel = CCLabelBMFont::create("Actions: 0", "chatFont.fnt");
    m_actionsLabel->setScale(0.4f);
    m_actionsLabel->setAnchorPoint({ 0, 0.5f });
    m_actionsLabel->setPosition(-180, 70);
    m_macroMenu->addChild(m_actionsLabel);

    auto* infoBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"),
        this,
        nullptr
    );
    infoBtn->setScale(0.5f);
    infoBtn->setPosition(180, 70);
    m_macroMenu->addChild(infoBtn);

    // Record
    auto* recBg = CCSprite::create("GJ_button_04.png");
    recBg->setScale(1.0f);
    recBg->setColor({ 150, 150, 150 });
    auto* recLabel = CCLabelBMFont::create("Record", "bigFont.fnt");
    recLabel->setScale(0.45f);
    recLabel->setPosition(recBg->getContentSize() / 2);
    recBg->addChild(recLabel);

    m_recordBtn = CCMenuItemSpriteExtra::create(recBg, this, menu_selector(DeepBotUI::onRecord));
    m_recordBtn->setPosition(-100, 15);
    m_macroMenu->addChild(m_recordBtn);

    // Play
    auto* playBg = CCSprite::create("GJ_button_04.png");
    playBg->setScale(1.0f);
    playBg->setColor({ 150, 150, 150 });
    auto* playLabel = CCLabelBMFont::create("Play", "bigFont.fnt");
    playLabel->setScale(0.45f);
    playLabel->setPosition(playBg->getContentSize() / 2);
    playBg->addChild(playLabel);

    auto* playBtn = CCMenuItemSpriteExtra::create(playBg, this, menu_selector(DeepBotUI::onPlay));
    playBtn->setPosition(100, 15);
    m_macroMenu->addChild(playBtn);

    // Stop
    auto* stopBg = CCSprite::create("GJ_button_04.png");
    stopBg->setScale(1.0f);
    stopBg->setColor({ 255, 100, 100 });
    auto* stopLabel = CCLabelBMFont::create("Stop", "bigFont.fnt");
    stopLabel->setScale(0.45f);
    stopLabel->setPosition(stopBg->getContentSize() / 2);
    stopBg->addChild(stopLabel);

    m_stopBtn = CCMenuItemSpriteExtra::create(stopBg, this, menu_selector(DeepBotUI::onStop));
    m_stopBtn->setPosition(-100, 15);
    m_stopBtn->setVisible(false);
    m_macroMenu->addChild(m_stopBtn);

    // Save / Load / Edit
    auto createSmallBtn = [&](const char* text, SEL_MenuHandler handler, float x) {
        auto* spr = ButtonSprite::create(text, 55, true, "bigFont.fnt", "GJ_button_01.png", 22, 0.48f);
        return CCMenuItemSpriteExtra::create(spr, this, handler);
    };

    auto* saveBtn = createSmallBtn("Save", menu_selector(DeepBotUI::onSave), -90);
    saveBtn->setPosition(-90, -45);
    m_macroMenu->addChild(saveBtn);

    auto* loadBtn = createSmallBtn("Load", menu_selector(DeepBotUI::onLoad), 0);
    loadBtn->setPosition(0, -45);
    m_macroMenu->addChild(loadBtn);

    auto* editBtn = createSmallBtn("Edit", menu_selector(DeepBotUI::onPlay), 90);
    editBtn->setPosition(90, -45);
    m_macroMenu->addChild(editBtn);

    this->addChild(m_macroMenu);

    // Settings Tab
    m_settingsMenu = CCMenu::create();
    m_settingsMenu->setPosition(cx, cy);
    m_settingsMenu->setVisible(false);

    auto* settingsText = CCLabelBMFont::create("Settings coming soon...", "chatFont.fnt");
    settingsText->setScale(0.5f);
    m_settingsMenu->addChild(settingsText);

    this->addChild(m_settingsMenu);

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
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    scene->addChild(this, 1000);
}

void DeepBotUI::hide() {
    this->removeFromParentAndCleanup(true);
}

void DeepBotUI::onClose(CCObject*) { hide(); }

void DeepBotUI::switchTab(int tab) {
    m_macroMenu->setVisible(tab == 0);
    m_settingsMenu->setVisible(tab == 1);
}

void DeepBotUI::onTabMacro(CCObject*) { switchTab(0); }
void DeepBotUI::onTabSettings(CCObject*) { switchTab(1); }

void DeepBotUI::refreshButtons() {
    auto& bot = DeepBot::instance();
    if (m_recordBtn) m_recordBtn->setVisible(!bot.isRecording());
    if (m_stopBtn) m_stopBtn->setVisible(bot.isRecording());
    if (m_actionsLabel) {
        m_actionsLabel->setString(("Actions: " + std::to_string(bot.getFrameCount())).c_str());
    }
}

void DeepBotUI::updateStatus(const std::string& status) {
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
    refreshButtons();
}

void DeepBotUI::onStop(CCObject*) {
    auto& bot = DeepBot::instance();
    if (!bot.isRecording()) return;
    bot.stopRecording();
    refreshButtons();
}

void DeepBotUI::onPlay(CCObject*) {
    auto& bot = DeepBot::instance();
    if (bot.isRecording() || bot.isPlaying()) return;
    if (bot.getFrameCount() == 0) {
        updateStatus("No macro!");
        return;
    }
    bot.startPlayback();
    refreshButtons();
}

void DeepBotUI::onSave(CCObject*) {
    auto* saveLayer = DeepBotSaveLayer::create();
    saveLayer->show();
}

void DeepBotUI::onLoad(CCObject*) {
    auto* loadLayer = DeepBotLoadLayer::create();
    loadLayer->show();
}

// ===== DeepBotLoadLayer =====
bool DeepBotLoadLayer::init() {
    if (!CCLayerColor::initWithColor(ccc4(0, 0, 0, 150))) return false;

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    float cx = winSize.width / 2;
    float cy = winSize.height / 2;

    auto* bg = CCScale9Sprite::create("GJ_square01.png");
    bg->setContentSize({ 380, 300 });
    bg->setPosition(cx, cy);
    bg->setColor({ 60, 60, 60 });
    this->addChild(bg);

    auto* title = CCLabelBMFont::create("Load Macro", "bigFont.fnt");
    title->setScale(0.5f);
    title->setPosition(cx, cy + 130);
    this->addChild(title);

    auto* closeMenu = CCMenu::create();
    closeMenu->setPosition(0, 0);
    auto* closeBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
        this,
        menu_selector(DeepBotLoadLayer::onClose)
    );
    closeBtn->setScale(0.55f);
    closeBtn->setPosition(cx + 170, cy + 130);
    closeMenu->addChild(closeBtn);
    this->addChild(closeMenu);

    refreshList();
    return true;
}

DeepBotLoadLayer* DeepBotLoadLayer::create() {
    auto* ret = new DeepBotLoadLayer();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void DeepBotLoadLayer::show() {
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    scene->addChild(this, 2000);
}

void DeepBotLoadLayer::hide() {
    this->removeFromParentAndCleanup(true);
}

void DeepBotLoadLayer::refreshList() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    float cx = winSize.width / 2;
    float cy = winSize.height / 2;

    auto* listMenu = CCMenu::create();
    listMenu->setPosition(cx - 160, cy + 80);

    auto macroDir = geode::dirs::getGameDir() / "macros";
    float y = 0;
    int count = 0;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(macroDir)) {
            if (!entry.is_regular_file()) continue;
            if (count >= 8) break;

            auto filename = entry.path().filename().string();
            auto ext = entry.path().extension().string();
            auto name = filename.substr(0, filename.find_last_of('.'));

            auto* nameLabel = CCLabelBMFont::create(name.c_str(), "chatFont.fnt");
            nameLabel->setScale(0.35f);
            nameLabel->setAnchorPoint({ 0, 0.5f });
            nameLabel->setPosition(30, y);

            auto* extLabel = CCLabelBMFont::create(ext.c_str(), "chatFont.fnt");
            extLabel->setScale(0.3f);
            extLabel->setAnchorPoint({ 0, 0.5f });
            extLabel->setColor({ 150, 150, 150 });
            extLabel->setPosition(30, y - 12);

            auto* checkOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
            checkOff->setScale(0.5f);
            auto* checkOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
            checkOn->setScale(0.5f);
            auto* checkBtn = CCMenuItemToggler::create(checkOff, checkOn, this, nullptr);
            checkBtn->setPosition(0, y - 5);

            auto* delBtn = CCMenuItemSpriteExtra::create(
                CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png"),
                this,
                menu_selector(DeepBotLoadLayer::onDeleteMacro)
            );
            delBtn->setScale(0.4f);
            delBtn->setPosition(200, y - 5);

            auto* loadBtn = CCMenuItemSpriteExtra::create(
                ButtonSprite::create("Load", 45, true, "bigFont.fnt", "GJ_button_01.png", 18, 0.42f),
                this,
                menu_selector(DeepBotLoadLayer::onLoadMacro)
            );
            loadBtn->setPosition(280, y - 5);
            loadBtn->setUserData(const_cast<char*>(filename.c_str()));

            listMenu->addChild(nameLabel);
            listMenu->addChild(extLabel);
            listMenu->addChild(checkBtn);
            listMenu->addChild(delBtn);
            listMenu->addChild(loadBtn);

            y -= 35;
            count++;
        }
    } catch (...) {}

    auto* bottomMenu = CCMenu::create();
    bottomMenu->setPosition(cx, cy - 120);

    auto* selAllBtn = CCMenuItemToggler::create(
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        this,
        nullptr
    );
    selAllBtn->setScale(0.5f);
    selAllBtn->setPosition(-120, 0);
    bottomMenu->addChild(selAllBtn);

    auto* selAllLabel = CCLabelBMFont::create("Select All", "chatFont.fnt");
    selAllLabel->setScale(0.35f);
    selAllLabel->setPosition(-70, 0);
    bottomMenu->addChild(selAllLabel);

    auto* delAllBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png"),
        this,
        nullptr
    );
    delAllBtn->setScale(0.45f);
    delAllBtn->setPosition(20, 0);
    bottomMenu->addChild(delAllBtn);

    auto* folderBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_openFolderBtn_001.png"),
        this,
        nullptr
    );
    folderBtn->setScale(0.5f);
    folderBtn->setPosition(70, 0);
    bottomMenu->addChild(folderBtn);

    auto* importBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png"),
        this,
        nullptr
    );
    importBtn->setScale(0.5f);
    importBtn->setPosition(120, 0);
    bottomMenu->addChild(importBtn);

    this->addChild(listMenu);
    this->addChild(bottomMenu);
}

void DeepBotLoadLayer::onLoadMacro(CCObject* sender) {
    auto* btn = static_cast<CCMenuItemSpriteExtra*>(sender);
    auto* filename = static_cast<char*>(btn->getUserData());
    if (!filename) return;

    auto path = (geode::dirs::getGameDir() / "macros" / filename).string();
    auto& bot = DeepBot::instance();
    
    std::string ext = std::filesystem::path(filename).extension().string();
    if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
    
    bot.loadFromFile(path, ext);
    hide();
}

void DeepBotLoadLayer::onDeleteMacro(CCObject*) {}
void DeepBotLoadLayer::onClose(CCObject*) { hide(); }

// ===== DeepBotSaveLayer — СИНИЕ ПОЛЯ КАК НА ФОТО =====
bool DeepBotSaveLayer::init() {
    if (!CCLayerColor::initWithColor(ccc4(0, 0, 0, 150))) return false;

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    float cx = winSize.width / 2;
    float cy = winSize.height / 2;

    // Тёмно-коричневый фон как на фото
    auto* bg = CCScale9Sprite::create("GJ_square01.png");
    bg->setContentSize({ 360, 340 });
    bg->setPosition(cx, cy);
    bg->setColor({ 60, 40, 30 });
    this->addChild(bg);

    auto* title = CCLabelBMFont::create("Save Macro", "bigFont.fnt");
    title->setScale(0.5f);
    title->setPosition(cx, cy + 150);
    this->addChild(title);

    // Close
    auto* closeMenu = CCMenu::create();
    closeMenu->setPosition(0, 0);
    auto* closeBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
        this,
        menu_selector(DeepBotSaveLayer::onClose)
    );
    closeBtn->setScale(0.55f);
    closeBtn->setPosition(cx + 160, cy + 150);
    closeMenu->addChild(closeBtn);
    this->addChild(closeMenu);

    auto* menu = CCMenu::create();
    menu->setPosition(cx, cy);

    // === Синее поле Macro Name ===
    auto* nameBg = CCScale9Sprite::create("GJ_square02.png");
    nameBg->setContentSize({ 300, 40 });
    nameBg->setPosition(0, 110);
    nameBg->setColor({ 80, 100, 200 }); // Синий
    menu->addChild(nameBg);

    m_nameInput = CCTextInputNode::create(280, 35, "Macro Name", "chatFont.fnt");
    m_nameInput->setPosition(0, 110);
    m_nameInput->setLabelPlaceholderColor({ 180, 180, 220 });
    menu->addChild(m_nameInput);

    // === Синее поле Author ===
    auto* authorBg = CCScale9Sprite::create("GJ_square02.png");
    authorBg->setContentSize({ 140, 35 });
    authorBg->setPosition(-80, 55);
    authorBg->setColor({ 80, 100, 200 });
    menu->addChild(authorBg);

    m_authorInput = CCTextInputNode::create(130, 30, "Author", "chatFont.fnt");
    m_authorInput->setPosition(-80, 55);
    m_authorInput->setLabelPlaceholderColor({ 180, 180, 220 });
    menu->addChild(m_authorInput);

    // === Синее поле Description ===
    auto* descBg = CCScale9Sprite::create("GJ_square02.png");
    descBg->setContentSize({ 300, 50 });
    descBg->setPosition(0, -5);
    descBg->setColor({ 80, 100, 200 });
    menu->addChild(descBg);

    m_descInput = CCTextInputNode::create(280, 45, "Description (optional)", "chatFont.fnt");
    m_descInput->setPosition(0, -5);
    m_descInput->setLabelPlaceholderColor({ 180, 180, 220 });
    menu->addChild(m_descInput);

    // === Save кнопка (зелёная) ===
    auto* saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", 90, true, "bigFont.fnt", "GJ_button_01.png", 38, 0.65f),
        this,
        menu_selector(DeepBotSaveLayer::onSaveConfirm)
    );
    saveBtn->setPosition(0, -65);
    menu->addChild(saveBtn);

    // === Back кнопка (зелёная) ===
    auto* backBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Back", 70, true, "bigFont.fnt", "GJ_button_01.png", 30, 0.55f),
        this,
        menu_selector(DeepBotSaveLayer::onClose)
    );
    backBtn->setPosition(0, -110);
    menu->addChild(backBtn);

    // === Other кнопка (серая) ===
    auto* otherBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Other", 70, true, "bigFont.fnt", "GJ_button_04.png", 30, 0.55f),
        this,
        menu_selector(DeepBotSaveLayer::onOtherFormat)
    );
    otherBtn->setPosition(0, -155);
    menu->addChild(otherBtn);

    // === Поле для ввода формата (скрыто по умолчанию) ===
    m_otherMenu = CCMenu::create();
    m_otherMenu->setPosition(0, -195);
    m_otherMenu->setVisible(false);

    auto* otherBg = CCScale9Sprite::create("GJ_square02.png");
    otherBg->setContentSize({ 120, 32 });
    otherBg->setPosition(0, 0);
    otherBg->setColor({ 80, 100, 200 });
    m_otherMenu->addChild(otherBg);

    m_otherFormatInput = CCTextInputNode::create(110, 28, "deep", "chatFont.fnt");
    m_otherFormatInput->setPosition(0, 0);
    m_otherFormatInput->setLabelPlaceholderColor({ 180, 180, 220 });
    m_otherMenu->addChild(m_otherFormatInput);

    menu->addChild(m_otherMenu);

    this->addChild(menu);
    return true;
}

DeepBotSaveLayer* DeepBotSaveLayer::create() {
    auto* ret = new DeepBotSaveLayer();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void DeepBotSaveLayer::show() {
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    scene->addChild(this, 2000);
}

void DeepBotSaveLayer::hide() {
    this->removeFromParentAndCleanup(true);
}

void DeepBotSaveLayer::onSaveConfirm(CCObject*) {
    auto& bot = DeepBot::instance();
    if (bot.getFrameCount() == 0) return;

    std::string format;
    if (m_otherMenu->isVisible() && m_otherFormatInput) {
        format = m_otherFormatInput->getString();
        if (format.empty()) format = "deep";
    } else {
        format = "deep"; // Default
    }

    auto macroDir = geode::dirs::getGameDir() / "macros";
    std::error_code ec;
    std::filesystem::create_directories(macroDir, ec);

    std::string name = m_nameInput ? m_nameInput->getString() : "macro";
    if (name.empty()) name = "macro";

    auto path = macroDir / (name + "." + format);
    bot.saveToFile(path.string(), format);
    hide();
}

void DeepBotSaveLayer::onOtherFormat(CCObject*) {
    m_otherMenu->setVisible(true);
}

void DeepBotSaveLayer::onClose(CCObject*) {
    hide();
}

} // namespace deepbot
