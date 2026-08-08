#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace deepbot {

class DeepBotUI : public FLAlertLayer {
protected:
    bool init() override;

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
    CCLabelBMFont* m_statusLabel = nullptr;
    CCMenu* m_buttonMenu = nullptr;
};

} // namespace deepbot
