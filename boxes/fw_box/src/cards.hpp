#pragma once

#include <Arduino.h>

#include <MFRC522Constants.h>
#include <MFRC522v2.h>
#include <MFRC522Debug.h>

#include <cstdint>
#include <routing_game.hpp>
#include <logging.hpp>

struct SpiPinout {
    int miso;
    int mosi;
    int sck;
    int ss;
};

class SoftwareSPI {
    SpiPinout pinout;
public:
    SoftwareSPI(SpiPinout pinout): pinout(pinout) {}

    void begin() {
        pinMode(pinout.miso, INPUT);
        pinMode(pinout.mosi, OUTPUT);
        pinMode(pinout.sck, OUTPUT);
        pinMode(pinout.ss, OUTPUT);
        digitalWrite(pinout.ss, HIGH);
    }

    byte transfer(byte data) {
        byte result = 0;
        for (int i = 0; i < 8; i++) {
            digitalWrite(pinout.sck, LOW);

            digitalWrite(pinout.mosi, (data & (1 << (7 - i))) ? HIGH : LOW);
            digitalWrite(pinout.sck, HIGH);
            result |= (digitalRead(pinout.miso) == HIGH) << (7 - i);
        }
        return result;
    }

    void select() {
        digitalWrite(pinout.ss, LOW);
    }

    void deselect() {
        digitalWrite(pinout.ss, HIGH);
    }
};

class MFRC522DriverSPISoftware : public MFRC522Driver {
    SoftwareSPI spi;
public:
    MFRC522DriverSPISoftware(SpiPinout pinout): spi(pinout) {}

    bool init() override {
        spi.begin();
        return true;
    }

    void PCD_WriteRegister(const PCD_Register reg, const byte value) override {
        spi.select();
        spi.transfer(reg << 1);            // MSB == 0 is for writing. LSB is not used in address. Datasheet section 8.1.2.3.
        spi.transfer(value);
        spi.deselect();
    }

    void PCD_WriteRegister(const PCD_Register reg, const byte count, byte *const values) override {
        spi.select();
        spi.transfer(reg << 1);            // MSB == 0 is for writing. LSB is not used in address. Datasheet section 8.1.2.3.
        for(byte index = 0; index < count; index++) {
            spi.transfer(values[index]);
        }
        spi.deselect();
    }

    byte PCD_ReadRegister(const PCD_Register reg) override {
        byte value;
        spi.select();
        spi.transfer((byte)0x80 | (reg << 1));          // MSB == 1 is for reading. LSB is not used in address. Datasheet section 8.1.2.3.
        value = spi.transfer(0);          // Read the value back. Send 0 to stop reading.
        spi.deselect();
        return value;
    }

    void PCD_ReadRegister(const PCD_Register reg, const byte count, byte *const values, const byte rxAlign = 0) override {
        if(count == 0) {
            return;
        }
        byte address  = (byte)0x80 | (reg << 1);        // MSB == 1 is for reading. LSB is not used in address. Datasheet section 8.1.2.3.
        byte index    = 0;              // Index in values array.

        spi.select();
        spi.transfer(address);          // Tell MFRC522 which address we want to read
        if(rxAlign) {    // Only update bit positions rxAlign..7 in values[0]
            // Create bit mask for bit positions rxAlign..7
            byte mask  = (byte)(0xFF << rxAlign) & 0xFF;
            // Read value and tell that we want to read the same address again.
            byte value = spi.transfer(address);
            // Apply mask to both current value of values[0] and the new data in value.
            values[0] = (values[0] & ~mask) | (value & mask);
            index++;
        }
        //while (index < count) { // changed because count changed to const
        while(index < count-1) {
            values[index] = spi.transfer(address);  // Read value and tell that we want to read the same address again.
            index++;
        }
        values[index] = spi.transfer(0);      // Read the final byte. Send 0 to stop reading.
        spi.deselect();
    }
};

class GameCardInterface: public rg::CardCommInterface {
    static constexpr int RETRY_COUNT = 4;
    static constexpr const char *TAG = "GameCardInterface";
    MFRC522& mfrc522;

    // Card layout
    // page 4: 2 bytes team ID, 2 bytes card sequence number
    // page 5: 2 bytes round ID, 2 bytes visit count
    // page 6: 4 bytes metadata
    // page 7-100: router visits
    //
    // Single visit is stored in a single page
    // - 8 bits: router ID
    // - 12 bits: timestamp in seconds
    // - 4 bits: flags
    // - 8 bits: points

    bool _successfully_read = false;

    rg::CardLogicalId _logical_id;
    int _round_id;
    int _visit_count;
    std::bitset<32> _metadata;

    std::map<int, rg::PacketVisit> _visits;

    std::optional<rg::PacketVisit> _new_visit = std::nullopt;
    std::optional<std::bitset<32>> _new_metadata = std::nullopt;
public:
    GameCardInterface(MFRC522& mfrc522): mfrc522(mfrc522) {
        for (int i = 0; i != RETRY_COUNT; i++) {
            uint8_t buffer[18];
            uint8_t buffer_size = sizeof(buffer);

            auto status = mfrc522.MIFARE_Read(4, buffer, &buffer_size);
            if (status != MFRC522Constants::STATUS_OK) {
                // rg_log_e(TAG, "Failed to read team ID and card sequence number: %s", MFRC522Debug::GetStatusCodeName(status));
                continue;
            }
            _logical_id = rg::CardLogicalId{
                .team_id = rg::TeamId(buffer[1] << 8 | buffer[0]),
                .seq = rg::CardSeqNum(buffer[3] << 8 | buffer[2])
            };

            buffer_size = sizeof(buffer);
            status = mfrc522.MIFARE_Read(5, buffer, &buffer_size);
            if (status != MFRC522Constants::STATUS_OK) {
                // rg_log_e(TAG, "Failed to read round ID and visit count: %s", MFRC522Debug::GetStatusCodeName(status));
                continue;
            }
            _round_id = (buffer[1] << 8) | buffer[0];
            _visit_count = (buffer[3] << 8) | buffer[2];

            buffer_size = sizeof(buffer);
            status = mfrc522.MIFARE_Read(6, buffer, &buffer_size);
            if (status != MFRC522Constants::STATUS_OK) {
                // rg_log_e(TAG, "Failed to read metadata: %s", MFRC522Debug::GetStatusCodeName(status));
                continue;
            }
            _metadata = buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0];

            _successfully_read = true;
            return;
        }
        _successfully_read = false;
    }

    bool is_correctly_initialized() {
        return _successfully_read;
    }

    rg::CardPhysicalId get_physical_id() {
        rg::CardPhysicalId uid;
        for (int i = 0; i <7 ; i++) {
            uid[i] = mfrc522.uid.uidByte[i];
        }
        return uid;
    }

    rg::CardLogicalId get_id() override {
        return _logical_id;
    }

    rg::CardSeqNum get_seq() override {
        return _logical_id.seq;
    }

    int get_round_id() {
        return _round_id;
    }

    int visit_count() override {
        return _visit_count;
    }

    rg::PacketVisit get_visit(int idx) override {
        rg_log_i("GameCardInterface", "Reading visit %d", idx);
        if (idx < 0) {
            idx = _visit_count + idx;
        }
        rg_log_i("GameCardInterface", "Transformed visit %d/%d", idx, _visit_count);
        if (idx < 0 || idx >= _visit_count || idx >= 100) {
            return rg::PacketVisit{
                .where = rg::RouterId(-1),
                .time = -1
            };
        }

        auto it = _visits.find(idx);
        if (it != _visits.end()) {
            return it->second;
        }

        int page = 7 + idx;
        uint8_t buffer[18];
        for (int i = 0; i != RETRY_COUNT; i++) {
            uint8_t buffer_size = sizeof(buffer);
            auto status = mfrc522.MIFARE_Read(page, buffer, &buffer_size);
            if (status != MFRC522Constants::STATUS_OK) {
                rg_log_e(TAG, "Failed to read visit %d: %s", idx, MFRC522Debug::GetStatusCodeName(status));
                continue;
            }
            rg::PacketVisit visit = {
                .where = buffer[0],
                .time = (buffer[2] << 8 | buffer[1]) & 0xFFF,
                .flag1 = buffer[2] & (1 << 4),
                .flag2 = buffer[2] & (1 << 5),
                .flag3 = buffer[2] & (1 << 6),
                .flag4 = buffer[2] & (1 << 7),
                .points = buffer[3]
            };
            _visits[idx] = visit;
            return visit;
        }

        return rg::PacketVisit{
            .where = rg::RouterId(-1),
            .time = -1
        };
    }

    void mark_visit(rg::PacketVisit visit) override {
        _new_visit = visit;
    }

    std::bitset<32> get_metadata() override {
        return _metadata;
    }

    void set_metadata(std::bitset<32> metadata) override {
        _metadata = metadata;
        _new_metadata = metadata;
    }

    bool write_logic_id(rg::CardLogicalId logical_id) {
        uint8_t buffer[4] = {
            uint8_t(logical_id.team_id & 0xFF),
            uint8_t(logical_id.team_id >> 8),
            uint8_t(logical_id.seq & 0xFF),
            uint8_t(logical_id.seq >> 8)
        };
        for (int i = 0; i != RETRY_COUNT; i++) {
            auto status = mfrc522.MIFARE_Ultralight_Write(4, buffer, 4);
            if (status == MFRC522Constants::STATUS_OK) {
                return true;
            }
            rg_log_e(TAG, "Failed to write team ID and card sequence number: %s", MFRC522Debug::GetStatusCodeName(status));
        }
        return false;
    }

    bool reset_for_round(int round_id) {
        uint8_t round_buffer[4] = {
            uint8_t(round_id & 0xFF),
            uint8_t(round_id >> 8),
            0,
            0
        };
        uint8_t metadata_buffer[4] = {
            0,
            0,
            0,
            0
        };

        for (int i = 0; i != RETRY_COUNT; i++) {
            auto status = mfrc522.MIFARE_Ultralight_Write(5, round_buffer, 4);
            if (status != MFRC522Constants::STATUS_OK) {
                rg_log_e(TAG, "Failed to write round ID and visit count: %s", MFRC522Debug::GetStatusCodeName(status));
                continue;
            }
            status = mfrc522.MIFARE_Ultralight_Write(6, metadata_buffer, 4);
            if (status != MFRC522Constants::STATUS_OK) {
                rg_log_e(TAG, "Failed to write metadata: %s", MFRC522Debug::GetStatusCodeName(status));
                continue;
            }
            return true;
        }
        return false;
    }

    bool finish_transaction() {
        if (_new_visit.has_value()) {
            if (!_commit_visit()) {
                return false;
            }
        }
        if (_new_metadata.has_value()) {
            if (!_commit_metadata()) {
                return false;
            }
        }

        mfrc522.PICC_HaltA();
        return true;
    }

    bool _commit_visit() {
        rg::PacketVisit visit = _new_visit.value();
        int idx = _visit_count;
        if (idx >= 100) {
            rg_log_e(TAG, "Too many visits");
            return false;
        }
        int page = 7 + idx;

        uint8_t visit_buffer[4] = {
            uint8_t(visit.where),
            uint8_t(visit.time & 0xFF),
            uint8_t(
                visit.time >> 8 && 0xF |
                (int(visit.flag1) << 4) |
                (int(visit.flag2) << 5) |
                (int(visit.flag3) << 6) |
                (int(visit.flag4) << 7)),
            uint8_t(visit.points)
        };

        uint8_t visit_count_buffer[4] = {
            uint8_t(_round_id & 0xFF),
            uint8_t(_round_id >> 8),
            uint8_t((_visit_count + 1) & 0xFF),
            uint8_t((_visit_count + 1) >> 8)
        };

        for (int i = 0; i != RETRY_COUNT; i++) {
            auto status = mfrc522.MIFARE_Ultralight_Write(page, visit_buffer, 4);
            if (status != MFRC522Constants::STATUS_OK) {
                rg_log_e(TAG, "Failed to write visit %d: %s", idx, MFRC522Debug::GetStatusCodeName(status));
                continue;
            }
            status = mfrc522.MIFARE_Ultralight_Write(5, visit_count_buffer, 4);
            if (status != MFRC522Constants::STATUS_OK) {
                rg_log_e(TAG, "Failed to write visit count: %s", MFRC522Debug::GetStatusCodeName(status));
                continue;
            }
            return true;
        }
        return false;
    }

    bool _commit_metadata() {
        auto m = _new_metadata->to_ulong();
        uint8_t metadata_buffer[4] = {
            uint8_t(m & 0xFF),
            uint8_t(m >> 8),
            uint8_t(m >> 16),
            uint8_t(m >> 24)
        };

        for (int i = 0; i != RETRY_COUNT; i++) {
            auto status = mfrc522.MIFARE_Ultralight_Write(6, metadata_buffer, 4);
            if (status != MFRC522Constants::STATUS_OK) {
                rg_log_e(TAG, "Failed to write metadata: %s", MFRC522Debug::GetStatusCodeName(status));
                continue;
            }
            return true;
        }
        return false;
    }

    bool had_new_visit() {
        return _new_visit.has_value();
    }

    rg::PacketVisit get_new_visit() {
        return _new_visit.value();
    }
};


class CardReader {
    MFRC522DriverSPISoftware driver;
    MFRC522 mfrc522;
public:
    CardReader(SpiPinout pinout):
        driver(pinout),
        mfrc522(driver)
    {}

    void init() {
        mfrc522.PCD_Init();
    }

    bool has_card() {
        return mfrc522.PICC_ReadCardSerial();
    }

    bool has_new_card() {
        if (!mfrc522.PICC_IsNewCardPresent())
            return false;
        if (!mfrc522.PICC_ReadCardSerial())
            return false;
        return true;
    }

    auto card_uid() {
        return mfrc522.uid;
    }

    const char *card_uid_str() {
        static char buffer[7 * 2 + 1];
        sprintf(buffer, "%02X%02X%02X%02X%02X%02X%02X",
            mfrc522.uid.uidByte[0], mfrc522.uid.uidByte[1], mfrc522.uid.uidByte[2],
            mfrc522.uid.uidByte[3], mfrc522.uid.uidByte[4], mfrc522.uid.uidByte[5],
            mfrc522.uid.uidByte[6]);
        return buffer;
    }

    GameCardInterface game_card_interface() {
        return GameCardInterface(mfrc522);
    }

    void debug_connection() {
        MFRC522Debug::PCD_DumpVersionToSerial(mfrc522, Serial);
    }

    void debug() {
        // if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        //     return;
        // }

        // MFRC522Debug::PICC_DumpToSerial(mfrc522, Serial, &(mfrc522.uid));
        MFRC522Constants::StatusCode status;
        byte       byteCount;
        byte       buffer[18];
        byte       i;

        Serial.println(F("Page  0  1  2  3"));
        // Try the mpages of the original Ultralight. Ultralight C has more pages.
        auto time = millis();
        for(byte page = 0; page < 135; page += 4) { // Read returns data for 4 pages at a time.
            // Read pages
            byteCount = sizeof(buffer);
            status    = mfrc522.MIFARE_Read(page, buffer, &byteCount);
            if(status != MFRC522Constants::StatusCode::STATUS_OK) {
                Serial.print(F("MIFARE_Read() failed: "));
                // Serial.println(GetStatusCodeName(status));
                break;
            }
            // Dump data
            for(byte offset = 0; offset < 4; offset++) {
            i = page+offset;
            if(i < 10)
                Serial.print(F("   ")); // Pad with spaces
            else if (i < 100)
                Serial.print(F("  ")); // Pad with spaces
            else
                Serial.print(" ");
            Serial.print(i);
            Serial.print(F("  "));
            for(byte index = 0; index < 4; index++) {
                i = 4*offset+index;
                if(buffer[i] < 0x10)
                Serial.print(F(" 0"));
                else
                Serial.print(F(" "));
                Serial.print(buffer[i], HEX);
            }
            Serial.println();
            }
        }
        Serial.print("Took: ");
        Serial.println(millis() - time);
    }
};

