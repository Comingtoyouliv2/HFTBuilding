#include <array> // std::array
#include <cstdint> // std::byte, std::to_integer
#include <cstddef> // int type (0x... )
#include <iostream> // std::cout
#include <iomanip>  // std::hex, std:;setw, std::setfill (hex 출력 format) 
#include <string>

/*
Objective: NASDAQ ITCH 5.0 ADD Order Message Parsing
1. 위 36바이트를 배열에 담기
2. 배열을 돌면서 각 바이트를 2자리 hex로 출력
3. 체크포인트(최소): 36개 바이트가 hex로 쭉 찍히면 성공
4. 보너스(여유 되면): 한 줄에 16바이트씩, 줄 앞에 오프셋 붙이기 — 진짜 hex 에디터처럼:
0000  41 00 01 00 00 00 00 00 00 0a 00 00 00 00 00 00
0010  00 00 2a 42 00 00 00 64 41 41 50 4c 20 20 20 20
0020  00 01 86 a0

ITCH Add Order Message: 36byte 
ITCH Cancel Order Message: 23Byte
ITCH Delete Message: 19BByte

Field           byte(hex)                     (Ref: menaing)
Type            41                            'A' = Add Order
Stock Locate    00 01
Tracking        00 00
Timestamp       00 00 00 00 0a 00
Order Ref       00 00 00 00 00 00 00 2a       주문번호 42
Buy/Sell        42                            'B' = 매수
Shares          00 00 00 64                   100주
Stock           41 41 50 4c 20 20 20 20       "AAPL    "
Price           00 01 86 a0                   100000 (= $10.0000)

*/
struct AddOrderMessage {
    char type;
    uint16_t stockLocate;
    uint16_t tracking;
    uint64_t timestamp; // 6byte 이지만, uint64_t 에 저장
    uint64_t orderReference;
    char side;
    uint32_t shares;
    char stock[8];
    uint32_t price;
}; 

class ByteParser {
public:
    ByteParser(const std::array<uint8_t, 36> buffer) : buffer_{buffer} {}

    uint8_t readUint8() {
        return buffer_[offset_++];
    }
 
    uint16_t readUint16() {
        // 2 sequential bytes are integrate as a uint16_t (16bit int)
        // 1st byte (8bits) are move to left and extented as 16bits 
        // 2nd byte (next 8bits) are appended to the right  
        // | (OR bit operator) will add 0 and 1 into 16bits 
        uint16_t value = 
            (static_cast<uint16_t>(buffer_[offset_]) << 8)  | 
             static_cast<uint16_t>(buffer_[offset_+1]);

        offset_ += 2;
        return value;
    }

    uint32_t readUint32() {
        uint32_t value = 0;

        for(int i = 0; i < 4; i++) {
            value = (value << 8) | static_cast<uint32_t>(buffer_[offset_+i]);
        }

        offset_ += 4; 
        return value;
    }

    uint64_t readUint48() {
        uint64_t value = 0;

        for(int i = 0; i < 6; i++) {
            value = (value << 8) | static_cast<uint32_t>(buffer_[offset_+i]);
        }

        offset_ += 6; 
        return value;
    }

    uint64_t readUint64() {
        uint64_t value = 0;

        for(int i = 0; i < 8; i++) {
            value = (value << 8) | static_cast<uint32_t>(buffer_[offset_+i]);
        }

        offset_ += 8; 
        return value;
    }

    void readChars(char* dest, size_t len) {

        for(size_t i = 0; i < len; i++) {
            dest[i] = static_cast<char>(buffer_[offset_ + i]);
        }

        offset_ += len;
    }
 
private:
    std::array<uint8_t, 36> buffer_; // 36byte 받기 위함 
    size_t offset_ = 0;

};

AddOrderMessage parseAddOrder(const std::array<uint8_t, 36>& buffer) {
    ByteParser parser{buffer};

    AddOrderMessage msg{};

    char type;
    uint16_t stockLocate;
    uint16_t tracking;
    uint64_t timestamp; // 6byte 이지만, uint64_t 에 저장
    uint64_t orderReference;
    char side;
    uint32_t shares;
    char stock[8];
    uint32_t price;

    msg.type = static_cast<char>(parser.readUint8());
    msg.stockLocate = parser.readUint16();
    msg.tracking = parser.readUint16();
    msg.timestamp = parser.readUint48();
    msg.orderReference = parser.readUint64();
    msg.side = static_cast<char>(parser.readUint8());
    msg.shares = parser.readUint32();
    parser.readChars(msg.stock, 8);
    msg.price = parser.readUint32();

    return msg;
}

void printHexDump(const std::array<uint8_t, 36>& buffer) {
    for (size_t i = 0; i < buffer.size(); i++) {
        if (i % 16 == 0) {
            std::cout << std::setw(4) << std::setfill('0')
                      << std::hex << i << "  ";
        }

        std::cout << std::setw(2) << std::setfill('0')
                  << std::hex << static_cast<int>(buffer[i]) << " ";

        if (i % 16 == 15) {
            std::cout << "\n";
        }
    }

    std::cout << std::dec << "\n";
}


int main() {
    std::array<uint8_t, 36> buffer = {
        0x41,
        0x00, 0x01,
        0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x0a, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2a,
        0x42,
        0x00, 0x00, 0x00, 0x64,
        0x41, 0x41, 0x50, 0x4c, 0x20, 0x20, 0x20, 0x20,
        0x00, 0x01, 0x86, 0xa0
    };

    printHexDump(buffer);

    AddOrderMessage msg = parseAddOrder(buffer);

    std::cout << "Type: " << msg.type << "\n";
    std::cout << "Stock Locate: " << msg.stockLocate << "\n";
    std::cout << "Tracking: " << msg.tracking << "\n";
    std::cout << "Timestamp: " << msg.timestamp << "\n";
    std::cout << "Order Ref: " << msg.orderReference << "\n";
    std::cout << "Side: " << msg.side << "\n";
    std::cout << "Shares: " << msg.shares << "\n";
    std::cout << "Stock: " << std::string(msg.stock, 8) << "\n";
    std::cout << "Price(raw): " << msg.price << "\n";
    std::cout << "Price($): " << msg.price / 10000.0 << "\n";

    return 0;
}