#define PIN_DATA_AVAILABLE 2
#define PIN_A 3
#define PIN_B 4
#define PIN_C 5
#define PIN_D 6

char value[] = {
  '1', //0000b
  '7', //0001b
  '4', //0010b
  '*', //0011b
  '3', //0100b
  '9', //0101b
  '6', //0110b
  '#', //0111b
  '2', //1000b
  '8', //1001b
  '5', //1010b
  '0', //1011b
  'A', //1100b
  'C', //1101b
  'B', //1110b
  'D', //1111b
};

void setup() {
  Serial.begin(9600);
  pinMode(PIN_DATA_AVAILABLE, INPUT);
  pinMode(PIN_A, INPUT);
  pinMode(PIN_B, INPUT);
  pinMode(PIN_C, INPUT);
  pinMode(PIN_D, INPUT);
}

void loop() {
  if (digitalRead(PIN_DATA_AVAILABLE)) {
    uint8_t data = digitalRead(PIN_A) << 3
      | digitalRead(PIN_B) << 2
      | digitalRead(PIN_C) << 1
      | digitalRead(PIN_D);
    Serial.print("Value: ");
    Serial.print(data, BIN);
    Serial.print(", Key: ");
    Serial.println(value[data]);
    delay(100);
  }
}
