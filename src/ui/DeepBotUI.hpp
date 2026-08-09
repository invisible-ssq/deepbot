#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace deepbot {

class DeepBotUI : public CCLayerColor {
public:
    static DeepBotUI* create();
    bool init() override;
    void show();
    void hide();

private:
    CCMenu* m_macroMenu = nullptr;
    CCLabelBMFont* m_actionsLabel = nullptr;
    CCMenuItemSpriteExtra* m_recordBtn = nullptr;
    CCMenuItemSpriteExtra* m_stopBtn = nullptr;
    
    void onRecord(CCObject*);
    void onStop(CCObject*);
    void onPlay(CCObject*);
    void onSave(CCObject*);
    void onLoad(CCObject*);
    void onClose(CCObject*);
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
    CCTextInputNode* m_nameInput = nullptr;
    CCTextInputNode* m_otherFormatInput = nullptr;
    CCMenu* m_otherMenu = nullptr;
    
    void onSaveConfirm(CCObject*);
    void onOtherFormat(CCObject*);
    void onClose(CCObject*);
};

} // namespace deepbot
