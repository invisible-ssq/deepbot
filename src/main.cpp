#include <Geode/Geode.hpp>
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
    log::info("deepbot v1.0.0 by goodxdeveloper");
    log::info("Free, closed-source, no watermarks, no subscriptions");
    log::info("Universal macro bot for Geometry Dash 2.2081");
}