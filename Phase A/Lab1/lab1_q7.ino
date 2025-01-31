#include <avr/io.h>
#include <stdint.h>

// Direct register addresses for an ATmega328P (e.g., Arduino Uno)
#define myPORTB  (*(volatile uint8_t*)0x25)  // PORTB register
#define myDDRB   (*(volatile uint8_t*)0x24)  // DDRB register

// Bitmasks for Red, Green, Blue LEDs on PORTB
#define RED   0b00010000  // bit 4
#define GREEN 0b00001000  // bit 3
#define BLUE  0b00000100  // bit 2

// Button pins (on Arduino digital pins, separate from PORTB)
const int BUTTON_SELECT  = 2;  // Press to cycle operation: +, -, *
const int BUTTON_EXECUTE = 3;  // Press to apply operation to arrays

int arr1[10] = { 1, 2, 3, 4, 5,  6,  7,  8,  9, 10 };
int arr2[10] = {10, 9, 8, 7, 6,  5,  4,  3,  2,  1 };
int arr3[10]; // Will store results here

// Operation counter: 1=Add, 2=Subtract, 3=Multiply
volatile int opMode = 1;

// Debounce variables for both buttons
bool lastSelectState  = HIGH;
bool lastExecuteState = HIGH;
unsigned long lastSelectDebounceTime  = 0;
unsigned long lastExecuteDebounceTime = 0;
const unsigned long debounceDelay = 50; // ms

void setup() {
  // Configure LED bits as output
  myDDRB |= (RED | GREEN | BLUE);
  // Turn LEDs off initially
  myPORTB &= ~(RED | GREEN | BLUE);

  // Set up the serial port (for debugging/verification)
  Serial.begin(9600);

  // Configure button pins as input with internal pullup
  pinMode(BUTTON_SELECT,  INPUT_PULLUP);
  pinMode(BUTTON_EXECUTE, INPUT_PULLUP);

  // Initialize arr3[] to zero
  for (int i = 0; i < 10; i++) {
    arr3[i] = 0;
  }

  Serial.println("Simple Calculator Ready.");
  Serial.println("Press SELECT to choose +, -, or *.");
  Serial.println("Press EXECUTE to perform the operation on arr1[] and arr2[].");
}

// -------------------------------------------------
// Helper function: sets the LED state based on opMode
//    opMode=1 => RED on
//    opMode=2 => GREEN on
//    opMode=3 => BLUE on
// -------------------------------------------------
void updateLEDsForOpMode() {
  // Turn off all first
  myPORTB &= ~(RED | GREEN | BLUE);

  switch (opMode) {
    case 1: // Addition
      myPORTB |= RED;
      break;
    case 2: // Subtraction
      myPORTB |= GREEN;
      break;
    case 3: // Multiplication
      myPORTB |= BLUE;
      break;
    default:
      // If somehow out of range, clamp to 1
      opMode = 1;
      myPORTB |= RED;
      break;
  }
}

// -------------------------------------------------
// Perform the operation for each element of arr1, arr2,
// storing the result in arr3. Called after the user
// presses the EXECUTE button.
// -------------------------------------------------
void doOperationOnArrays() {
  // Turn off all LEDs while working
  myPORTB &= ~(RED | GREEN | BLUE);

  const char* opName = (opMode == 1) ? "Addition" :
                       (opMode == 2) ? "Subtraction" : "Multiplication";
  Serial.print("Performing ");
  Serial.println(opName);

  for (int i = 0; i < 10; i++) {
    int a = arr1[i];
    int b = arr2[i];
    int result = 0;

    switch (opMode) {
      case 1: // Addition
        result = a + b;
        break;
      case 2: // Subtraction
        result = a - b;
        break;
      case 3: // Multiplication
        result = a * b;
        break;
    }

    arr3[i] = result;

    // Print each operation to Serial
    Serial.print("arr1[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.print(a);
    Serial.print(", arr2[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.print(b);
    Serial.print(" => arr3[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(result);

    delay(100); // small delay, just for demonstration
  }

  myPORTB |= (RED | GREEN | BLUE);

  Serial.println("All results saved. All LEDs are ON now!");
}

// -------------------------------------------------
// Main loop: handle button presses & run operations
// -------------------------------------------------
void loop() {
  // -------------------------------------------------
  // 1) Check the SELECT button (for operation mode)
  // -------------------------------------------------
  bool selectReading = digitalRead(BUTTON_SELECT);

  if (selectReading != lastSelectState) {
    lastSelectDebounceTime = millis();
  }
  if ((millis() - lastSelectDebounceTime) > debounceDelay) {
    // If stable state is LOW => button pressed
    if (selectReading == LOW && lastSelectState == HIGH) {
      // Increment opMode
      opMode++;
      if (opMode > 3) {
        opMode = 1; // Wrap around
      }
      // Update LEDs
      updateLEDsForOpMode();
      // Print to Serial
      switch (opMode) {
        case 1: Serial.println("Selected operation: ADD (+). Red LED on."); break;
        case 2: Serial.println("Selected operation: SUBTRACT (-). Green LED on."); break;
        case 3: Serial.println("Selected operation: MULTIPLY (*). Blue LED on."); break;
      }
    }
  }
  lastSelectState = selectReading;

  // -------------------------------------------------
  // 2) Check the EXECUTE button (to apply operation)
  // -------------------------------------------------
  bool executeReading = digitalRead(BUTTON_EXECUTE);

  if (executeReading != lastExecuteState) {
    lastExecuteDebounceTime = millis();
  }
  if ((millis() - lastExecuteDebounceTime) > debounceDelay) {
    // If stable state is LOW => button pressed
    if (executeReading == LOW && lastExecuteState == HIGH) {
      // Perform the chosen operation on arr1[] and arr2[]
      doOperationOnArrays();
    }
  }
  lastExecuteState = executeReading;
}
