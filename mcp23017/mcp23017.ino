#include <Wire.h>
#define PIN_INTA 2

// configure from 0x20 - 0x27
#define EXP_ADDRESS 0x20

// following addresses are for IOCON.BANK = 0 which is the default
#define EXP_IODIRA 0x00
#define EXP_IODIRB 0x01
#define EXP_IPOLA 0x02
#define EXP_IPOLB 0x03
#define EXP_GPINTENA 0x04
#define EXP_GPINTENB 0x05
#define EXP_INTCONA 0x08
#define EXP_INTCONB 0x09
#define EXP_IOCON 0x0A
#define EXP_GPPUA 0x0C
#define EXP_GPPUB 0x0D
#define EXP_INTCAPA 0x10
#define EXP_INTCAPB 0x11
#define EXP_GPIOA 0x12
#define EXP_GPIOB 0x13

uint8_t output_portb = 0xF;
volatile uint8_t porta_interrupt = 0;
unsigned long last_updated = 0;

void setup() {
  Wire.begin();
  Serial.begin(9600);

  // set initial flags
  // disable sequential access (0x20)
  // and INTPOL active high (0x02)
  writeRegister(EXP_IOCON, 0x22);

  // set portB as output
  writeRegister(EXP_IODIRB, 0x00);

  // enable pull up
  writeRegister(EXP_GPPUA, 0xFF);

  // invert polarity
  writeRegister(EXP_IPOLA, 0xFF);

  // set interrupt
  attachInterrupt(digitalPinToInterrupt(PIN_INTA), onInterruptA, CHANGE);

  // enable interrupt on port A
  writeRegister(EXP_GPINTENA, 0xFF);

  // reset interrupt flags just in case
  readRegister(EXP_GPIOA);
}

void onInterruptA() {
  porta_interrupt = 1;
}

void writeRegister(uint8_t address, uint8_t value) {
  Wire.beginTransmission(EXP_ADDRESS);
  Wire.write(address);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t readRegister(uint8_t address) {
  Wire.beginTransmission(EXP_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(EXP_ADDRESS, 1);
  // read data response
  while (!Wire.available());
  return Wire.read();
}

void loop() {
  unsigned long current = millis();
  if (porta_interrupt) {
    porta_interrupt = 0;
    // check flags and ACK interrupt
    uint8_t flags = readRegister(EXP_INTCAPA);

    Serial.print(millis());
    Serial.print(" InterruptA triggered, flags: ");
    Serial.println(flags, HEX);
  }

  if (current - last_updated >= 1000) {
    last_updated = current;
    writeRegister(EXP_GPIOB, output_portb);

    if (output_portb == 0x0) {
      // reset
      output_portb = 0xF;
    } else {
      output_portb--;
    }
  }
}
