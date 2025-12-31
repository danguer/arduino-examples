#include <Arduino.h>
#define PIN_DIN  2
#define PIN_LOAD 3
#define PIN_CLK  4

#define REGISTER_DIGIT0       0x1
#define REGISTER_DIGIT1       0x2
#define REGISTER_DIGIT2       0x3
#define REGISTER_DIGIT3       0x4
#define REGISTER_DIGIT4       0x5
#define REGISTER_DIGIT5       0x6
#define REGISTER_DIGIT6       0x7
#define REGISTER_DIGIT7       0x8
#define REGISTER_DECODE_MODE  0x9
#define REGISTER_INTENSITY    0xA
#define REGISTER_SCAN_LIMIT   0xB
#define REGISTER_SHUTDOWN     0xC
#define REGISTER_DISPLAY_TEST 0xF

uint16_t buildCmd(uint8_t registerAddress, uint8_t data)
{
    return ((uint16_t)registerAddress << 8) | data;
}

void sendCmd(uint16_t cmd)
{
    // shift out 16 bits, MSB first
    for (int i = 0; i < 16; i++) {
        digitalWrite(PIN_CLK, LOW);
        // check if MSB bit is on
        if (cmd & 0x8000) {
            digitalWrite(PIN_DIN, HIGH);
        } else {
            digitalWrite(PIN_DIN, LOW);
        }
        digitalWrite(PIN_CLK, HIGH);
        cmd <<= 1;
    }

    // disable clock
    digitalWrite(PIN_CLK, LOW);
    // just in case
    digitalWrite(PIN_DIN, LOW);

    // latch the data
    digitalWrite(PIN_LOAD, LOW);
    digitalWrite(PIN_LOAD, HIGH);
    digitalWrite(PIN_LOAD, LOW);
}

void cmdShutdown(bool enable)
{
    uint8_t data = enable ? 0x00 : 0x01;
    uint16_t cmd = buildCmd(REGISTER_SHUTDOWN, data);
    sendCmd(cmd);
}

void cmdSetIntensity(uint8_t intensity)
{
    if (intensity > 15) {
        intensity = 15;
    }
    uint16_t cmd = buildCmd(REGISTER_INTENSITY, intensity);
    sendCmd(cmd);
}

void cmdSetScanLimit(uint8_t scanLimit)
{
    if (scanLimit > 7) {
        scanLimit = 7;
    }
    uint16_t cmd = buildCmd(REGISTER_SCAN_LIMIT, scanLimit);
    sendCmd(cmd);
}

void cmdSetDecodeMode(uint8_t decodeMode)
{
    uint16_t cmd = buildCmd(REGISTER_DECODE_MODE, decodeMode);
    sendCmd(cmd);
}

void cmdSetDisplayTest(bool enable)
{
    uint8_t data = enable ? 0x01 : 0x00;
    uint16_t cmd = buildCmd(REGISTER_DISPLAY_TEST, data);
    sendCmd(cmd);
}

void resetSegments()
{
    for (uint8_t i = 0; i < 8; i++) {
        // set all segments to blank
        uint16_t cmd = buildCmd(REGISTER_DIGIT0 + i, 0x0);
        sendCmd(cmd);
    }
}

void setup()
{
    pinMode(PIN_DIN, OUTPUT);
    pinMode(PIN_CLK, OUTPUT);
    pinMode(PIN_LOAD, OUTPUT);

    digitalWrite(PIN_LOAD, LOW);
    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_DIN, LOW);
}

void loop()
{
    // Example usage
    cmdShutdown(false); // Wake up the display
    cmdSetIntensity(1);
    cmdSetDisplayTest(true); // test operation
    delay(2000);

    // Display a pattern on the 8 digits
    cmdSetDisplayTest(false); // Disable test mode
    cmdSetScanLimit(7); // Enable all 8 digits
    cmdSetIntensity(0); // lowest intensity
    cmdSetDecodeMode(0); // no decode mode

    resetSegments();
    for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            uint8_t pattern = 1 << j;
            uint16_t cmd = buildCmd(REGISTER_DIGIT0 + i, pattern); // show only one segment
            sendCmd(cmd);
            delay(200);
        }
    }

    resetSegments();
    // use bcd
    cmdSetDecodeMode(0xFF); // BCD mode for all digits
    for (uint8_t i = 0; i < 8; i++) {
        // note that 0xF or 15 means blank, so will set off that digit
        // after showing it
        for (uint8_t j = 0; j < 16; j++) {
            uint16_t cmd = buildCmd(REGISTER_DIGIT0 + i, j); // display digit
            sendCmd(cmd);
            delay(200);
        }
    }
}
