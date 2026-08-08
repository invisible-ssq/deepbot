#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "../utils/TPSFix.hpp"

namespace deepbot {

struct UnifiedInput {
    double absoluteTime;
    bool down;
    bool player2;
    uint8_t button;
    float x, y, rotation, yAccel;
};

struct UnifiedReplay {
    std::string author;
    std::string description;
    double tps = 240.0;
    double duration = 0.0;
    uint32_t seed = 0;
    std::vector<UnifiedInput> inputs;
};

class IFormatHandler {
public:
    virtual ~IFormatHandler() = default;
    virtual std::string getExtension() const = 0;
    virtual std::string getName() const = 0;
    virtual std::vector<uint8_t> write(const UnifiedReplay& replay) = 0;
    virtual UnifiedReplay read(const std::vector<uint8_t>& data) = 0;
};

class FormatRegistry {
private:
    std::unordered_map<std::string, std::unique_ptr<IFormatHandler>> m_handlers;

public:
    static FormatRegistry& instance() {
        static FormatRegistry inst;
        return inst;
    }

    void registerFormat(std::unique_ptr<IFormatHandler> handler) {
        m_handlers[handler->getExtension()] = std::move(handler);
    }

    IFormatHandler* getHandler(const std::string& ext) {
        auto it = m_handlers.find(ext);
        return (it != m_handlers.end()) ? it->second.get() : nullptr;
    }

    std::vector<std::string> getSupportedExtensions() const {
        std::vector<std::string> exts;
        for (const auto& [ext, _] : m_handlers) {
            exts.push_back(ext);
        }
        return exts;
    }
};

} // namespace deepbot