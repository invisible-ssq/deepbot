#pragma once
#include <vector>
#include <string>
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
        auto j = matjson::parse(jsonStr);
        Replay replay;
        replay.author = j.value("author", "");
        replay.description = j.value("description", "");
        replay.duration = j.value("duration", 0.0f);
        replay.gameVersion = j.value("gameVersion", 0.0f);
        replay.version = j.value("version", 1.0f);
        replay.framerate = j.value("framerate", 240.0f);
        replay.seed = j.value("seed", 0);
        replay.coins = j.value("coins", 0);
        replay.ldm = j.value("ldm", false);
        if (j.contains("bot")) {
            replay.bot.name = j["bot"].value("name", "");
            replay.bot.version = j["bot"].value("version", "");
        }
        if (j.contains("level")) {
            replay.level.id = j["level"].value("id", 0);
            replay.level.name = j["level"].value("name", "");
        }
        if (j.contains("inputs")) {
            for (const auto& inp : j["inputs"]) {
                Input input;
                input.player2 = inp.value("2p", false);
                input.button = inp.value("btn", 1);
                input.down = inp.value("down", false);
                input.frame = inp.value("frame", 0);
                if (inp.contains("correction")) {
                    auto& c = inp["correction"];
                    input.correction.nodeXPos = c.value("nodeXPos", 0.0f);
                    input.correction.nodeYPos = c.value("nodeYPos", 0.0f);
                    input.correction.player2 = c.value("player2", false);
                    input.correction.rotation = c.value("rotation", 0.0f);
                    input.correction.xPos = c.value("xPos", 0.0f);
                    input.correction.yPos = c.value("yPos", 0.0f);
                    input.correction.yVel = c.value("yVel", 0.0f);
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
        j["bot"] = matjson::Object{{"name", replay.bot.name}, {"version", replay.bot.version}};
        j["level"] = matjson::Object{{"id", replay.level.id}, {"name", replay.level.name}};

        matjson::Value inputs = matjson::Array();
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

            inputs.asArray().push_back(inp);
        }
        j["inputs"] = inputs;

        return j.dump(2);
    }
};

} // namespace deepbot