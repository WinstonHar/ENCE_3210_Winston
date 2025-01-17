#define BUTTON1 2
#define BUTTON2 3

#define GREENLED 4
#define REDLED 5
#define BLUELED 0

volatile int button1count = 0;
volatile int button2count = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);

  pinMode(REDLED, OUTPUT);
  pinMode(BLUELED, OUTPUT);
  pinMode(GREENLED, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON1), button1ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BUTTON2), button2ISR, CHANGE);

  digitalWrite(REDLED, LOW);
  digitalWrite(GREENLED, LOW);
  digitalWrite(BLUELED, LOW);
}

//check if led lit order is correct
void loop() {
  // put your main code here, to run repeatedly:
  if (button1count > button2count){
    Serial.println(F("RED, -blue,green"));
    digitalWrite(REDLED, HIGH);
    digitalWrite(BLUELED, LOW);
    digitalWrite(GREENLED, LOW);
  } else if(button1count == button2count) {
    Serial.println(F("BLUE, -red,green"));
    digitalWrite(REDLED, LOW);
    digitalWrite(BLUELED, HIGH);
    digitalWrite(GREENLED, LOW);
  } else {
    Serial.println(F("GREEN, =red,blue"));
    digitalWrite(REDLED, LOW);
    digitalWrite(BLUELED, LOW);
    digitalWrite(GREENLED, HIGH);
  }
}

void button1ISR() {
  button1count++;
}

void button2ISR() {
  button2count++;
}