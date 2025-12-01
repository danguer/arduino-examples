#include <Arduino.h>
#include <Wire.h>

#define I2C_ADDRESS            0x70
#define CMD_SETUP              0x20
#define CMD_SETUP_ON           0x01
#define CMD_DISPLAY            0x80
#define CMD_DISPLAY_ON         0x01
#define CMD_DISPLAY_BLINK_2HZ  0x02
#define CMD_DISPLAY_BLINK_1HZ  0x04
#define CMD_DISPLAY_BLINK_HALF 0x06
#define CMD_DIM                0xE0

#define MATRIX_LENGTH 16
uint8_t bitmap_matrix[MATRIX_LENGTH] = {
   /* ROW7-0 */ /*ROW15-8, not used*/
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000
};

uint8_t bitmap_matrix_heart_on[MATRIX_LENGTH] = {
   /* ROW7-0 */ /*ROW15-8, not used*/
    0b01100110, 0b00000000,
    0b11111111, 0b00000000,
    0b11111111, 0b00000000,
    0b11111111, 0b00000000,
    0b01111110, 0b00000000,
    0b00111100, 0b00000000,
    0b00011000, 0b00000000,
    0b00000000, 0b00000000
};

uint8_t bitmap_matrix_heart_large[MATRIX_LENGTH] = {
   /* ROW7-0 */ /*ROW15-8, not used*/
    0b01100110, 0b00000000,
    0b10011001, 0b00000000,
    0b10000001, 0b00000000,
    0b10000001, 0b00000000,
    0b01000010, 0b00000000,
    0b00100100, 0b00000000,
    0b00011000, 0b00000000,
    0b00000000, 0b00000000
};


void setup()
{
    Wire.begin();
    Serial.begin(9600);
}

void writeCmd(uint8_t cmd)
{
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(cmd);
    Wire.endTransmission();
}

void writeCmdDataPage(uint8_t address, uint8_t *data, uint8_t length)
{
    Wire.beginTransmission(I2C_ADDRESS);
    // address is only first 4 bits
    Wire.write(address & 0x0F);
    for (uint8_t i = 0; i < length; i++) {
        Wire.write(data[i]);
    }
    Wire.endTransmission();
}

void writeCmdData(uint8_t address, uint8_t data)
{
    writeCmdDataPage(address, &data, 1);
}

void loop()
{
    Serial.println("Setup on");
    writeCmd(CMD_SETUP | CMD_SETUP_ON);

    Serial.println("Display on without blink");
    writeCmd(CMD_DISPLAY | CMD_DISPLAY_ON);

    // send patterns
    uint8_t dots_filled = 0;
    while(true) {
        uint8_t max_row = dots_filled / 8;
        for (int idx=0; idx<8; idx++) {
            uint8_t pattern = 0;
            if (idx < max_row) {
                // full row
                pattern = 0xFF;
            } else if (idx == max_row) {
                // partial row
                uint8_t bits_filled = dots_filled % 8;
                pattern = (1 << bits_filled) | ((1 << bits_filled)-1);
            }

            bitmap_matrix[idx * 2] = pattern;
        }

        writeCmd(CMD_DIM | (max_row*2)); // set depending on amount of columns
        Serial.print("Sending pattern to matrix: ");
        Serial.println(dots_filled);
        writeCmdDataPage(0x00, bitmap_matrix, MATRIX_LENGTH);
        delay(80);

        // next iteration
        dots_filled++;
        if (dots_filled > 64) {
            Serial.println("Sending heart");
            for (int times=0; times<10; times++) {
                writeCmd(CMD_DIM | 0x0F); // set brightness to max
                writeCmdDataPage(0x00, bitmap_matrix_heart_on, MATRIX_LENGTH);
                delay(150);
                writeCmd(CMD_DIM | 0x00); // set brightness yo minimal
                writeCmdDataPage(0x00, bitmap_matrix_heart_large, MATRIX_LENGTH);
                delay(150);
            }

            dots_filled = 0;
            max_row = 0;
            // reset matrix
            for (int i=0; i<MATRIX_LENGTH; i++) {
                bitmap_matrix[0] = 0;
            }
        }
    }
}