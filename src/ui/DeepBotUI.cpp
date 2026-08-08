#include "DeepBotUI.hpp"
#include "../DeepBot.hpp"
#include <Geode/utils/async.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <algorithm>

using namespace geode::prelude;

namespace deepbot {

// ===== Маленькая плавающая кнопка D =====
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

    // Фон панели
    auto* bg = CCScale9Sprite::create("GJ_square01.png");
    bg->setContentSize({ 420, 280 });
    bg->setPosition(winSize.width / 2, winSize.height / 2);
    bg->setColor({ 80, 60, 140 }); // Фиолетовый как у xdBot
    this->addChild(bg);

    // Заголовок
    auto* title = CCLabelBMFont::create("DeepBot", "bigFont.fnt");
    title->setScale(0.5f);
    title->setPosition(winSize.width / 2, winSize.height / 2 + 120);
    this->addChild(title);

    // Кнопка закрытия
    auto* closeBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
        this,
        menu_selector(DeepBotUI::onClose)
    );
    closeBtn->setScale(0.6f);
    closeBtn->setPosition(winSize.width / 2 + 190, winSize.height / 2 + 120);

    auto* closeMenu = CCMenu::create();
    closeMenu->setPosition(0, 0);
    closeMenu->addChild(closeBtn);
    this->addChild(closeMenu);

    // Вкладки
    m_tabMenu = CCMenu::create();
    m_tabMenu->setPosition(winSize.width / 2, winSize.height / 2 + 90);

    auto createTabBtn = [&](const char* text, SEL_MenuHandler handler, float x) {
        auto* spr = ButtonSprite::create(text, 80, true, "bigFont.fnt", "GJ_button_01.png", 30, 0.55f);
        auto* btn = CCMenuItemSpriteExtra::create(spr, this, handler);
        btn->setPosition(x, 0);
        return btn;
    };

    auto* macroTab = createTabBtn("Macro", menu_selector(DeepBotUI::onTabMacro), -80);
    macroTab->setTag(0);
    m_tabMenu->addChild(macroTab);

    auto* settingsTab = createTabBtn("Settings", menu_selector(DeepBotUI::onTabSettings), 80);
    settingsTab->setTag(1);
    m_tabMenu->addChild(settingsTab);

    this->addChild(m_tabMenu);

    // ===== Macro Tab =====
    m_macroMenu = CCMenu::create();
    m_macroMenu->setPosition(winSize.width / 2, winSize.height / 2 + 20);

    // Actions label
    auto* actionsLabel = CCLabelBMFont::create("Actions: 0", "chatFont.fnt");
    actionsLabel->setScale(0.4f);
    actionsLabel->setPosition(-140, 50);
    actionsLabel->setTag(100);
    m_macroMenu->addChild(actionsLabel);

    // Record button
    auto* recSpr = CCSprite::create("GJ_button_01.png");
    recSpr->setScale(0.8f);
    auto* recLabel = CCLabelBMFont::create("Record", "bigFont.fnt");
    recLabel->setScale(0.4f);
    recLabel->setPosition(recSpr->getContentSize() / 2);
    recSpr->addChild(recLabel);

    m_recordBtn = CCMenuItemSpriteExtra::create(recSpr, this, menu_selector(DeepBotUI::onRecord));
    m_recordBtn->setPosition(-100, 0);
    m_macroMenu->addChild(m_recordBtn);

    // Play button
    auto* playSpr = CCSprite::create("GJ_button_01.png");
    playSpr->setScale(0.8f);
    auto* playLabel = CCLabelBMFont::create("Play", "bigFont.fnt");
    playLabel->setScale(0.4f);
    playLabel->setPosition(playSpr->getContentSize() / 2);
    playSpr->addChild(playLabel);

    auto* playBtn = CCMenuItemSpriteExtra::create(playSpr, this, menu_selector(DeepBotUI::onPlay));
    playBtn->setPosition(0, 0);
    m_macroMenu->addChild(playBtn);

    // Stop button (hidden by default)
    auto* stopSpr = CCSprite::create("GJ_button_04.png");
    stopSpr->setScale(0.8f);
    auto* stopLabel = CCLabelBMFont::create("Stop", "bigFont.fnt");
    stopLabel->setScale(0.4f);
    stopLabel->setPosition(stopSpr->getContentSize() / 2);
    stopSpr->addChild(stopLabel);

    m_stopBtn = CCMenuItemSpriteExtra::create(stopSpr, this, menu_selector(DeepBotUI::onStop));
    m_stopBtn->setPosition(-100, 0);
    m_stopBtn->setVisible(false);
    m_macroMenu->addChild(m_stopBtn);

    // Save / Load / Edit row
    auto* saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", 60, true, "bigFont.fnt", "GJ_button_01.png", 25, 0.5f),
        this,
        menu_selector(DeepBotUI::onSave)
    );
    saveBtn->setPosition(-80, -50);
    m_macroMenu->addChild(saveBtn);

    auto* loadBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Load", 60, true, "bigFont.fnt", "GJ_button_01.png", 25, 0.5f),
        this,
        menu_selector(DeepBotUI::onLoad)
    );
    loadBtn->setPosition(0, -50);
    m_macroMenu->addChild(loadBtn);

    auto* editBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Edit", 60, true, "bigFont.fnt", "GJ_button_01.png", 25, 0.5f),
        this,
        menu_selector(DeepBotUI::onPlay) // placeholder
    );
    editBtn->setPosition(80, -50);
    m_macroMenu->addChild(editBtn);

    this->addChild(m_macroMenu);

    // ===== Settings Tab (hidden by default) =====
    m_settingsMenu = CCMenu::create();
    m_settingsMenu->setPosition(winSize.width / 2, winSize.height / 2);
    m_settingsMenu->setVisible(false);

    auto* settingsLabel = CCLabelBMFont::create("Settings placeholder", "chatFont.fnt");
    settingsLabel->setScale(0.5f);
    m_settingsMenu->addChild(settingsLabel);

    this->addChild(m_settingsMenu);

    // Info button
    auto* infoBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"),
        this,
        menu_selector(DeepBotUI::onClose)
    );
    infoBtn->setScale(0.6f);
    infoBtn->setPosition(winSize.width / 2 + 150, winSize.height / 2 + 120);
    closeMenu->addChild(infoBtn);

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

void DeepBotUI::onClose(CCObject*) {
    hide();
}

void DeepBotUI::switchTab(DeepBotTab tab) {
    m_currentTab = tab;
    m_macroMenu->setVisible(tab == DeepBotTab::Macro);
    m_settingsMenu->setVisible(tab == DeepBotTab::Settings);
}

void DeepBotUI::onTabMacro(CCObject*) {
    switchTab(DeepBotTab::Macro);
}

void DeepBotUI::onTabSettings(CCObject*) {
    switchTab(DeepBotTab::Settings);
}

void DeepBotUI::refreshButtons() {
    auto& bot = DeepBot::instance();
    if (m_recordBtn) m_recordBtn->setVisible(!bot.isRecording());
    if (m_stopBtn) m_stopBtn->setVisible(bot.isRecording());
    
    // Update actions count
    if (m_macroMenu) {
        auto* actionsLabel = static_cast<CCLabelBMFont*>(m_macroMenu->getChildByTag(100));
        if (actionsLabel) {
            actionsLabel->setString(("Actions: " + std::to_string(bot.getFrameCount())).c_str());
        }
    }
}

void DeepBotUI::updateStatus(const std::string& status) {
    refreshButtons();
}

void DeepBotUI::onRecord(CCObject*) {
    auto& bot = DeepBot::instance();
    if (bot.isRecording()) return;

    auto* playLayer = PlayLayer::get();
    if (!playLayer) return;

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
    if (bot.getFrameCount() == 0) return;

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

    auto* bg = CCScale9Sprite::create("GJ_square01.png");
    bg->setContentSize({ 380, 300 });
    bg->setPosition(winSize.width / 2, winSize.height / 2);
    bg->setColor({ 60, 60, 60 });
    this->addChild(bg);

    auto* title = CCLabelBMFont::create("Load Macro", "bigFont.fnt");
    title->setScale(0.5f);
    title->setPosition(winSize.width / 2, winSize.height / 2 + 130);
    this->addChild(title);

    // Close
    auto* closeBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
        this,
        menu_selector(DeepBotLoadLayer::onClose)
    );
    closeBtn->setScale(0.6f);

    auto* closeMenu = CCMenu::create();
    closeMenu->setPosition(winSize.width / 2 + 170, winSize.height / 2 + 130);
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
    // List macros from /game/macros/
    auto macroDir = geode::dirs::getGameDir() / "macros";
    
    auto* listMenu = CCMenu::create();
    listMenu->setPosition(CCDirector::sharedDirector()->getWinSize().width / 2 - 150,
                          CCDirector::sharedDirector()->getWinSize().height / 2 + 80);

    float y = 0;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(macroDir)) {
            if (!entry.is_regular_file()) continue;
            
            auto filename = entry.path().filename().string();
            auto ext = entry.path().extension().string();

            // Name label
            auto* nameLabel = CCLabelBMFont::create(filename.c_str(), "chatFont.fnt");
            nameLabel->setScale(0.35f);
            nameLabel->setAnchorPoint({ 0, 0.5f });
            nameLabel->setPosition(30, y);

            // Checkbox
            auto* checkOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
            checkOff->setScale(0.5f);
            auto* checkOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
            checkOn->setScale(0.5f);
            auto* checkBtn = CCMenuItemToggler::create(checkOff, checkOn, this, nullptr);
            checkBtn->setPosition(0, y);

            // Delete
            auto* delBtn = CCMenuItemSpriteExtra::create(
                CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png"),
                this,
                menu_selector(DeepBotLoadLayer::onDeleteMacro)
            );
            delBtn->setScale(0.4f);
            delBtn->setPosition(200, y);
            delBtn->setTag(static_cast<int>(y));

            // Load button
            auto* loadBtn = CCMenuItemSpriteExtra::create(
                ButtonSprite::create("Load", 50, true, "bigFont.fnt", "GJ_button_01.png", 20, 0.45f),
                this,
                menu_selector(DeepBotLoadLayer::onLoadMacro)
            );
            loadBtn->setPosition(280, y);
            loadBtn->setUserData(const_cast<char*>(filename.c_str()));

            listMenu->addChild(nameLabel);
            listMenu->addChild(checkBtn);
            listMenu->addChild(delBtn);
            listMenu->addChild(loadBtn);

            y -= 30;
        }
    } catch (...) {}

    this->addChild(listMenu);
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

void DeepBotLoadLayer::onDeleteMacro(CCObject*) {
    // TODO
}

void DeepBotLoadLayer::onClose(CCObject*) {
    hide();
}

// ===== DeepBotSaveLayer =====
bool DeepBotSaveLayer::init() {
    if (!CCLayerColor::initWithColor(ccc4(0, 0, 0, 150))) return false;

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    auto* bg = CCScale9Sprite::create("GJ_square01.png");
    bg->setContentSize({ 380, 320 });
    bg->setPosition(winSize.width / 2, winSize.height / 2);
    bg->setColor({ 60, 60, 60 });
    this->addChild(bg);

    auto* title = CCLabelBMFont::create("Save Macro", "bigFont.fnt");
    title->setScale(0.5f);
    title->setPosition(winSize.width / 2, winSize.height / 2 + 140);
    this->addChild(title);

    // Close
    auto* closeBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
        this,
        menu_selector(DeepBotSaveLayer::onClose)
    );
    closeBtn->setScale(0.6f);

    auto* closeMenu = CCMenu::create();
    closeMenu->setPosition(winSize.width / 2 + 170, winSize.height / 2 + 140);
    closeMenu->addChild(closeBtn);
    this->addChild(closeMenu);

    auto* menu = CCMenu::create();
    menu->setPosition(winSize.width / 2, winSize.height / 2);

    // Name field (placeholder using label)
    auto* nameLabel = CCLabelBMFont::create("Macro Name", "chatFont.fnt");
    nameLabel->setScale(0.4f);
    nameLabel->setPosition(0, 100);
    menu->addChild(nameLabel);

    // Author
    auto* authorLabel = CCLabelBMFont::create("Author (optional)", "chatFont.fnt");
    authorLabel->setScale(0.35f);
    authorLabel->setPosition(0, 60);
    menu->addChild(authorLabel);

    // Description
    auto* descLabel = CCLabelBMFont::create("Description (optional)", "chatFont.fnt");
    descLabel->setScale(0.35f);
    descLabel->setPosition(0, 20);
    menu->addChild(descLabel);

    // Save button
    auto* saveBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", 80, true, "bigFont.fnt", "GJ_button_01.png", 35, 0.6f),
        this,
        menu_selector(DeepBotSaveLayer::onSaveConfirm)
    );
    saveBtn->setPosition(0, -40);
    menu->addChild(saveBtn);

    // Format buttons row
    const char* formats[] = {"CML", "JSON", "GDR", "GDR2"};
    float x = -120;
    for (int i = 0; i < 4; i++) {
        auto* checkOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
        checkOff->setScale(0.4f);
        auto* checkOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
        checkOn->setScale(0.4f);
        
        auto* toggle = CCMenuItemToggler::create(checkOff, checkOn, this, menu_selector(DeepBotSaveLayer::onFormatSelect));
        toggle->setPosition(x, -90);
        toggle->setTag(i);
        if (i == 0) toggle->toggle(true);
        
        auto* fmtLabel = CCLabelBMFont::create(formats[i], "chatFont.fnt");
        fmtLabel->setScale(0.3f);
        fmtLabel->setPosition(x + 20, -90);
        
        menu->addChild(toggle);
        menu->addChild(fmtLabel);
        x += 70;
    }

    // Other button
    auto* otherBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Other", 60, true, "bigFont.fnt", "GJ_button_04.png", 25, 0.5f),
        this,
        menu_selector(DeepBotSaveLayer::onOtherFormat)
    );
    otherBtn->setPosition(0, -130);
    menu->addChild(otherBtn);

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

void DeepBotSaveLayer::onFormatSelect(CCObject* sender) {
    auto* toggle = static_cast<CCMenuItemToggler*>(sender);
    m_selectedFormat = toggle->getTag();
    
    // Untoggle others
    if (auto* menu = toggle->getParent()) {
        for (int i = 0; i < 4; i++) {
            if (i == m_selectedFormat) continue;
            if (auto* other = static_cast<CCMenuItemToggler*>(menu->getChildByTag(i))) {
                other->toggle(false);
            }
        }
    }
}

void DeepBotSaveLayer::onSaveConfirm(CCObject*) {
    auto& bot = DeepBot::instance();
    
    const char* formatMap[] = {"deep", "ttr3", "gdr", "gdr2"};
    auto format = formatMap[m_selectedFormat];
    
    auto macroDir = geode::dirs::getGameDir() / "macros";
    std::error_code ec;
    std::filesystem::create_directories(macroDir, ec);
    
    auto timestamp = std::to_string(std::time(nullptr));
    auto path = macroDir / ("macro_" + timestamp + "." + format);
    
    bot.saveToFile(path.string(), format);
    hide();
}

void DeepBotSaveLayer::onOtherFormat(CCObject*) {
    geode::async::spawn(
        file::pick(file::PickMode::SaveFile, {
            .defaultPath = "macro",
            .filters = {{ .description = "All files", .files = { "*" } }}
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
    hide();
}

void DeepBotSaveLayer::onClose(CCObject*) {
    hide();
}

} // namespace deepbot
