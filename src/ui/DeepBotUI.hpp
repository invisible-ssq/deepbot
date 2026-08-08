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

} // namespace deepbot