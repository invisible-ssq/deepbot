#pragma once
#include <string>
#include <vector>
#include <matjson.hpp>
#include "../utils/BinaryReader.hpp"

namespace deepbot {

class GDRFormat {
public:
    struct Correction {
        float nodeXPos = 0, nodeYPos = 0;
        bool player2 = false;
        float rotation = 0, rotationRate = 0;
        float time = 0;
        float xPos = 0, xVel = 0;
        float yPos = 0, yVel = 0;
    };

    struct Input {
        bool player2 = false;
        int32_t button = 1;
        Correction correction;
        bool down = false;
        uint32_t frame = 0;
    };

    struct Replay {
        std::string author;
        std::string description;
        float duration = 0;
        float gameVersion = 0;
        float version = 1.0f;
        float framerate = 240.0f;
        int32_t seed = 0;
        int32_t coins = 0;
        bool ldm = false;
        struct { std::string name, version; } bot;
        struct { uint32_t id; std::string name; } level;
        std::vector<Input> inputs;
    };

    static Replay readJSON(const std::string& jsonStr) {
        auto val = matjson::Value::fromString(jsonStr).unwrapOr(matjson::Value());
        Replay replay;
        replay.author = val["author"].asString().unwrapOr("");
        replay.description = val["description"].asString().unwrapOr("");
        replay.duration = val["duration"].asDouble().unwrapOr(0.0f);
        replay.gameVersion = static_cast<float>(val["gameVersion"].asDouble().unwrapOr(0.0));
        replay.version = static_cast<float>(val["version"].asDouble().unwrapOr(1.0));
        replay.framerate = static_cast<float>(val["framerate"].asDouble().unwrapOr(240.0));
        replay.seed = val["seed"].asInt().unwrapOr(0);
        replay.coins = val["coins"].asInt().unwrapOr(0);
        replay.ldm = val["ldm"].asBool().unwrapOr(false);
        if (val.contains("bot")) {
            replay.bot.name = val["bot"]["name"].asString().unwrapOr("");
            replay.bot.version = val["bot"]["version"].asString().unwrapOr("");
        }
        if (val.contains("level")) {
            replay.level.id = val["level"]["id"].asInt().unwrapOr(0);
            replay.level.name = val["level"]["name"].asString().unwrapOr("");
        }
        if (val.contains("inputs")) {
            for (const auto& inp : val["inputs"].asArray().unwrapOr(std::vector<matjson::Value>())) {
                Input input;
                input.player2 = inp["2p"].asBool().unwrapOr(false);
                input.button = inp["btn"].asInt().unwrapOr(1);
                input.down = inp["down"].asBool().unwrapOr(false);
                input.frame = inp["frame"].asInt().unwrapOr(0);
                if (inp.contains("correction")) {
                    auto& c = inp["correction"];
                    input.correction.nodeXPos = static_cast<float>(c["nodeXPos"].asDouble().unwrapOr(0.0));
                    input.correction.nodeYPos = static_cast<float>(c["nodeYPos"].asDouble().unwrapOr(0.0));
                    input.correction.player2 = c["player2"].asBool().unwrapOr(false);
                    input.correction.rotation = static_cast<float>(c["rotation"].asDouble().unwrapOr(0.0));
                    input.correction.xPos = static_cast<float>(c["xPos"].asDouble().unwrapOr(0.0));
                    input.correction.yPos = static_cast<float>(c["yPos"].asDouble().unwrapOr(0.0));
                    input.correction.yVel = static_cast<float>(c["yVel"].asDouble().unwrapOr(0.0));
                }
                replay.inputs.push_back(input);
            }
        }
        return replay;
    }

    static std::string writeJSON(const Replay& replay) {
        matjson::Value j;
        j["author"] = replay.author;
        j["description"] = replay.description;
        j["duration"] = replay.duration;
        j["gameVersion"] = replay.gameVersion;
        j["version"] = replay.version;
        j["framerate"] = replay.framerate;
        j["seed"] = replay.seed;
        j["coins"] = replay.coins;
        j["ldm"] = replay.ldm;
        j["bot"] = matjson::Value::object();
        j["bot"]["name"] = replay.bot.name;
        j["bot"]["version"] = replay.bot.version;
        j["level"] = matjson::Value::object();
        j["level"]["id"] = replay.level.id;
        j["level"]["name"] = replay.level.name;
        std::vector<matjson::Value> inputs;
        for (const auto& input : replay.inputs) {
            matjson::Value inp;
            inp["2p"] = input.player2;
            inp["btn"] = input.button;
            inp["down"] = input.down;
            inp["frame"] = input.frame;
            matjson::Value corr;
            corr["nodeXPos"] = input.correction.nodeXPos;
            corr["nodeYPos"] = input.correction.nodeYPos;
            corr["player2"] = input.correction.player2;
            corr["rotation"] = input.correction.rotation;
            corr["xPos"] = input.correction.xPos;
            corr["yPos"] = input.correction.yPos;
            corr["yVel"] = input.correction.yVel;
            inp["correction"] = corr;
            inputs.push_back(inp);
        }
        j["inputs"] = inputs;
        return j.dump(2);
    }
};

} // namespace deepbot
