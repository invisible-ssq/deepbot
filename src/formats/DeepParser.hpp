#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <nlohmann/json.hpp>
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
    // Normalize button (ensure it's never 0)
    static void normalizeButton(uint8_t& button) {
        if (button == 0) button = 1;
    }

    // Format detection by file extension and content
    static std::string detectFormat(const std::string& path, const std::vector<uint8_t>& data) {
        std::string ext = path.substr(path.find_last_of(".") + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        // Check magic bytes for binary formats
        if (data.size() >= 4) {
            if (std::memcmp(data.data(), "DEEP", 4) == 0) return "deep";
            if (std::memcmp(data.data(), "TTR3", 4) == 0) return "ttr3";
            if (std::memcmp(data.data(), "GDR", 3) == 0) {
                if (data.size() > 3 && data[3] >= '0' && data[3] <= '9') return "gdr2";
                return "gdr"; // FIXED: was "gdr2"
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

        // Check for JSON formats using proper JSON validation
        if (data.size() > 0 && (data[0] == '{' || data[0] == '[')) {
            try {
                std::string content(data.begin(), data.end());
                // FIXED: validate JSON before parsing
                if (nlohmann::json::accept(content)) {
                    auto j = nlohmann::json::parse(content);
                    if (j.contains("framerate")) return "gdr";
                    if (j.contains("fps")) return "mhr";
                }
            } catch (...) {
                // Not valid JSON, fall through
            }
        }

        // Check for text format
        if (data.size() > 0 && (data[0] == '#' || (data[0] >= '0' && data[0] <= '9'))) {
            return "txt";
        }

        // Fallback to extension
        if (ext == "deep" || ext == "ttr3" || ext == "gdr" || ext == "gdr2" ||
            ext == "slc" || ext == "cml" || ext == "xd" || ext == "ybot" ||
            ext == "ybf" || ext == "tcm" || ext == "re" || ext == "re2" ||
            ext == "re3" || ext == "re4" || ext == "zbf" || ext == "mhr" ||
            ext == "echo" || ext == "txt") {
            return ext;
        }

        return "";
    }

    // ... rest of the file unchanged ...
    // (parse, parseFormat, serialize, getSupportedFormats, getFormatDescription,
    //  and all conversion helpers remain the same as in repo)
