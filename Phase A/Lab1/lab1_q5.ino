#include <avr/io.h>
#include <stdint.h>

#define myPORTB (*(volatile uint8_t*) 0x25)
#define myDDRB  (*(volatile uint8_t*) 0x24)

// Bitmasks for controlling bits in PORTB
#define PORTB5 0b00010000 // Red LED
#define PORTB4 0b00001000 // Green LED
#define PORTB3 0b00000100 // Blue LED (not used here, but defined)


char inputArray[] = {'A','X','B','G','R','Y','G','R','B','G','R','Z','B','G','R'};
char pattern[3] = {'B','G','R'};

void setup() {
  Serial.begin(9600);

  // Configure the Red and Green LED pins as outputs(and blue for fun).
  myDDRB |= (PORTB5 | PORTB4 | PORTB3);

  // Turn all LEDs OFF initially
  myPORTB &= ~(PORTB5 | PORTB4 | PORTB3);

  // Count how many times the pattern occurs in inputArray
  int patternCount = detectPattern(inputArray, sizeof(inputArray), pattern, 3);

  // Report to Serial
  if (patternCount > 0) {
    Serial.print("Pattern detected! Number of occurrences: ");
    Serial.println(patternCount);

    // Turn on Green LED
    myPORTB |= PORTB4;
    Serial.println("Green LED blink");
  } 
  else {
    Serial.println("Pattern NOT detected!");

    // Turn on Red LED
    myPORTB |= PORTB5;
    Serial.println("Red LED blink");
  }
}

void loop() {
  // Nothing else to do repeatedly in this example.
}

int detectPattern(const char* arr, int arrSize, const char* pat, int patSize) {
  int count = 0;

  for (int i = 0; i <= arrSize - patSize; i++) {
    // Check if a match is found at position i
    if (arr[i]     == pat[0] &&
        arr[i+1]   == pat[1] &&
        arr[i+2]   == pat[2]) 
    {
      count++;
    }
  }

  return count;
}

