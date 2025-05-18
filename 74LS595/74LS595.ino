// pin to send data for shift register
#define PIN_DATA_INPUT 8
// pin to enable output
#define PIN_RCLK 9
// pin to enable store the serial data
#define PIN_SRCLK 10


const bool use_shiftout = 0;
uint8_t byte_to_write = 0x0;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_DATA_INPUT, OUTPUT);
  pinMode(PIN_RCLK, OUTPUT);
  pinMode(PIN_SRCLK, OUTPUT);
}


// simplified version of shiftOut
void send_byte_steps(uint8_t value) {
  for (uint8_t i = 0; i < 8; i++) {
    // 0x80 is 10000000 so will only check for MSB
    // if MSB is HIGH, send send HIGH to serial pin input
    digitalWrite(PIN_DATA_INPUT, (value & 0x80) != 0);
    value <<= 1;

    // enable clock input to store in the serial
    digitalWrite(PIN_SRCLK, HIGH);

    // disable clock input
    digitalWrite(PIN_SRCLK, LOW);
  }
}

void send_byte(uint8_t value) {
  // need to turn output off
  digitalWrite(PIN_RCLK, LOW);

  // need to send the bytes to register one by one
  if (use_shiftout) {
    // this is the most simple approach
    shiftOut(PIN_DATA_INPUT, PIN_SRCLK, MSBFIRST, value);
  } else {
    send_byte_steps(value);
  }

  // need to turn the output on
  digitalWrite(PIN_RCLK, HIGH);
}

void loop() {
  // send byte
  Serial.print("Sending ");
  Serial.println(byte_to_write, HEX);
  send_byte(byte_to_write);

  if (byte_to_write == 0xFF) {
    // reset
    byte_to_write = 0x00;
  } else {
    byte_to_write++;
  }

  delay(500);
}
