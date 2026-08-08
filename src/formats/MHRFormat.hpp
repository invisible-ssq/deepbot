#pragma once
#include <vector>
#include <string>
#include <matjson.hpp>
#include "../utils/BinaryReader.hpp"

namespace deepbot {

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
        matjson::Value j;
        j["fps"] = replay.fps;
        j["levelName"] = replay.levelName;
        j["levelId"] = replay.levelId;

        std::vector<matjson::Value> inputs;
        for (const auto& input : replay.inputs) {
            matjson::Value inp;
            inp["frame"] = input.frame;
            inp["down"] = input.down;
            inp["player2"] = input.player2;
            inp["button"] = input.button;
            inputs.push_back(inp);
        }
        j["inputs"] = inputs;

        std::string jsonStr = j.dump();
        return std::vector<uint8_t>(jsonStr.begin(), jsonStr.end());
    }

    static Replay read(const std::vector<uint8_t>& data) {
        std::string jsonStr(data.begin(), data.end());
        auto val = matjson::Value::fromString(jsonStr).unwrapOr(matjson::Value());
        Replay replay;

        replay.fps = val["fps"].asDouble().unwrapOr(240.0);
        replay.levelName = val["levelName"].asString().unwrapOr("");
        replay.levelId = val["levelId"].asInt().unwrapOr(0);

        if (val.contains("inputs")) {
            for (const auto& inp : val["inputs"].asArray().unwrapOr(std::vector<matjson::Value>())) {
                Input input;
                input.frame = inp["frame"].asInt().unwrapOr(0);
                input.down = inp["down"].asBool().unwrapOr(false);
                input.player2 = inp["player2"].asBool().unwrapOr(false);
                input.button = inp["button"].asInt().unwrapOr(1);
                DeepParser::normalizeButton(input.button);
                replay.inputs.push_back(input);
            }
        }

        return replay;
    }
};

} // namespace deepbot
