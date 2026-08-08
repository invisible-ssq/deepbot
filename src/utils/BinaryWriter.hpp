#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>

namespace deepbot {

class BinaryWriter {
private:
    std::vector<uint8_t> m_data;

public:
    BinaryWriter() = default;

    const std::vector<uint8_t>& data() const { return m_data; }
    std::vector<uint8_t>&& intoVec() { return std::move(m_data); }
    size_t size() const { return m_data.size(); }

    void writeBytes(const uint8_t* data, size_t len) {
        m_data.insert(m_data.end(), data, data + len);
    }

    template<typename T>
    void write(T value) {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        uint8_t bytes[sizeof(T)];
        std::memcpy(bytes, &value, sizeof(T));
        m_data.insert(m_data.end(), bytes, bytes + sizeof(T));
    }

    void writeU8(uint8_t v) { write(v); }
    void writeU16(uint16_t v) { write(v); }
    void writeU32(uint32_t v) { write(v); }
    void writeU64(uint64_t v) { write(v); }
    void writeI8(int8_t v) { write(v); }
    void writeI16(int16_t v) { write(v); }
    void writeI32(int32_t v) { write(v); }
    void writeI64(int64_t v) { write(v); }
    void writeF32(float v) { write(v); }
    void writeF64(double v) { write(v); }
    void writeBool(bool v) { writeU8(v ? 1 : 0); }

    void writeVarU64(uint64_t value) {
        while (value >= 0x80) {
            writeU8((value & 0x7F) | 0x80);
            value >>= 7;
        }
        writeU8(value);
    }

    void writeVarI64(int64_t value) {
        writeVarU64((value << 1) ^ (value >> 63));
    }

    void writeString(const std::string& str) {
        uint16_t len = static_cast<uint16_t>(std::min(str.size(), size_t(UINT16_MAX)));
        if (len < str.size()) {
            // Silently truncated - consider adding warning
        }
        writeU16(len);
        writeBytes(reinterpret_cast<const uint8_t*>(str.data()), len);
    }

    void writeStringVar(const std::string& str) {
        writeVarU64(str.size());
        writeBytes(reinterpret_cast<const uint8_t*>(str.data()), str.size());
    }
    
    // Overwrite data at specific offset (for fixing header sizes after the fact)
    void overwrite(size_t offset, const void* data, size_t len) {
        if (offset + len > m_data.size()) throw std::runtime_error("Overwrite out of bounds");
        std::memcpy(m_data.data() + offset, data, len);
    }
};

} // namespace deepbot
