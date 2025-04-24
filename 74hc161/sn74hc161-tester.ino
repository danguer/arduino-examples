#define PIN_CLK A0
#define PIN_CLR 13
#define PIN_ENP 12
#define PIN_LOAD 3

// carry
#define PIN_RCO 2

// inputs
#define A 4
#define B 5
#define C 6
#define D 7


// outputs
#define QA 8
#define QB 9
#define QC 10
#define QD 11

void clockCycle()
{
  digitalWrite(PIN_CLK, HIGH);
  delayMicroseconds(1);
  digitalWrite(PIN_CLK, LOW);
  delayMicroseconds(1);
}

void setup() {
  Serial.begin(9600);
  pinMode(QA, INPUT);
  pinMode(QB, INPUT);
  pinMode(QC, INPUT); 
  pinMode(QD, INPUT);
  pinMode(PIN_RCO, INPUT);

  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT); 
  pinMode(D, OUTPUT);
  pinMode(PIN_CLK, OUTPUT);
  pinMode(PIN_CLR, OUTPUT);
  pinMode(PIN_ENP, OUTPUT);
  pinMode(PIN_LOAD, OUTPUT);

  Serial.println("Reset circuit");

  // reset circuit
  digitalWrite(PIN_ENP, LOW);
  digitalWrite(PIN_CLR, LOW);
  clockCycle();
  digitalWrite(PIN_CLR, HIGH);

  // set input load as per datasheet example will be 12 = 0b1100
  digitalWrite(A, LOW);
  digitalWrite(B, LOW);
  digitalWrite(C, HIGH);
  digitalWrite(D, HIGH);

  // enable load
  digitalWrite(PIN_LOAD, LOW);

  // enable clock cycle to load data
  clockCycle();

  // disable load
  digitalWrite(PIN_LOAD, HIGH);

  // allow circuit to work
  digitalWrite(PIN_ENP, HIGH);
}

void loop() {
  // read outputs
  Serial.print("QA: ");
  Serial.print(digitalRead(QA));
  Serial.print(", QB: ");
  Serial.print(digitalRead(QB));
  Serial.print(", QC: ");
  Serial.print(digitalRead(QC));
  Serial.print(", QD: ");
  Serial.print(digitalRead(QD));
  Serial.print(", RCO: ");
  Serial.print(digitalRead(PIN_RCO));
  Serial.println("");

  clockCycle();
  delay(500);
}
