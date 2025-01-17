#define RED 4    // R set to pin on mask
#define GREEN 5  // G set to pin on mask
#define BLUE 0   // B set to unsed pin as no blue led

#define BUTTON1 2

// An array describing the 6 steps in the desired sequence
// 1) R      = RED
// 2) RG     = RED + GREEN
// 3) RGB    = RED + GREEN + BLUE
// 4) GB     = GREEN + BLUE
// 5) B      = BLUE
// 6) RB     = RED + BLUE
uint8_t sequence[] = {
  (1 << RED),                 // R
  (1 << RED) | (1 << GREEN),         // RG
  (1 << RED) | (1 << GREEN) | (1 << BLUE),  // RGB
  (1 << GREEN) | (1 << BLUE),        // GB
  (1 << BLUE),                // B
  (1 << RED) | (1 << BLUE)           // RB
};

//volatile as used in ISR
volatile bool isSequenceRunning = false;
volatile unsigned long lastInterruptTime = 0;

void buttonISR(){
  unsigned long currentTime = millis();
  if ((currentTime - lastInterruptTime) > 50) {
    //trigger
    isSequenceRunning = !isSequenceRunning;

    //if not running then turn off led's
    if (!isSequenceRunning){
      digitalWrite(RED, LOW);
      digitalWrite(GREEN, LOW);
      digitalWrite(BLUE, LOW);
    }
  }

  lastInterruptTime = currentTime;
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("Initalize."));
  // --------------------------------------------------------------------
  // Set the LED pins
  // --------------------------------------------------------------------
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

  // Turn all LEDs off initially
  digitalWrite(RED, LOW);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);

  //button define initalize, hign when pressed, low when not pressed
  pinMode(BUTTON1, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(BUTTON1), buttonISR, FALLING);
}

void loop() {
  if(isSequenceRunning) {
    for (int i = 0; i < 6; i++){
      //if isr turns sequence off abort
      if(!isSequenceRunning){
        break;
      }
      //led initally off
      digitalWrite(RED, LOW);
      digitalWrite(GREEN, LOW);
      digitalWrite(BLUE, LOW);

      if (sequence[i] & (1 << RED)) {
        digitalWrite(RED, HIGH);
        Serial.print(F("Red "));
      }
      if (sequence[i] & (1 << GREEN)) {
        digitalWrite(GREEN, HIGH);
        Serial.print(F("Green "));
      }
      if (sequence[i] & (1 << BLUE)) {
        digitalWrite(BLUE, HIGH);
        Serial.print(F("Blue "));
      }
      Serial.println(F("--"));

      //wait 500ms without blocking ISR
      unsigned long startTime = millis();
      while (millis() - startTime < 500) {
        if(!isSequenceRunning){
          digitalWrite(RED, LOW);
          digitalWrite(GREEN, LOW);
          digitalWrite(BLUE, LOW);
          break;
        }
      }

      //if toggled while waiting, stop loop
      if(!isSequenceRunning){
        break;
      }
    }
  }
}


