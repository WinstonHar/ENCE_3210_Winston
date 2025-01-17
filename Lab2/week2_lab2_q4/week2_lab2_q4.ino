#define BUTTON1 2

//prefilled x and y
volatile int x[10] = { 5, 8, 12, 2,  2,  9,  3, 14, 10, 6 }; 
volatile int y[10] = { 0, 0,  0, 0,  0,  0,  0,  0,  0,  0 };

//simpled debounce, fix spam
volatile unsigned long gLastInterruptTime = 0;

void button1ISR() {
  unsigned long currentTime = millis();

  if ((currentTime - gLastInterruptTime) > 100){
    //compute y[0] w/ bounduary case
    y[0] = 2 * x[0];

    // for n = 1 to 9, apply y[n] = 2 * x[n] - x[n-1]
    for (int n = 1; n < 10; n++) {
      y[n] = 2 * x[n] - x[n-1];
    }
  }
  
  //update lastInterruptTime
  gLastInterruptTime = currentTime;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println(F("Initalize"));

  //configure button pin
  pinMode(BUTTON1, INPUT_PULLUP);

  //attach isr
  attachInterrupt(digitalPinToInterrupt(BUTTON1), button1ISR, CHANGE);
}

void loop() {
  // put your main code here, to run repeatedly:
  static unsigned long lastPrintTime = 0;
  unsigned long currentTime = millis();

  if (currentTime - lastPrintTime >= 1000) {
    lastPrintTime = currentTime;
    Serial.println(F("Current y[] values (updated when button1 is pressed) :"));

    for (int i = 0; i < 10; i++) {
      Serial.print(F("y["));
      Serial.print(i);
      Serial.print(F("] = "));
      Serial.println(y[i]);
    }

    Serial.println(F("--------------------------------"));
  }
}
