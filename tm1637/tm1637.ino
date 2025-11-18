#include "ascii.h"

// commands
// 01000000, B7=0,B6=1
#define CMD_DATA 0x40
// 10000000, B7=1,B6=0
#define CMD_DISPLAY 0x80
// 11000000, B7=1,B6=1
#define CMD_ADDRESS_BASE 0xC0

// data commands
#define CMD_DATA_READ_KEY 0x2
#define CMD_DATA_ADDR_FIXED 0x4
#define CMD_DATA_MODE_TEST 0x8

#define CMD_DISPLAY_ON 0x8

// brighnestt
#define CMD_BRIGHTNESS_1 0x1
#define CMD_BRIGHTNESS_2 0x2
#define CMD_BRIGHTNESS_3 0x3
#define CMD_BRIGHTNESS_4 0x4
#define CMD_BRIGHTNESS_5 0x5
#define CMD_BRIGHTNESS_6 0x6
#define CMD_BRIGHTNESS_7 0x7

// settings of project
#define PIN_CLK 2
#define PIN_DIO 3

/* 
the 7 segments are 
      
    --A--
  F |    | B
    --G--
  E |    | C
    --D--

and order is
.GFEDCBA
the dot . is only in second segment
*/



void delayClock() {
  // the maximum speed is 500Khz or 2us on each cycle
  delayMicroseconds(2);
}

void dataTransferStart() {
  // according to doc, start must be
  // a transition of DIO from LOW to HIGH
  // when a CLK clock is HIGH
  digitalWrite(PIN_DIO, HIGH);
  digitalWrite(PIN_CLK, HIGH);
  delayClock();
  digitalWrite(PIN_DIO, LOW);
  delayClock();
  digitalWrite(PIN_CLK, LOW);
  delayClock();
}

void dataTransferEnd() {
  // according to doc, end must be
  // a transition of DIO from HIGH to LOW
  // when a CLK clock is HIGH
  digitalWrite(PIN_DIO, LOW);
  digitalWrite(PIN_CLK, HIGH);
  delayClock();
  digitalWrite(PIN_DIO, HIGH);
  digitalWrite(PIN_CLK, LOW);
  delayClock();
}

unsigned char writeByte(unsigned char data) {
  // clock is assumed to be low, but just in case
  digitalWrite(PIN_CLK, LOW);

  for (unsigned char i = 0; i < 8; i++) {
    digitalWrite(PIN_DIO, (data & 0x1) ? HIGH : LOW);
    data = data >> 1;

    // run a clock cycle
    delayClock();
    digitalWrite(PIN_CLK, HIGH);
    delayClock();
    digitalWrite(PIN_CLK, LOW);
  }

  // here the chip does an ACK for one cycle
  pinMode(PIN_DIO, INPUT);
  delayClock();
  unsigned char ack = digitalRead(PIN_DIO);

  // finish clock cycle
  digitalWrite(PIN_CLK, HIGH);
  delayClock();
  digitalWrite(PIN_CLK, LOW);
  pinMode(PIN_DIO, OUTPUT);

  return ack;
}

void setBrightness(unsigned char brightness) {
  // display settings
  dataTransferStart();
  writeByte(CMD_DISPLAY | CMD_DISPLAY_ON | (brightness & 0x7));
  dataTransferEnd();
}

void writeData(unsigned char data, unsigned char address_index) {
  dataTransferStart();
  // tell IC want to write data to display
  // fixed address
  writeByte(CMD_DATA | CMD_DATA_ADDR_FIXED);
  dataTransferEnd();

  // set address
  dataTransferStart();
  writeByte(CMD_ADDRESS_BASE + address_index);
  // write data
  writeByte(data);
  dataTransferEnd();
}

void writeDataArray(unsigned char data[], unsigned char length) {
  length = min(6, length);
  dataTransferStart();
  // tell IC want to write data to display
  // incremental address
  writeByte(CMD_DATA);
  dataTransferEnd();

  // set base address
  dataTransferStart();
  writeByte(CMD_ADDRESS_BASE);

  // send all data
  for (unsigned int i = 0; i < length; i++) {
    writeByte(data[i]);
  }
  dataTransferEnd();
}

void setup() {
  // set output pins
  pinMode(PIN_CLK, OUTPUT);
  pinMode(PIN_DIO, OUTPUT);
}

/* helper function */
void showMarquee(unsigned char data[], unsigned char length, unsigned int delay_time) {
  unsigned char current_data[] = {' ', ' ', ' ', ' '};
  unsigned int offset = 0;
  while(offset + 4 < length) {
    // make a copy from text data into display
    // "unroll" to make it simple
    current_data[0] = GET_ASCII_CODE(data[offset]);
    current_data[1] = GET_ASCII_CODE(data[offset+1]);
    current_data[2] = GET_ASCII_CODE(data[offset+2]);
    current_data[3] = GET_ASCII_CODE(data[offset+3]);
    writeDataArray(current_data, 4);
    delay(delay_time);
    offset++;
  }
}

void exampleMarquee() {
  unsigned char marquee[] = {"    0123456789 hello world 9876543210    "};
  unsigned char marquee_length = sizeof(marquee) / sizeof(marquee[0]);

  showMarquee(marquee, marquee_length, 300);
}

void exampleCounter() {
  unsigned char data[] = {
    GET_ASCII_CODE('H'),
    GET_ASCII_CODE('i'),
    GET_ASCII_NUMBER(0),
    GET_ASCII_NUMBER(0),
  };

  writeDataArray(data, 4);
  for (unsigned char i=0; i<100; i++) {
    // convert number to string
    data[2] = GET_ASCII_NUMBER(i / 10);
    data[3] = GET_ASCII_NUMBER(i % 10);
    writeDataArray(data, 4);
    delay(100);
  }
}

void exampleClock() {
  unsigned char data[] = {'0', '0', '0', '0'};
  // simulate a clock
  unsigned char seconds = 0;
  unsigned char minutes = 0;
  unsigned char finished = 0;
  while(!finished) {
    // convert number to string
    data[0] = GET_ASCII_NUMBER(minutes / 10);
    data[1] = GET_ASCII_NUMBER(minutes % 10);

    // to show the two dots needs to set first bit
    data[1] |= 0x80;

    data[2] = GET_ASCII_NUMBER(seconds / 10);
    data[3] = GET_ASCII_NUMBER(seconds % 10);
    writeDataArray(data, 4);
    delay(50);

    // update time
    seconds++;
    if (seconds > 59) {
      minutes++;
      if (minutes > 59) {
        // reset
        finished = 1;
      }

      seconds = 0;
    }
  }
}

void loop() {
  setBrightness(CMD_BRIGHTNESS_2);

  exampleMarquee();
  exampleCounter();
  exampleClock();
}
