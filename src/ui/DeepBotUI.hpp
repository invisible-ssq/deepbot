#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace deepbot {

class DeepBotUI : public FLAlertLayer {
protected:
    bool init() override;

public:
    static DeepBotUI* create();
    void show() override;
    void onClose(CCObject*);
    void updateStatus(const std::string& status);
    void refreshButtons();

private:
    CCLabelBMFont* m_statusLabel = nullptr;
    CCMenuItemSpriteExtra* m_recordBtn = nullptr;
    CCMenuItemSpriteExtra* m_stopBtn = nullptr;
    CCMenuItemSpriteExtra* m_playBtn = nullptr;

    void onRecord(CCObject*);
    void onStop(CCObject*);
    void onPlay(CCObject*);
    void onSave(CCObject*);
    void onLoad(CCObject*);
    void onConvert(CCObject*);
    void onSaveFormat(CCObject*);
    void onSaveCustomFormat(CCObject*);
};

} // namespace deepbot
