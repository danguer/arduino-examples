#include <Arduino.h>
#include <Wire.h>

// address is from 0x50 to 0x57 depending on A0, A1, A2 pins
#define I2C_ADDRESS 0x50

#define PAGE_SIZE 20

void setup()
{
    Wire.begin();
    Serial.begin(9600);
}

void writeAddress(uint16_t addr)
{
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write((addr >> 8) & 0xFF); // high byte
    Wire.write(addr & 0xFF);        // low byte
}

void writeByte(uint16_t addr, uint8_t data)
{
    writeAddress(addr);
    Wire.write(data);
    Wire.endTransmission();
}

void writePage(uint16_t addr, const uint8_t* data, int length)
{
    writeAddress(addr);
    for (int i = 0; i < length; i++) {
        Wire.write(data[i]);
    }
    Wire.endTransmission();
}

uint8_t readByte(uint16_t addr)
{
    writeAddress(addr);
    Wire.endTransmission();
    Wire.requestFrom(I2C_ADDRESS, 1);
    if (Wire.available()) {
        return Wire.read();
    }

    return 0;
}

void readPage(uint16_t addr, uint8_t* buffer, int length)
{
    writeAddress(addr);
    Wire.endTransmission();
    Wire.requestFrom(I2C_ADDRESS, length);
    int index = 0;
    while (index < length) {
        if (Wire.available()) {
            buffer[index++] = Wire.read();
        }
    }
}

void loop()
{
    Serial.println("Writing data");
    uint8_t data = 0xFF;
    for (uint16_t addr = 0; addr < PAGE_SIZE; addr++) {
        writeByte(addr, data);
        data--;
    }

    Serial.println("Reading back data, by address");
    for (uint16_t addr = 0; addr < PAGE_SIZE; addr++) {
        Serial.print("Addr: ");
        Serial.print(addr, HEX);
        Serial.print(" Data: ");
        Serial.print(readByte(addr), HEX);
        Serial.println();
    }

    Serial.println("Reading back data, by page");
    uint8_t buffer[PAGE_SIZE];
    readPage(0, buffer, PAGE_SIZE);
    for (uint16_t addr = 0; addr < PAGE_SIZE; addr++) {
        Serial.print("Addr: ");
        Serial.print(addr, HEX);
        Serial.print(" Data: ");
        Serial.print(buffer[addr], HEX);
        Serial.println();
    }

    // do nothing
    while(1);
}