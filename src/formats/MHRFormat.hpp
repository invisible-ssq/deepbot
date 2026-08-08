#pragma once
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "../utils/BinaryReader.hpp"

namespace deepbot {

// MegaHack Replay Format (.mhr)
// JSON-based format

class MHRFormat {
public:
    struct Input {
        uint32_t frame;
        bool player2;
        bool down;
        uint8_t button;
    };

    struct Replay {
        double fps = 240.0;
        std::string levelName;
        int32_t levelId = 0;
        std::vector<Input> inputs;
    };

    static std::vector<uint8_t> write(const Replay& replay) {
        nlohmann::json j;
        j["fps"] = replay.fps;
        j["levelName"] = replay.levelName;
        j["levelId"] = replay.levelId;

        nlohmann::json inputs = nlohmann::json::array();
        for (const auto& input : replay.inputs) {
            nlohmann::json inp;
            inp["frame"] = input.frame;
            inp["down"] = input.down;
            inp["player2"] = input.player2;
            inp["button"] = input.button;
            inputs.push_back(inp);
        }
        j["inputs"] = inputs;

        std::string jsonStr = j.dump(); // Compact, no indent
        return std::vector<uint8_t>(jsonStr.begin(), jsonStr.end());
    }

    static Replay read(const std::vector<uint8_t>& data) {
        std::string jsonStr(data.begin(), data.end());
        auto j = nlohmann::json::parse(jsonStr);
        Replay replay;

        replay.fps = j.value("fps", 240.0);
        replay.levelName = j.value("levelName", "");
        replay.levelId = j.value("levelId", 0);

        if (j.contains("inputs")) {
            for (const auto& inp : j["inputs"]) {
                Input input;
                input.frame = inp.value("frame", 0);
                input.down = inp.value("down", false);
                input.player2 = inp.value("player2", false);
                input.button = inp.value("button", 1);
                DeepParser::normalizeButton(input.button);
                replay.inputs.push_back(input);
            }
        }

        return replay;
    }
};

} // namespace deepbot
