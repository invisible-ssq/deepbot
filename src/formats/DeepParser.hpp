#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <string>
#include <algorithm>
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
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (data.size() >= 4) {
            if (std::memcmp(data.data(), "DEEP", 4) == 0) return "deep";
            if (std::memcmp(data.data(), "TTR3", 4) == 0) return "ttr3";
            if (std::memcmp(data.data(), "GDR", 3) == 0) {
                if (data.size() > 3 && data[3] >= '0' && data[3] <= '9') return "gdr2";
                return "gdr";
            }
            if (std::memcmp(data.data(), "YBOT", 4) == 0) return "ybot";
            if (std::memcmp(data.data(), "ZBF", 3) == 0) return "zbf";
            if (std::memcmp(data.data(), "ECHO", 4) == 0) return "echo";
        }
        if (data.size() >= 3) {
            if (std::memcmp(data.data(), "XD", 2) == 0) return "xd";
            if (std::memcmp(data.data(), "RE2", 3) == 0) return "re2";
            if (std::memcmp(data.data(), "RE3", 3) == 0) return "re3";
            if (std::memcmp(data.data(), "RE4", 3) == 0) return "re4";
            if (std::memcmp(data.data(), "SILL", 4) == 0) return "slc";
        }
        if (data.size() >= 8 && std::memcmp(data.data(), "SLC3RPLY", 8) == 0) return "slc";

        if (data.size() > 0 && (data[0] == '{' || data[0] == '[')) {
            try {
                std::string content(data.begin(), data.end());
                auto parseResult = matjson::parse(content);
                if (parseResult.isOk()) {
                    auto j = parseResult.unwrap();
                    if (j.contains("framerate")) return "gdr";
                    if (j.contains("fps")) return "mhr";
                }
            } catch (...) {}
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

    static geode::Result<UnifiedReplay> parseFormat(const std::string& format, const std::vector<uint8_t>& data) {
        std::string fmt = format;
        std::transform(fmt.begin(), fmt.end(), fmt.begin(), ::tolower);

        try {
            if (fmt == "deep") {
                auto replay = DeepFormat::read(data);
                return geode::Ok(deepToUnified(replay));
            }
            if (fmt == "ttr3") {
                auto replay = TTR3Format::read(data);
                return geode::Ok(ttr3ToUnified(replay));
            }
            if (fmt == "gdr") {
                std::string jsonStr(data.begin(), data.end());
                auto replay = GDRFormat::readJSON(jsonStr);
                return geode::Ok(gdrToUnified(replay));
            }
            if (fmt == "gdr2") {
                auto replay = GDR2Format::read(data);
                return geode::Ok(gdr2ToUnified(replay));
            }
            if (fmt == "slc") {
                auto replay = SLCFormat::read(data);
                return geode::Ok(slcToUnified(replay));
            }
            if (fmt == "xd") {
                auto replay = XDFormat::read(data);
                return geode::Ok(xdToUnified(replay));
            }
            if (fmt == "ybot" || fmt == "ybf") {
                auto replay = YBotFormat::read(data);
                return geode::Ok(ybotToUnified(replay));
            }
            if (fmt == "tcm") {
                auto replay = TCMFormat::read(data);
                return geode::Ok(tcmToUnified(replay));
            }
            if (fmt == "re" || fmt == "re2" || fmt == "re3" || fmt == "re4") {
                auto replay = REFormat::read(data);
                return geode::Ok(reToUnified(replay));
            }
            if (fmt == "zbf") {
                auto replay = ZBotFormat::read(data);
                return geode::Ok(zbfToUnified(replay));
            }
            if (fmt == "mhr") {
                auto replay = MHRFormat::read(data);
                return geode::Ok(mhrToUnified(replay));
            }
            if (fmt == "echo") {
                auto replay = EchoFormat::read(data);
                return geode::Ok(echoToUnified(replay));
            }
            if (fmt == "txt") {
                auto replay = PlaintextFormat::read(data);
                return geode::Ok(txtToUnified(replay));
            }
        } catch (const std::exception& e) {
            return geode::Err("Parse error: " + std::string(e.what()));
        }

        return geode::Err("Unsupported format: " + format);
    }

    static geode::Result<std::vector<uint8_t>> serialize(const UnifiedReplay& replay, const std::string& format) {
        std::string fmt = format;
        std::transform(fmt.begin(), fmt.end(), fmt.begin(), ::tolower);

        try {
            if (fmt == "deep") {
                auto deep = unifiedToDeep(replay);
                return geode::Ok(DeepFormat::write(deep));
            }
            if (fmt == "ttr3") {
                auto ttr3 = unifiedToTTR3(replay);
                return geode::Ok(TTR3Format::write(ttr3));
            }
            if (fmt == "gdr") {
                auto gdr = unifiedToGDR(replay);
                auto json = GDRFormat::writeJSON(gdr);
                return geode::Ok(std::vector<uint8_t>(json.begin(), json.end()));
            }
            if (fmt == "gdr2") {
                auto gdr2 = unifiedToGDR2(replay);
                return geode::Ok(GDR2Format::write(gdr2));
            }
            if (fmt == "slc") {
                auto slc = unifiedToSLC(replay);
                return geode::Ok(SLCFormat::write(slc));
            }
            if (fmt == "xd") {
                auto xd = unifiedToXD(replay);
                return geode::Ok(XDFormat::write(xd));
            }
            if (fmt == "ybot") {
                auto ybot = unifiedToYBot(replay);
                return geode::Ok(YBotFormat::write(ybot));
            }
            if (fmt == "tcm") {
                auto tcm = unifiedToTCM(replay);
                return geode::Ok(TCMFormat::write(tcm));
            }
            if (fmt == "re" || fmt == "re2" || fmt == "re3" || fmt == "re4") {
                int version = 1;
                if (fmt == "re2") version = 2;
                else if (fmt == "re3") version = 3;
                else if (fmt == "re4") version = 4;
                auto re = unifiedToRE(replay, version);
                return geode::Ok(REFormat::write(re, version));
            }
            if (fmt == "zbf") {
                auto zbf = unifiedToZBF(replay);
                return geode::Ok(ZBotFormat::write(zbf));
            }
            if (fmt == "mhr") {
                auto mhr = unifiedToMHR(replay);
                return geode::Ok(MHRFormat::write(mhr));
            }
            if (fmt == "echo") {
                auto echo = unifiedToEcho(replay);
                return geode::Ok(EchoFormat::write(echo));
            }
            if (fmt == "txt") {
                auto txt = unifiedToTxt(replay);
                return geode::Ok(PlaintextFormat::write(txt));
            }
        } catch (const std::exception& e) {
            return geode::Err("Serialize error: " + std::string(e.what()));
        }

        return geode::Err("Unsupported format for serialization: " + format);
    }

    static std::vector<std::string> getSupportedFormats() {
        return {"deep", "ttr3", "gdr", "gdr2", "slc", "xd", "ybot", "ybf",
                "tcm", "re", "re2", "re3", "re4", "zbf", "mhr", "echo", "txt"};
    }

    // ... (все toUnified/unifiedTo методы без изменений)
    static UnifiedReplay deepToUnified(const DeepFormat::Replay& replay) {
        UnifiedReplay u;
        u.author = replay.author;
        u.description = replay.description;
        u.tps = replay.tps;
        u.duration = replay.duration;
        u.seed = replay.seed;
        for (const auto& inp : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = inp.absoluteTime;
            ui.down = (inp.flags & 1) != 0;
            ui.player2 = (inp.flags & 2) != 0;
            ui.button = (inp.flags >> 2) & 3;
            normalizeButton(ui.button);
            ui.x = inp.x; ui.y = inp.y;
            ui.rotation = inp.rotation;
            ui.yAccel = inp.yAccel;
            u.inputs.push_back(ui);
        }
        return u;
    }

    static DeepFormat::Replay unifiedToDeep(const UnifiedReplay& replay) {
        DeepFormat::Replay d;
        d.author = replay.author;
        d.description = replay.description;
        d.tps = replay.tps;
        d.duration = replay.duration;
        d.seed = replay.seed;
        for (const auto& inp : replay.inputs) {
            DeepInput di;
            di.absoluteTime = inp.absoluteTime;
            di.flags = (inp.down ? 1 : 0) | (inp.player2 ? 2 : 0) | ((inp.button & 3) << 2);
            di.x = inp.x; di.y = inp.y;
            di.rotation = inp.rotation;
            di.yAccel = inp.yAccel;
            d.inputs.push_back(di);
        }
        return d;
    }

    static UnifiedReplay ttr3ToUnified(const TTR3Format::Replay& replay) {
        UnifiedReplay u;
        u.tps = replay.fps;
        for (const auto& inp : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = inp.timeSeconds;
            ui.down = (inp.actionType & 1) != 0;
            ui.player2 = (inp.flags & 1) != 0;
            ui.button = (inp.flags >> 1) & 3;
            normalizeButton(ui.button);
            u.inputs.push_back(ui);
        }
        return u;
    }

    static TTR3Format::Replay unifiedToTTR3(const UnifiedReplay& replay) {
        TTR3Format::Replay t;
        t.fps = replay.tps;
        for (const auto& inp : replay.inputs) {
            TTR3Format::Input ti;
            ti.timeSeconds = inp.absoluteTime;
            ti.actionType = inp.down ? 1 : 0;
            ti.flags = (inp.player2 ? 1 : 0) | ((inp.button & 3) << 1);
            ti.reserved = 0;
            t.inputs.push_back(ti);
        }
        return t;
    }

    static UnifiedReplay gdrToUnified(const GDRFormat::Replay& replay) {
        UnifiedReplay u;
        u.author = replay.author;
        u.description = replay.description;
        u.tps = replay.framerate;
        u.seed = replay.seed;
        for (const auto& inp : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = inp.frame / replay.framerate;
            ui.down = inp.down;
            ui.player2 = inp.player2;
            ui.button = inp.button;
            normalizeButton(ui.button);
            u.inputs.push_back(ui);
        }
        return u;
    }

    static GDRFormat::Replay unifiedToGDR(const UnifiedReplay& replay) {
        GDRFormat::Replay g;
        g.author = replay.author;
        g.description = replay.description;
        g.framerate = static_cast<int>(replay.tps);
        g.seed = replay.seed;
        for (const auto& inp : replay.inputs) {
            GDRFormat::Input gi;
            gi.frame = static_cast<int>(inp.absoluteTime * replay.tps);
            gi.down = inp.down;
            gi.player2 = inp.player2;
            gi.button = inp.button;
            g.inputs.push_back(gi);
        }
        return g;
    }

    static UnifiedReplay gdr2ToUnified(const GDR2Format::Replay& replay) {
        UnifiedReplay u;
        u.author = replay.author;
        u.description = replay.description;
        u.tps = replay.framerate;
        u.seed = replay.seed;
        for (const auto& inp : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = inp.frame / replay.framerate;
            ui.down = inp.down;
            ui.player2 = inp.player2;
            ui.button = inp.button;
            normalizeButton(ui.button);
            if (inp.physics) {
                ui.x = inp.physics->xPosition;
                ui.y = inp.physics->yPosition;
                ui.rotation = inp.physics->rotation;
            }
            u.inputs.push_back(ui);
        }
        return u;
    }

    static GDR2Format::Replay unifiedToGDR2(const UnifiedReplay& replay) {
        GDR2Format::Replay g;
        g.author = replay.author;
        g.description = replay.description;
        g.framerate = replay.tps;
        g.seed = replay.seed;
        for (const auto& inp : replay.inputs) {
            GDR2Format::Input gi;
            gi.frame = static_cast<int>(inp.absoluteTime * replay.tps);
            gi.down = inp.down;
            gi.player2 = inp.player2;
            gi.button = inp.button;
            g.inputs.push_back(std::move(gi));
        }
        return g;
    }

    static UnifiedReplay slcToUnified(const SLCFormat::Replay& replay) {
        UnifiedReplay u;
        u.tps = replay.fps;
        u.seed = static_cast<uint32_t>(replay.seed);
        for (const auto& inp : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = inp.frame / replay.fps;
            ui.down = inp.down;
            ui.player2 = inp.player2;
            ui.button = inp.button;
            normalizeButton(ui.button);
            u.inputs.push_back(ui);
        }
        return u;
    }

    static SLCFormat::Replay unifiedToSLC(const UnifiedReplay& replay) {
        SLCFormat::Replay s;
        s.fps = replay.tps;
        s.seed = replay.seed;
        for (const auto& inp : replay.inputs) {
            SLCFormat::Input si;
            si.frame = static_cast<int>(inp.absoluteTime * replay.tps);
            si.down = inp.down;
            si.player2 = inp.player2;
            si.button = inp.button;
            s.inputs.push_back(si);
        }
        return s;
    }

    static UnifiedReplay xdToUnified(const XDFormat::Replay& replay) {
        UnifiedReplay u;
        u.tps = replay.fps;
        for (const auto& inp : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = inp.frame / replay.fps;
            ui.down = inp.down;
            ui.player2 = inp.player2;
            ui.button = inp.button;
            normalizeButton(ui.button);
            u.inputs.push_back(ui);
        }
        return u;
    }

    static XDFormat::Replay unifiedToXD(const UnifiedReplay& replay) {
        XDFormat::Replay x;
        x.fps = replay.tps;
        for (const auto& inp : replay.inputs) {
            XDFormat::Input xi;
            xi.frame = static_cast<int>(inp.absoluteTime * replay.tps);
            xi.down = inp.down;
            xi.player2 = inp.player2;
            xi.button = inp.button;
            x.inputs.push_back(xi);
        }
        return x;
    }

    static UnifiedReplay ybotToUnified(const YBotFormat::Replay& replay) {
        UnifiedReplay u;
        u.tps = replay.fps;
        for (const auto& inp : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = inp.frame / replay.fps;
            ui.down = inp.down;
            ui.player2 = inp.player2;
            ui.button = inp.button;
            normalizeButton(ui.button);
            u.inputs.push_back(ui);
        }
        return u;
    }

    static YBotFormat::Replay unifiedToYBot(const UnifiedReplay& replay) {
        YBotFormat::Replay y;
        y.fps = replay.tps;
        for (const auto& inp : replay.inputs) {
            YBotFormat::Input yi;
            yi.frame = static_cast<int>(inp.absoluteTime * replay.tps);
            yi.down = inp.down;
            yi.player2 = inp.player2;
            yi.button = inp.button;
            y.inputs.push_back(yi);
        }
        return y;
    }

    static UnifiedReplay tcmToUnified(const TCMFormat::Replay& replay) {
        UnifiedReplay u;
        u.tps = replay.fps;
        for (const auto& inp : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = inp.frame / replay.fps;
            ui.down = inp.down;
            ui.player2 = inp.player2;
            ui.button = inp.button;
            normalizeButton(ui.button);
            u.inputs.push_back(ui);
        }
        return u;
    }

    static TCMFormat::Replay unifiedToTCM(const UnifiedReplay& replay) {
        TCMFormat::Replay t;
        t.fps = replay.tps;
        for (const auto& inp : replay.inputs) {
            TCMFormat::Input ti;
            ti.frame = static_cast<int>(inp.absoluteTime * replay.tps);
            ti.down = inp.down;
            ti.player2 = inp.player2;
            ti.button = inp.button;
            t.inputs.push_back(ti);
        }
        return t;
    }

    static UnifiedReplay reToUnified(const REFormat::Replay& replay) {
        UnifiedReplay u;
        u.tps = replay.fps;
        for (const auto& inp : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = inp.frame / replay.fps;
            ui.down = inp.down;
            ui.player2 = inp.player2;
            ui.button = inp.button;
            normalizeButton(ui.button);
            u.inputs.push_back(ui);
        }
        return u;
    }

    static REFormat::Replay unifiedToRE(const UnifiedReplay& replay, int version) {
        REFormat::Replay r;
        r.fps = replay.tps;
        r.version = version;
        for (const auto& inp : replay.inputs) {
            REFormat::Input ri;
            ri.frame = static_cast<int>(inp.absoluteTime * replay.tps);
            ri.down = inp.down;
            ri.player2 = inp.player2;
            ri.button = inp.button;
            r.inputs.push_back(ri);
        }
        return r;
    }

    static UnifiedReplay zbfToUnified(const ZBotFormat::Replay& replay) {
        UnifiedReplay u;
        u.tps = replay.fps;
        for (const auto& inp : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = inp.frame / replay.fps;
            ui.down = inp.down;
            ui.player2 = inp.player2;
            ui.button = inp.button;
            normalizeButton(ui.button);
            u.inputs.push_back(ui);
        }
        return u;
    }

    static ZBotFormat::Replay unifiedToZBF(const UnifiedReplay& replay) {
        ZBotFormat::Replay z;
        z.fps = replay.tps;
        for (const auto& inp : replay.inputs) {
            ZBotFormat::Input zi;
            zi.frame = static_cast<int>(inp.absoluteTime * replay.tps);
            zi.down = inp.down;
            zi.player2 = inp.player2;
            zi.button = inp.button;
            z.inputs.push_back(zi);
        }
        return z;
    }

    static UnifiedReplay mhrToUnified(const MHRFormat::Replay& replay) {
        UnifiedReplay u;
        u.tps = replay.fps;
        for (const auto& inp : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = inp.frame / replay.fps;
            ui.down = inp.down;
            ui.player2 = inp.player2;
            ui.button = inp.button;
            normalizeButton(ui.button);
            u.inputs.push_back(ui);
        }
        return u;
    }

    static MHRFormat::Replay unifiedToMHR(const UnifiedReplay& replay) {
        MHRFormat::Replay m;
        m.fps = replay.tps;
        m.levelName = "";
        m.levelId = 0;
        for (const auto& inp : replay.inputs) {
            MHRFormat::Input mi;
            mi.frame = static_cast<int>(inp.absoluteTime * replay.tps);
            mi.down = inp.down;
            mi.player2 = inp.player2;
            mi.button = inp.button;
            m.inputs.push_back(mi);
        }
        return m;
    }

    static UnifiedReplay echoToUnified(const EchoFormat::Replay& replay) {
        UnifiedReplay u;
        u.tps = replay.fps;
        for (const auto& inp : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = inp.frame / replay.fps;
            ui.down = inp.down;
            ui.player2 = inp.player2;
            ui.button = inp.button;
            normalizeButton(ui.button);
            u.inputs.push_back(ui);
        }
        return u;
    }

    static EchoFormat::Replay unifiedToEcho(const UnifiedReplay& replay) {
        EchoFormat::Replay e;
        e.fps = replay.tps;
        for (const auto& inp : replay.inputs) {
            EchoFormat::Input ei;
            ei.frame = static_cast<int>(inp.absoluteTime * replay.tps);
            ei.down = inp.down;
            ei.player2 = inp.player2;
            ei.button = inp.button;
            e.inputs.push_back(ei);
        }
        return e;
    }

    static UnifiedReplay txtToUnified(const PlaintextFormat::Replay& replay) {
        UnifiedReplay u;
        u.tps = replay.fps;
        for (const auto& inp : replay.inputs) {
            UnifiedInput ui;
            ui.absoluteTime = inp.frame / replay.fps;
            ui.down = inp.down;
            ui.player2 = inp.player2;
            ui.button = inp.button;
            normalizeButton(ui.button);
            u.inputs.push_back(ui);
        }
        return u;
    }

    static PlaintextFormat::Replay unifiedToTxt(const UnifiedReplay& replay) {
        PlaintextFormat::Replay t;
        t.fps = replay.tps;
        for (const auto& inp : replay.inputs) {
            PlaintextFormat::Input ti;
            ti.frame = static_cast<int>(inp.absoluteTime * replay.tps);
            ti.down = inp.down;
            ti.player2 = inp.player2;
            ti.button = inp.button;
            t.inputs.push_back(ti);
        }
        return t;
    }
};

} // namespace deepbot
