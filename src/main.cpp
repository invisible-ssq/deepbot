#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "DeepBot.hpp"
#include "ui/DeepBotUI.hpp"
#include "formats/DeepParser.hpp"

using namespace geode::prelude;

$execute {
    auto formats = deepbot::DeepParser::getSupportedFormats();
    std::string formatList;
    for (const auto& fmt : formats) {
        if (!formatList.empty()) formatList += ", ";
        formatList += fmt;
    }
    log::info("deepbot loaded! Supported formats: {}", formatList);
}

$on_mod(Loaded) {
    auto version = Mod::get()->getVersion();
    log::info("deepbot {}.{}.{} by goodxdeveloper", version.getMajor(), version.getMinor(), version.getPatch());
    log::info("Free, no watermarks, no subscriptions");
    log::info("Universal macro bot for Geometry Dash 2.2081");
}
