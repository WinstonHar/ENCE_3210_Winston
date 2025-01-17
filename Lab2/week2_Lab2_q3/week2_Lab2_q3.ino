#define REDLED 4 //R set to pin on mask
#define GREENLED 5 //G set to pin on mask
#define BLUELED 0 //B set to null pin as no blue led

#define BUTTON_SELECT 2
#define BUTTON_APPLY 3

//Part 1, three arrays with two elements
int arr1[10] = { 1, 2, 3, 4, 5,  6,  7,  8,  9, 10 };
int arr2[10] = {10, 9, 8, 7, 6,  5,  4,  3,  2,  1 };
int arr3[10]; // Will store results here

// Operation mode, 1 = Add, 2 = subtract, 3 = multiply
volatile int gOpMode = 1;

volatile unsigned long gLastInterruptTimeSelect = 0;
volatile unsigned long gLastInterruptTimeApply = 0;

//flag for debug
volatile bool gOpFinished = false;

void selectISR() {
  unsigned long currentTime = millis();
  if (currentTime - gLastInterruptTimeSelect > 100) {
    gOpMode++;
    //if too far than rotate back on the op mode (1 2 3 1 etc)
    if (gOpMode > 3){
      gOpMode = 1;
    }
  }
  //update select interrupt time
  gLastInterruptTimeSelect = currentTime;
}

void applyISR() {
  unsigned long currentTime = millis();
  //debounce delay
  if (currentTime - gLastInterruptTimeApply > 100){

    //computer arr3
    switch (gOpMode){
      case 1:  // Addition
        arr3[0] = arr1[0] + arr2[0];
        break;
      case 2:  // Subtraction
        arr3[0] = arr1[0] - arr2[0];
        break;
      case 3:  // Multiplication
        arr3[0] = arr1[0] * arr2[0];
        break;
    }
  
    //turn off led
    digitalWrite(REDLED, LOW);
    digitalWrite(GREENLED, LOW);
    digitalWrite(BLUELED, LOW);

    //compute remaining arr
    for (int i = 1; i < 10; i++) {
      switch (gOpMode) {
        case 1: arr3[i] = arr1[i] + arr2[i]; break;
        case 2: arr3[i] = arr1[i] - arr2[i]; break;
        case 3: arr3[i] = arr1[i] * arr2[i]; break;
      }
    }
    
    //turn on all led
    digitalWrite(REDLED, HIGH);
    digitalWrite(GREENLED, HIGH);
    digitalWrite(BLUELED, HIGH);

    //flag for debug
    gOpFinished = true;
  }

  gLastInterruptTimeApply = currentTime;
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("Calculator initalize"));

  //Set pinmode led
  pinMode(REDLED, OUTPUT);
  pinMode(GREENLED, OUTPUT);
  pinMode(BLUELED, OUTPUT);

  //Start with red lit
  digitalWrite(REDLED, HIGH);
  digitalWrite(GREENLED, LOW);
  digitalWrite(BLUELED, LOW);

  //buttonPINMODE
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(BUTTON_APPLY, INPUT_PULLUP);

  //button ISR
  attachInterrupt(digitalPinToInterrupt(BUTTON_SELECT), selectISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BUTTON_APPLY), applyISR, CHANGE);
}

void loop() {
  //only handle ISR
  //when selectbutton pressed, change op mode
  //    1 => addition, turn on RED, off others
  //    2 => subtraction, turn on GREEN, off others
  //    3 => multiplication, turn on BLUE, off others

  static int lastOperationMode = 1; //only changes in loop when there is a change in select
  if (gOpMode != lastOperationMode) {
    //initalize led's off
    digitalWrite(REDLED, LOW);
    digitalWrite(GREENLED, LOW);
    digitalWrite(BLUELED, LOW);

    //can use an if else or a switch case like in java
    //switch to proper led per case and also output to terminal (as not blue led)
    switch (gOpMode) {
      case 1: //add
        digitalWrite(REDLED, HIGH);
        Serial.println(F("RED --add"));
        break;
      case 2: //subtract
        digitalWrite(GREENLED, HIGH);
        Serial.println(F("GREEN --subtract"));
        break;
      case 3: //mult
        digitalWrite(BLUELED, HIGH);
        Serial.println(F("BLUE --multiply"));
    }

    lastOperationMode = gOpMode;
  }

  //output to serial for debug
  if (gOpFinished) {
    gOpFinished = false;  // Clear the flag so we print only once

    Serial.println(F("Operation done. Results in arr3:"));
    for (int i = 0; i < 10; i++) {
      Serial.print(F("arr3["));
      Serial.print(i);
      Serial.print(F("] = "));
      Serial.println(arr3[i]);
    }
    Serial.println(F("-------------------------------------------------"));
  }
}