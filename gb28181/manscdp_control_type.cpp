#include "manscdp_control_type.h"

#include <sstream>
#include <iomanip>
#include <cstdint>
#include <array>
#include <stdexcept>
#include <algorithm>

namespace gb28181 {

static uint8_t hexCharToNibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    throw std::invalid_argument("Invalid hex character");
}

static std::array<uint8_t, 8> parseHexString(const std::string& hexStr) {
    std::string clean;
    for (char ch : hexStr) {
        if (ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r') {
            clean.push_back(ch);
        }
    }
    if (clean.length() != 16) {
        throw std::invalid_argument("cmd hex string must be 16 hex chars (8 bytes)");
    }
    std::array<uint8_t, 8> bytes;
    for (size_t i = 0; i < 8; ++i) {
        uint8_t high = hexCharToNibble(clean[i * 2]);
        uint8_t low = hexCharToNibble(clean[i * 2 + 1]);
        bytes[i] = (high << 4) | low;
    }
    return bytes;
}


std::string CreateCmdString(uint8_t cmdCode, uint8_t parameter1, uint8_t parameter2, uint8_t combineCode2) {
    // 前2个字节固定为A50F，地址(12位)固定为0x001，第3个字节为地址低8位，固定为0x01，字节7的低4位为地址高4位，固定为0x0
    uint8_t bytes[8] = {
        0xA5,                       // 字节1
        0x0F,                       // 字节2（版本0，校验位 = (0xA+0x5+0)%16 = 0xF）
        0x01,                       // 字节3（地址低8位）
        (cmdCode & 0xFF),           // 字节4：指令码
        (parameter1 & 0xFF),        // 字节5：数据1
        (parameter2 & 0xFF),        // 字节6：数据2
        ((combineCode2 & 0x0F) << 4), // 字节7：组合码2（低4位为地址高4位，这里补0）
        0x00                        // 字节8：校验码，待计算
    };
    //uint16_t addr = 0x0001;
    //if (addr > 0x0FFF) throw std::out_of_range("Address must be 0~4095");
    //bytes[2] = static_cast<uint8_t>(addr & 0xFF);                   // 低8位 -> 字节3
    //bytes[6] = ((combineCode2 & 0x0F) << 4) | ((addr >> 8) & 0x0F); // 高4位 -> 字节7低4位

    // 计算校验和：前7个字节之和取低8位
    int sum = 0;
    for (int i = 0; i < 7; ++i) {
        sum += bytes[i];
    }
    bytes[7] = static_cast<uint8_t>(sum & 0xFF);

    // 格式化为十六进制字符串（无空格，如 "A50F0101..."）
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0');
    for (int i = 0; i < 8; ++i) {
        oss << std::setw(2) << static_cast<int>(bytes[i]);
    }
    return oss.str();
}


std::string PTZCmdType::ToString() const
{
    uint8_t cmdCode = 0x00;

    // 映射 leftRight（bit0=右，bit1=左）
    if (leftRight > 0) cmdCode |= 0x01; // 右
    else if (leftRight < 0) cmdCode |= 0x02; // 左

    // 映射 upDown（bit2=下，bit3=上）
    if (upDown > 0) cmdCode |= 0x04; // 下
    else if (upDown < 0) cmdCode |= 0x08; // 上

    // 映射 inOut（bit4=放大，bit5=缩小）
    if (inOut > 0) cmdCode |= 0x10; // 放大+（In）
    else if (inOut < 0) cmdCode |= 0x20; // 缩小-（Out）

    return CreateCmdString(cmdCode, horSpeed, verSpeed, zoomSpeed);
}

void PTZCmdType::FromString(const std::string& str)
{
    auto bytes = parseHexString(str);

    // 校验和验证
    uint16_t sum = 0;
    for (int i = 0; i < 7; ++i) sum += bytes[i];
    bool checksumOk = (static_cast<uint8_t>(sum & 0xFF) == bytes[7]);
    if (!checksumOk)
        return;

    // 12位地址（字节3低8位 + 字节7低4位）
    //uint16_t address = bytes[2] | ((bytes[6] & 0x0F) << 8); // 低4位为地址高4位

    uint8_t cmdCode = bytes[3];
    if ((cmdCode & 0x01) != 0) {
        leftRight = 1;
    } else if ((cmdCode & 0x02) != 0) {
        leftRight = -1;
    }
    if ((cmdCode & 0x04) != 0) {
        upDown = 1;
    } else if ((cmdCode & 0x08) != 0) {
        upDown = -1;
    }
    if ((cmdCode & 0x10) != 0) {
        inOut = 1;
    } else if ((cmdCode & 0x20) != 0) {
        inOut = -1;
    }
    horSpeed = bytes[4];
    verSpeed = bytes[5];
    zoomSpeed = bytes[6] >> 4;
}

} // namespace gb28181