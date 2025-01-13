#include <avr/io.h>
#include <stdint.h>

#define myPORTB (*(volatile uint8_t*)0x25)  // PORTB register
#define myDDRB (*(volatile uint8_t*)0x24)   // DDRB register

// Bitmasks for controlling bits in PORTB (must match your wiring)
#define RED 0b00010000    // R -> bit 4
#define GREEN 0b00001000  // G -> bit 3
#define BLUE 0b00000100   // B -> bit 2

const int BUTTON_PIN = 2;

// An array describing the 6 steps in the desired sequence
// 1) R      = RED
// 2) RG     = RED + GREEN
// 3) RGB    = RED + GREEN + BLUE
// 4) GB     = GREEN + BLUE
// 5) B      = BLUE
// 6) RB     = RED + BLUE
uint8_t sequence[] = {
  RED,                 // R
  RED | GREEN,         // RG
  RED | GREEN | BLUE,  // RGB
  GREEN | BLUE,        // GB
  BLUE,                // B
  RED | BLUE           // RB
};

bool isSequenceRunning = false;

// Used for simple debouncing in loop()
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;  // ms

void setup() {
  // --------------------------------------------------------------------
  // Set the LED pins as outputs by setting the corresponding bits in DDRB
  // --------------------------------------------------------------------
  myDDRB |= (RED | GREEN | BLUE);

  // Turn all LEDs off initially
  myPORTB &= ~(RED | GREEN | BLUE);

  // Initialize the serial port (optional for debugging)
  Serial.begin(9600);
  Serial.println("Initalize.");

  // Setup the button pin as input with internal pull-up
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // That means the button is HIGH when not pressed, and LOW when pressed.
}

void loop() {
  // --------------------------------------------------------------------
  // 1) Check the button with basic debouncing
  // --------------------------------------------------------------------
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    // The button state changed, so note the time
    lastDebounceTime = millis();
  }
  // If the current time - lastDebounceTime is > debounceDelay,
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the stable state is LOW => the button is pressed
    if (reading == LOW && lastButtonState == HIGH) {
      // Toggle the sequence on/off
      isSequenceRunning = !isSequenceRunning;

      if (!isSequenceRunning) {
        // Turn all LEDs off if we just stopped
        myPORTB &= ~(RED | GREEN | BLUE);
      }
      //  the next block of code below will handle running the sequence.)
    }
  }
  // Update lastButtonState for next iteration
  lastButtonState = reading;

  // --------------------------------------------------------------------
  // 2) If isSequenceRunning == true, cycle through the sequence
  // --------------------------------------------------------------------
  if (isSequenceRunning) {
    // Go through each of the 6 steps in the sequence
    for (int i = 0; i < 6; i++) {
      if (!isSequenceRunning) {
        break;
      }

      // Turn off all LED bits first
      myPORTB &= ~(RED | GREEN | BLUE);

      // Turn on the bits for this step
      myPORTB |= sequence[i];

      Serial.print("Step ");
      Serial.print(i + 1);
      Serial.print(": Turning on ");
      Serial.println(sequence[i]);

      delay(500);

      // During that delay, the user might have pressed the button again.

      if (digitalRead(BUTTON_PIN) == LOW) {
        // A press is detected => toggle isSequenceRunning
        isSequenceRunning = false;
        // Turn off all LEDs
        myPORTB &= ~(RED | GREEN | BLUE);
        // Simple debounce wait
        delay(200);
        break;
      }
    }
    // After completing the 6 steps, loop() will run again and do them again,
    // unless the button was pressed.
  }

  // If isSequenceRunning == false, do nothing here,
  // and all LEDs stay off or remain in the last known state.
}
