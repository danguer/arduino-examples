#define PIN_E A15
#define PIN_CS2 A14
#define PIN_RW A13
#define PIN_RS0 A11
#define PIN_RS1 A12
#define PIN_RESET A10

#define PIN_INTERRUPT 21

// some definitions to access easily to registers
#define REGISTER_PORTA 0
#define REGISTER_CTRLA 2
#define REGISTER_PORTB 3
#define REGISTER_CTRLB 4

// flags for control
#define FLAG_C1_INTERRUPT_ENABLE 0x1
#define FLAG_C1_TRANSITION_LOW_TO_HIGH 0x2
#define FLAG_DATA 0x4
#define FLAG_C2_INTERRUPT_ENABLE 0x8
#define FLAG_C2_TRANSITION_LOW_TO_HIGH 0x10
#define FLAG_C2_STROBE_E_RESTORE 0x28
#define FLAG_C2_OUTPUT_RESET 0x30
#define FLAG_C2_OUTPUT_SET 0x38

uint8_t led_value = 0xF;
uint8_t interrupt_control = 0;
unsigned long last_led_control = 0;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_E, OUTPUT);
  pinMode(PIN_CS2, OUTPUT);
  pinMode(PIN_RW, OUTPUT);
  pinMode(PIN_RS0, OUTPUT);
  pinMode(PIN_RS1, OUTPUT);
  pinMode(PIN_RESET, OUTPUT);

  // reset chip
  digitalWrite(PIN_RESET, LOW);
  delay(1);
  digitalWrite(PIN_RESET, HIGH);

  // disable access to chip
  digitalWrite(PIN_E, LOW);
  digitalWrite(PIN_CS2, HIGH);
  digitalWrite(PIN_RW, LOW);
  digitalWrite(PIN_RS0, LOW);
  digitalWrite(PIN_RS1, LOW);

  // interrupt needs to be pullup as IRQ are low when activated
  pinMode(PIN_INTERRUPT, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT), onInterrupt, FALLING);

  // by default the direction register will be selected in both ports

  // set portb as output
  writeRegister(REGISTER_PORTB, 0xFF);

  // set flags to read data from Port B
  writeRegister(REGISTER_CTRLB, FLAG_DATA);

  // all ports by default are read only, so no need to configure direction of 
  // Port A, but is needed to set control to read peripheral register
  // and set flags in this case to allow interrupts
  writeRegister(REGISTER_CTRLA, FLAG_DATA | FLAG_C1_INTERRUPT_ENABLE);
  Serial.print("Control set: ");
  Serial.println(readRegister(REGISTER_CTRLA), HEX);
}

void onInterrupt() {
  // read the register and save for later
  interrupt_control = readRegister(REGISTER_CTRLA);
}

void _startCycle() {
  // enable chip select
  digitalWrite(PIN_CS2, LOW);
  delay(1);

  // enable chip
  digitalWrite(PIN_E, HIGH);
  delay(1);
}

void _endCycle() {
  digitalWrite(PIN_E, LOW);
  digitalWrite(PIN_CS2, HIGH);
  delay(1);
}

void _setRsAddress(uint8_t rs0, uint8_t rs1) {
  digitalWrite(PIN_RS0, rs0);
  digitalWrite(PIN_RS1, rs1);
}

void _selectRsAddress(uint8_t reg) {
  // check Table 1 with internal addressing
  if (reg == REGISTER_PORTA) {
    _setRsAddress(LOW, LOW);
  } else if (reg == REGISTER_PORTB) {
    _setRsAddress(LOW, HIGH);
  } else if (REGISTER_CTRLA) {
    _setRsAddress(HIGH, LOW);
  } else if (REGISTER_CTRLB) {
    _setRsAddress(HIGH, HIGH);
  }
}

void writeRegister(uint8_t reg, uint8_t value) {
  // set output for data port
  DDRA = 0xFF;
  
  // tell PIA to write data to
  digitalWrite(PIN_RW, LOW);

  // select proper address
  _selectRsAddress(reg);

  // put data into bus
  PORTA = value;
  _startCycle();
  _endCycle();
}

uint8_t readRegister(uint8_t reg) {
  // set input for data port
  DDRA = 0x00;
  
  // tell PIA to read data to
  digitalWrite(PIN_RW, HIGH);

  // select proper address
  _selectRsAddress(reg);
  // wait for chip to get data
  _startCycle();
  uint8_t data = PINA;
  _endCycle();

  return data;
}

void loop() {
  if (interrupt_control) {
    Serial.print("Control interrupt: ");
    Serial.println(interrupt_control, HEX);
    interrupt_control = 0;
  }

  uint8_t control = readRegister(REGISTER_CTRLA);
  // bit7(CA1) or bit6(CA2) can be enabled if there is an interrupt
  if ((control & 0xC0) != 0) {
    Serial.print("Interrupt in register: ");
    Serial.println(control, HEX);
  }

  // the port is a pull up so the default value read will be 0xFF
  uint8_t pressed = readRegister(REGISTER_PORTA);
  if (pressed != 0xFF) {
    Serial.print("Read port: ");
    Serial.println(pressed, HEX);
  }

  // check if write the value to Port B
  // note that output load is low so that is why
  // the leds are set to LOW to be ON
  // if needed can be added a buffer IC or similar
  unsigned long current = millis();
  if (current - last_led_control >= 1000) {
    writeRegister(REGISTER_PORTB, led_value);
    last_led_control = current;
    if (led_value == 0) {
      led_value = 0xF;
    } else {
      led_value--;
    }
  }
}
