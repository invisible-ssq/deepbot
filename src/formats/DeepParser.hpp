#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <cctype>
#include <cstring>
#include "FormatRegistry.hpp"
#include "DeepFormat.hpp"
#include "TTR3Format.hpp"
#include "GDRFormat.hpp"
#include "GDR2Format.hpp"
#include "SLCFormat.hpp"
#include "XDFormat.hpp"
#include "YBotFormat.hpp"
#include "TCMFormat.hpp"
#include "REFormat.hpp"
#include "ZBotFormat.hpp"
#include "MHRFormat.hpp"
#include "EchoFormat.hpp"
#include "PlaintextFormat.hpp"

namespace deepbot {

class DeepParser {
public:
    static std::string detectFormat(const std::string& path, const std::vector<uint8_t>& data) {
        std::string ext = path.substr(path.find_last_of(".") + 1);
        for (auto& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

        if (data.size() >= 4) {
            if (std::memcmp(data.data(), "DEEP", 4) == 0) return "deep";
            if (std::memcmp(data.data(), "TTR3", 4) == 0) return "ttr3";
            if (std::memcmp(data.data(), "GDR", 3) == 0) {
                if (data.size() > 3 && data[3] >= '0' && data[3] <= '9') return "gdr2";
                return "gdr2";
            }
            if (std::memcmp(data.data(), "YBOT", 4) == 0) return "ybot";
            if (std::memcmp(data.data(), "ZBF", 3) == 0) return "zbf";
            if (std::memcmp(data.data(), "ECHO", 4) == 0) return "echo";
            if (std::memcmp(data.data(), "XD", 2) == 0) return "xd";
            if (std::memcmp(data.data(), "RE2", 3) == 0) return "re2";
            if (std::memcmp(data.data(), "RE3", 3) == 0) return "re3";
            if (std::memcmp(data.data(), "RE4", 3) == 0) return "re4";
            if (std::memcmp(data.data(), "SILL", 4) == 0) return "slc";
            if (data.size() >= 8 && std::memcmp(data.data(), "SLC3RPLY", 8) == 0) return "slc";
        }

        if (data.size() > 0 && (data[0] == '{' || data[0] == '[')) {
            std::string content(data.begin(), data.begin() + std::min(size_t(100), data.size()));
            if (content.find("\"framerate\"") != std::string::npos) return "gdr";
            if (content.find("\"fps\"") != std::string::npos) return "mhr";
        }

        if (data.size() > 0 && (data[0] == '#' || (data[0] >= '0' && data[0] <= '9'))) {
            return "txt";
        }

        if (ext == "deep" || ext == "ttr3" || ext == "gdr" || ext == "gdr2" ||
            ext == "slc" || ext == "cml" || ext == "xd" || ext == "ybot" ||
            ext == "ybf" || ext == "tcm" || ext == "re" || ext == "re2" ||
            ext == "re3" || ext == "re4" || ext == "zbf" || ext == "mhr" ||
            ext == "echo" || ext == "txt") {
            return ext;
        }

        return "";
    }

    static UnifiedReplay parse(const std::string& path, const std::vector<uint8_t>& data) {
        std::string format = detectFormat(path, data);
        if (format.empty()) {
            throw std::runtime_error("Unknown format: " + path);
        }
        return parseFormat(format, data);
    }

    static UnifiedReplay parseFormat(const std::string& format, const std::vector<uint8_t>& data) {
        if (format == "deep") {
            auto replay = DeepFormat::read(data);
            return deepToUnified(replay);
        }
        else if (format == "ttr3") {
            auto replay = TTR3Format::read(data);
            return ttr3ToUnified(replay);
        }
        else if (format == "gdr") {
            std::string jsonStr(data.begin(), data.end());
            auto replay = GDRFormat::readJSON(jsonStr);
            return gdrToUnified(replay);
        }
        else if (format == "gdr2") {
            auto replay = GDR2Format::read(data);
            return gdr2ToUnified(replay);
        }
        else if (format == "slc") {
            auto replay = SLCFormat::read(data);
            return slcToUnified(replay);
        }
        else if (format == "xd") {
            auto replay = XDFormat::read(data);
            return xdToUnified(replay);
        }
        else if (format == "ybot" || format == "ybf") {
            auto replay = YBotFormat::read(data);
            return ybotToUnified(replay);
        }
        else if (format == "tcm") {
            auto replay = TCMFormat::read(data);
            return tcmToUnified(replay);
        }
        else if (format == "re" || format == "re2" || format == "re3" || format == "re4") {
            auto replay = REFormat::read(data);
            return reToUnified(replay);
        }
        else if (format == "zbf") {
            auto replay = ZBotFormat::read(data);
            return zbotToUnified(replay);
        }
        else if (format == "mhr") {
            auto replay = MHRFormat::read(data);
            return mhrToUnified(replay);
        }
        else if (format == "echo") {
            auto replay = EchoFormat::read(data);
            return echoToUnified(replay);
        }
        else if (format == "txt") {
            auto replay = PlaintextFormat::read(data);
            return plaintextToUnified(replay);
        }

        throw std::runtime_error("Unsupported format: " + format);
    }

    static std::vector<uint8_t> serialize(const UnifiedReplay& replay, const std::string& format) {
        if (format == "deep") {
            auto deep = unifiedToDeep(replay);
            return DeepFormat::write(deep);
        }
        else if (format == "ttr3") {
            auto ttr3 = unifiedToTTR3(replay);
            return TTR3Format::write(ttr3);
        }
        else if (format == "gdr") {
            auto gdr = unifiedToGDR(replay);
            auto json = GDRFormat::writeJSON(gdr);
            return std::vector<uint8_t>(json.begin(), json.end());
        }
        else if (format == "gdr2") {
            auto gdr2 = unifiedToGDR2(replay);
            return GDR2Format::write(gdr2);
        }
        else if (format == "slc") {
            auto slc = unifiedToSLC(replay);
            return SLCFormat::write(slc);
        }
        else if (format == "xd") {
            auto xd = unifiedToXD(replay);
            return XDFormat::write(xd);
        }
        else if (format == "ybot") {
            auto ybot = unifiedToYBot(replay);
            return YBotFormat::write(ybot);
        }
        else if (format == "tcm") {
            auto tcm = unifiedToTCM(replay);
            return TCMFormat::write(tcm);
        }
        else if (format == "re" || format == "re2" || format == "re3" || format == "re4") {
            int version = 1;
            if (format == "re2") version = 2;
            else if (format == "re3") version = 3;
            else if (format == "re4") version = 4;
            auto re = unifiedToRE(replay);
            return REFormat::write(re, version);
        }
        else if (format == "zbf") {
            auto zbf = unifiedToZBot(replay);
            return ZBotFormat::write(zbf);
        }
        else if (format == "mhr") {
            auto mhr = unifiedToMHR(replay);
            return MHRFormat::write(mhr);
        }
        else if (format == "echo") {
            auto echo = unifiedToEcho(replay);
            return EchoFormat::write(echo);
        }
        else if (format == "txt") {
            auto txt = unifiedToPlaintext(replay);
            return PlaintextFormat::write(txt);
        }

        throw std::runtime_error("Unsupported output format: " + format);
    }

    static std::vector<std::string> getSupportedFormats() {
        return {
            "deep", "ttr3", "gdr", "gdr2", "slc", "cml",
            "xd", "ybot", "ybf", "tcm", "re", "re2", "re3", "re4",
            "zbf", "mhr", "echo", "txt"
        };
    }

    static std::string getFormatDescription(const std::string& format) {
        static const std::unordered_map<std::string, std::string> descriptions = {
            {"deep", "deepbot Native (.deep)"},
            {"ttr3", "ToastyReplay 3 (.ttr3)"},
            {"gdr", "GDReplayFormat (.gdr)"},
            {"gdr2", "GDReplayFormat 2 (.gdr2)"},
            {"slc", "Silicate (.slc)"},
            {"cml", "xdBot Compressed (.cml)"},
            {"xd", "xdBot (.xd)"},
            {"ybot", "yBot 2 (.ybot)"},
            {"ybf", "yBot 1 (.ybf)"},
            {"tcm", "TCBot (.tcm)"},
            {"re", "ReplayEngine (.re)"},
            {"re2", "ReplayEngine 2 (.re2)"},
            {"re3", "ReplayEngine 3 (.re3)"},
            {"re4", "ReplayEngine 4 (.re4)"},
            {"zbf", "zBot (.zbf)"},
            {"mhr", "MegaHack Replay (.mhr)"},
            {"echo", "Echo (.echo)"},
            {"txt", "Plaintext (.txt)"}
        };
        auto it = descriptions.find(format);
        return (it != descriptions.end()) ? it->second : "Unknown";
    }

private:
    static UnifiedReplay deepToUnified(const DeepFormat::Replay& replay) {
        UnifiedReplay unified;
        unified.author = replay.author;
        unified.description = replay.description;
        unified.tps = replay.tps;
        unified.duration = replay.duration;
        unified.seed = replay.seed;
        for (const auto& input : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = input.absoluteTime;
            ui.down = input.flags & 1;
            ui.player2 = input.flags & 2;
            ui.button = (input.flags >> 2) & 3;
            if (ui.button == 0) ui.button = 1;
            ui.x = input.x;
            ui.y = input.y;
            ui.rotation = input.rotation;
            ui.yAccel = input.yAccel;
            unified.inputs.push_back(ui);
        }
        return unified;
    }

    static UnifiedReplay ttr3ToUnified(const TTR3Format::Replay& replay) {
        UnifiedReplay unified;
        unified.tps = replay.fps;
        for (const auto& input : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = input.timeSeconds;
            ui.down = input.flags & 1;
            ui.player2 = input.flags & 2;
            ui.button = input.actionType;
            if (ui.button == 0) ui.button = 1;
            unified.inputs.push_back(ui);
        }
        return unified;
    }

    static UnifiedReplay gdrToUnified(const GDRFormat::Replay& replay) {
        UnifiedReplay unified;
        unified.author = replay.author;
        unified.tps = replay.framerate;
        unified.duration = replay.duration;
        unified.seed = replay.seed;
        for (const auto& input : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = input.frame / replay.framerate;
            ui.down = input.down;
            ui.player2 = input.player2;
            ui.button = input.button;
            if (ui.button == 0) ui.button = 1;
            unified.inputs.push_back(ui);
        }
        return unified;
    }

    static UnifiedReplay gdr2ToUnified(const GDR2Format::Replay& replay) {
        UnifiedReplay unified;
        unified.author = replay.author;
        unified.tps = replay.framerate;
        unified.duration = replay.duration;
        unified.seed = replay.seed;
        for (const auto& input : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = input.frame / replay.framerate;
            ui.down = input.down;
            ui.player2 = input.player2;
            ui.button = input.button;
            if (ui.button == 0) ui.button = 1;
            unified.inputs.push_back(ui);
        }
        return unified;
    }

    static UnifiedReplay slcToUnified(const SLCFormat::Replay& replay) {
        UnifiedReplay unified;
        unified.tps = replay.fps;
        unified.seed = replay.seed;
        for (const auto& input : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = input.frame / replay.fps;
            ui.down = input.down;
            ui.player2 = input.player2;
            ui.button = input.button;
            if (ui.button == 0) ui.button = 1;
            unified.inputs.push_back(ui);
        }
        return unified;
    }

    static UnifiedReplay xdToUnified(const XDFormat::Replay& replay) {
        UnifiedReplay unified;
        unified.tps = replay.fps;
        for (const auto& input : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = input.frame / replay.fps;
            ui.down = input.down;
            ui.player2 = input.player2;
            ui.button = input.button;
            if (ui.button == 0) ui.button = 1;
            unified.inputs.push_back(ui);
        }
        return unified;
    }

    static UnifiedReplay ybotToUnified(const YBotFormat::Replay& replay) {
        UnifiedReplay unified;
        unified.tps = replay.fps;
        for (const auto& input : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = input.frame / replay.fps;
            ui.down = input.down;
            ui.player2 = input.player2;
            ui.button = input.button;
            if (ui.button == 0) ui.button = 1;
            unified.inputs.push_back(ui);
        }
        return unified;
    }

    static UnifiedReplay tcmToUnified(const TCMFormat::Replay& replay) {
        UnifiedReplay unified;
        unified.tps = replay.fps;
        for (const auto& input : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = input.frame / replay.fps;
            ui.down = input.down;
            ui.player2 = input.player2;
            ui.button = input.button;
            if (ui.button == 0) ui.button = 1;
            unified.inputs.push_back(ui);
        }
        return unified;
    }

    static UnifiedReplay reToUnified(const REFormat::Replay& replay) {
        UnifiedReplay unified;
        unified.tps = replay.fps;
        for (const auto& input : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = input.frame / replay.fps;
            ui.down = input.down;
            ui.player2 = input.player2;
            ui.button = input.button;
            if (ui.button == 0) ui.button = 1;
            unified.inputs.push_back(ui);
        }
        return unified;
    }

    static UnifiedReplay zbotToUnified(const ZBotFormat::Replay& replay) {
        UnifiedReplay unified;
        unified.tps = replay.fps;
        for (const auto& input : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = input.frame / replay.fps;
            ui.down = input.down;
            ui.player2 = input.player2;
            ui.button = input.button;
            if (ui.button == 0) ui.button = 1;
            unified.inputs.push_back(ui);
        }
        return unified;
    }

    static UnifiedReplay mhrToUnified(const MHRFormat::Replay& replay) {
        UnifiedReplay unified;
        unified.tps = replay.fps;
        unified.seed = replay.levelId;
        for (const auto& input : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = input.frame / replay.fps;
            ui.down = input.down;
            ui.player2 = input.player2;
            ui.button = input.button;
            if (ui.button == 0) ui.button = 1;
            unified.inputs.push_back(ui);
        }
        return unified;
    }

    static UnifiedReplay echoToUnified(const EchoFormat::Replay& replay) {
        UnifiedReplay unified;
        unified.tps = replay.fps;
        for (const auto& input : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = input.frame / replay.fps;
            ui.down = input.down;
            ui.player2 = input.player2;
            ui.button = input.button;
            if (ui.button == 0) ui.button = 1;
            unified.inputs.push_back(ui);
        }
        return unified;
    }

    static UnifiedReplay plaintextToUnified(const PlaintextFormat::Replay& replay) {
        UnifiedReplay unified;
        unified.tps = replay.fps;
        for (const auto& input : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = input.frame / replay.fps;
            ui.down = input.down;
            ui.player2 = input.player2;
            ui.button = input.button;
            if (ui.button == 0) ui.button = 1;
            unified.inputs.push_back(ui);
        }
        return unified;
    }

    static DeepFormat::Replay unifiedToDeep(const UnifiedReplay& unified) {
        DeepFormat::Replay replay;
        replay.author = unified.author;
        replay.description = unified.description;
        replay.tps = unified.tps;
        replay.duration = unified.duration;
        replay.seed = unified.seed;
        for (const auto& input : unified.inputs) {
            DeepFormat::DeepInput di;
            di.absoluteTime = input.absoluteTime;
            di.flags = (input.down ? 1 : 0)
                     | (input.player2 ? 2 : 0)
                     | ((input.button & 3) << 2);
            di.x = input.x;
            di.y = input.y;
            di.rotation = input.rotation;
            di.yAccel = input.yAccel;
            replay.inputs.push_back(di);
        }
        return replay;
    }

    static TTR3Format::Replay unifiedToTTR3(const UnifiedReplay& unified) {
        TTR3Format::Replay replay;
        replay.fps = unified.tps;
        for (const auto& input : unified.inputs) {
            TTR3Format::Input ti;
            ti.timeSeconds = input.absoluteTime;
            ti.actionType = input.button;
            ti.flags = (input.down ? 1 : 0) | (input.player2 ? 2 : 0);
            ti.reserved = 0;
            replay.inputs.push_back(ti);
        }
        return replay;
    }

    static GDRFormat::Replay unifiedToGDR(const UnifiedReplay& unified) {
        GDRFormat::Replay replay;
        replay.author = unified.author;
        replay.framerate = unified.tps;
        replay.duration = unified.duration;
        replay.seed = unified.seed;
        for (const auto& input : unified.inputs) {
            GDRFormat::Input gi;
            gi.frame = static_cast<uint32_t>(input.absoluteTime * unified.tps);
            gi.down = input.down;
            gi.player2 = input.player2;
            gi.button = input.button;
            replay.inputs.push_back(gi);
        }
        return replay;
    }

    static GDR2Format::Replay unifiedToGDR2(const UnifiedReplay& unified) {
        GDR2Format::Replay replay;
        replay.author = unified.author;
        replay.framerate = unified.tps;
        replay.duration = unified.duration;
        replay.seed = unified.seed;
        for (const auto& input : unified.inputs) {
            GDR2Format::Input gi;
            gi.frame = static_cast<uint64_t>(input.absoluteTime * unified.tps);
            gi.down = input.down;
            gi.player2 = input.player2;
            gi.button = input.button;
            replay.inputs.push_back(gi);
        }
        return replay;
    }

    static SLCFormat::Replay unifiedToSLC(const UnifiedReplay& unified) {
        SLCFormat::Replay replay;
        replay.fps = unified.tps;
        replay.seed = unified.seed;
        for (const auto& input : unified.inputs) {
            SLCFormat::Input si;
            si.frame = static_cast<uint32_t>(input.absoluteTime * unified.tps);
            si.down = input.down;
            si.player2 = input.player2;
            si.button = input.button;
            replay.inputs.push_back(si);
        }
        return replay;
    }

    static XDFormat::Replay unifiedToXD(const UnifiedReplay& unified) {
        XDFormat::Replay replay;
        replay.fps = unified.tps;
        for (const auto& input : unified.inputs) {
            XDFormat::Input xi;
            xi.frame = static_cast<uint32_t>(input.absoluteTime * unified.tps);
            xi.down = input.down;
            xi.player2 = input.player2;
            xi.button = input.button;
            replay.inputs.push_back(xi);
        }
        return replay;
    }

    static YBotFormat::Replay unifiedToYBot(const UnifiedReplay& unified) {
        YBotFormat::Replay replay;
        replay.fps = unified.tps;
        for (const auto& input : unified.inputs) {
            YBotFormat::Input yi;
            yi.frame = static_cast<uint64_t>(input.absoluteTime * unified.tps);
            yi.down = input.down;
            yi.player2 = input.player2;
            yi.button = input.button;
            replay.inputs.push_back(yi);
        }
        return replay;
    }

    static TCMFormat::Replay unifiedToTCM(const UnifiedReplay& unified) {
        TCMFormat::Replay replay;
        replay.fps = unified.tps;
        for (const auto& input : unified.inputs) {
            TCMFormat::Input ti;
            ti.frame = static_cast<uint32_t>(input.absoluteTime * unified.tps);
            ti.down = input.down;
            ti.player2 = input.player2;
            ti.button = input.button;
            replay.inputs.push_back(ti);
        }
        return replay;
    }

    static REFormat::Replay unifiedToRE(const UnifiedReplay& unified) {
        REFormat::Replay replay;
        replay.fps = unified.tps;
        for (const auto& input : unified.inputs) {
            REFormat::Input ri;
            ri.frame = static_cast<uint32_t>(input.absoluteTime * unified.tps);
            ri.down = input.down;
            ri.player2 = input.player2;
            ri.button = input.button;
            replay.inputs.push_back(ri);
        }
        return replay;
    }

    static ZBotFormat::Replay unifiedToZBot(const UnifiedReplay& unified) {
        ZBotFormat::Replay replay;
        replay.fps = unified.tps;
        for (const auto& input : unified.inputs) {
            ZBotFormat::Input zi;
            zi.frame = static_cast<uint32_t>(input.absoluteTime * unified.tps);
            zi.down = input.down;
            zi.player2 = input.player2;
            zi.button = input.button;
            replay.inputs.push_back(zi);
        }
        return replay;
    }

    static MHRFormat::Replay unifiedToMHR(const UnifiedReplay& unified) {
        MHRFormat::Replay replay;
        replay.fps = unified.tps;
        replay.levelId = unified.seed;
        for (const auto& input : unified.inputs) {
            MHRFormat::Input mi;
            mi.frame = static_cast<uint32_t>(input.absoluteTime * unified.tps);
            mi.down = input.down;
            mi.player2 = input.player2;
            mi.button = input.button;
            replay.inputs.push_back(mi);
        }
        return replay;
    }

    static EchoFormat::Replay unifiedToEcho(const UnifiedReplay& unified) {
        EchoFormat::Replay replay;
        replay.fps = unified.tps;
        for (const auto& input : unified.inputs) {
            EchoFormat::Input ei;
            ei.frame = static_cast<uint64_t>(input.absoluteTime * unified.tps);
            ei.down = input.down;
            ei.player2 = input.player2;
            ei.button = input.button;
            replay.inputs.push_back(ei);
        }
        return replay;
    }

    static PlaintextFormat::Replay unifiedToPlaintext(const UnifiedReplay& unified) {
        PlaintextFormat::Replay replay;
        replay.fps = unified.tps;
        for (const auto& input : unified.inputs) {
            PlaintextFormat::Input pi;
            pi.frame = static_cast<uint32_t>(input.absoluteTime * unified.tps);
            pi.down = input.down;
            pi.player2 = input.player2;
            pi.button = input.button;
            replay.inputs.push_back(pi);
        }
        return replay;
    }
};

} // namespace deepbot