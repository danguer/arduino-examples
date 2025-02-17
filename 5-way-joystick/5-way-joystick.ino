#define PIN_RST 6
#define PIN_SET 7

void setup() {
  Serial.begin(9600);

  // set pins 8-12 as input
  DDRB &= ~0x1F;

  // set input pull-ups
  PORTB = 0x1F;

  // set specific pins for RST and SET
  pinMode(PIN_RST, INPUT_PULLUP);
  pinMode(PIN_SET, INPUT_PULLUP);
}

void loop() {
  // since inputs are pulled-up
  // and the com is GND it needs to 
  // inverse to get which one 
  // was set to GND
  uint8_t current_status = ~PINB;
  bool printed = 0;
  
  // check individual pins
  if (!digitalRead(PIN_RST)) { 
    Serial.print("RST ");
    printed = 1;
  }

  if (!digitalRead(PIN_SET)) { 
    Serial.print("SET ");
    printed = 1;
  }

  // check joystick positions
  if (current_status & 0x1) {
    Serial.print("CENTER ");
    printed = 1;
  }

  if (current_status & 0x2) {
    Serial.print("RIGHT ");
    printed = 1;
  }

  if (current_status & 0x4) {
    Serial.print("LEFT ");
    printed = 1;
  }

  if (current_status & 0x8) {
    Serial.print("DOWN ");
    printed = 1;
  }

  if (current_status & 0x10) {
    Serial.print("UP ");
    printed = 1;
  }

  if (printed) {
    Serial.print("\n");
  }

  delay(100);
}
