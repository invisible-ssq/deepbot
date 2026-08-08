#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>

namespace deepbot {

class BinaryReader {
private:
    const uint8_t* m_data;
    size_t m_size;
    size_t m_pos;

public:
    BinaryReader(const std::vector<uint8_t>& data)
        : m_data(data.data()), m_size(data.size()), m_pos(0) {}

    BinaryReader(const uint8_t* data, size_t size)
        : m_data(data), m_size(size), m_pos(0) {}

    size_t position() const { return m_pos; }
    size_t remaining() const { return m_size - m_pos; }
    bool eof() const { return m_pos >= m_size; }

    void seek(size_t pos) {
        if (pos > m_size) throw std::runtime_error("Seek out of bounds");
        m_pos = pos;
    }

    void skip(size_t bytes) {
        if (m_pos + bytes > m_size) throw std::runtime_error("Skip out of bounds");
        m_pos += bytes;
    }

    template<typename T>
    T read() {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        if (m_pos + sizeof(T) > m_size) throw std::runtime_error("Read out of bounds");
        T value;
        std::memcpy(&value, m_data + m_pos, sizeof(T));
        m_pos += sizeof(T);
        return value;
    }

    uint8_t readU8() { return read<uint8_t>(); }
    uint16_t readU16() { return read<uint16_t>(); }
    uint32_t readU32() { return read<uint32_t>(); }
    uint64_t readU64() { return read<uint64_t>(); }
    int8_t readI8() { return read<int8_t>(); }
    int16_t readI16() { return read<int16_t>(); }
    int32_t readI32() { return read<int32_t>(); }
    int64_t readI64() { return read<int64_t>(); }
    float readF32() { return read<float>(); }
    double readF64() { return read<double>(); }
    bool readBool() { return readU8() != 0; }

    uint64_t readVarU64() {
        uint64_t value = 0;
        uint32_t shift = 0;
        while (true) {
            if (m_pos >= m_size) throw std::runtime_error("Varint EOF");
            uint8_t byte = m_data[m_pos++];
            value |= uint64_t(byte & 0x7F) << shift;
            if ((byte & 0x80) == 0) return value;
            shift += 7;
            if (shift >= 64) throw std::runtime_error("Varint overflow");
        }
    }

    int64_t readVarI64() {
        uint64_t encoded = readVarU64();
        return int64_t((encoded >> 1) ^ (-(encoded & 1)));
    }

    std::string readString() {
        uint16_t len = readU16();
        if (m_pos + len > m_size) throw std::runtime_error("String read out of bounds");
        std::string result(reinterpret_cast<const char*>(m_data + m_pos), len);
        m_pos += len;
        return result;
    }

    std::string readStringVar() {
        uint64_t len = readVarU64();
        if (len > remaining()) throw std::runtime_error("String var read out of bounds");
        std::string result(reinterpret_cast<const char*>(m_data + m_pos), len);
        m_pos += len;
        return result;
    }

    std::vector<uint8_t> readBytes(size_t count) {
        if (count > remaining()) throw std::runtime_error("Bytes read out of bounds");
        std::vector<uint8_t> result(m_data + m_pos, m_data + m_pos + count);
        m_pos += count;
        return result;
    }
};

} // namespace deepbot
