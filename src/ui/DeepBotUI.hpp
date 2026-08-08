#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace deepbot {

enum class DeepBotTab {
    Macro,
    Settings
};

class DeepBotUI : public CCLayerColor {
public:
    static DeepBotUI* create();
    bool init() override;
    void show();
    void hide();
    void updateStatus(const std::string& status);

private:
    CCMenu* m_tabMenu = nullptr;
    CCMenu* m_macroMenu = nullptr;
    CCMenu* m_settingsMenu = nullptr;
    CCLabelBMFont* m_statusLabel = nullptr;
    CCMenuItemSpriteExtra* m_recordBtn = nullptr;
    CCMenuItemSpriteExtra* m_stopBtn = nullptr;
    
    DeepBotTab m_currentTab = DeepBotTab::Macro;
    
    void onTabMacro(CCObject*);
    void onTabSettings(CCObject*);
    void onRecord(CCObject*);
    void onStop(CCObject*);
    void onPlay(CCObject*);
    void onSave(CCObject*);
    void onLoad(CCObject*);
    void onClose(CCObject*);
    
    void switchTab(DeepBotTab tab);
    void refreshButtons();
};

class DeepBotLoadLayer : public CCLayerColor {
public:
    static DeepBotLoadLayer* create();
    bool init() override;
    void show();
    void hide();
    
private:
    void refreshList();
    void onLoadMacro(CCObject*);
    void onDeleteMacro(CCObject*);
    void onClose(CCObject*);
};

class DeepBotSaveLayer : public CCLayerColor {
public:
    static DeepBotSaveLayer* create();
    bool init() override;
    void show();
    void hide();
    
private:
    int m_selectedFormat = 0; // 0=deep, 1=ttr3, 2=gdr, 3=gdr2, 4=slc, 5=xd, 6=ybot, 7=txt
    std::string m_name;
    std::string m_author;
    std::string m_desc;
    
    void onFormatSelect(CCObject*);
    void onSaveConfirm(CCObject*);
    void onOtherFormat(CCObject*);
    void onClose(CCObject*);
    void refreshFormatButtons();
};

} // namespace deepbot
