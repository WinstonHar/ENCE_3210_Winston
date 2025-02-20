#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "images.h"
#include "I2Cdev.h"
#include "MPU6050.h"

// Button pins
#define LOCK_BUTTON  3
#define ONOFF_BUTTON 2
#define UNITS_BUTTON 4
#define CAL_BUTTON   5
#define LIGHT_BUTTON 6

// LED pins
#define LED 9
#define LED_DISPLAY 9

// TEMP PINS
#define TEMP_ANALOGUE 8   // Analog temperature sensor pin (or for oneWire communication)

// Timer settings
#define TIMER1_COMPARE_VALUE  62500 // 1 second for Timer1
#define TIMER2_COMPARE_VALUE  250   // ~4ms for Timer2

// Gyro
MPU6050 accelgyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;
#define OUTPUT_READABLE_ACCELGYRO

// OLED display settings (I2C)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Temperature sensor (OneWire)
OneWire oneWire(TEMP_ANALOGUE);
DallasTemperature sensors(&oneWire);
//global temp var store
float lockedTemp = 0.0;

// System states
enum State_e { LOCKED_C, LOCKED_F, UNLOCKED_C, UNLOCKED_F };
enum State_e gState = LOCKED_C;
volatile bool gFlagRoLock = true; // true = F, false = C
volatile bool gFlagPower = true;
volatile bool gFlagUnits = false;
volatile bool gFlagLight = false;

// --- Calibration mode globals ---
volatile bool calMode = false;      // Set to true when calibration mode is active
volatile uint8_t calCounter = 0;      // Counter incremented every second (via Timer1 ISR)

// --- Debouncing via Timer2 ---
// For LIGHT button we keep the existing debouncing
const uint8_t debounceThreshold = 13; // ~13*4ms ≈ 52ms debounce delay
volatile uint8_t lightStable = LOW;
volatile uint8_t lightCounter = 0;

// --- Interrupt Service Routines for external interrupts ---
void isr_lock_button() {
  unsigned long currentTime = micros();
  static unsigned long lastLockInterruptTime = 0;
  const unsigned long lockDebounceDelay = 50000; // 50ms

  if (currentTime - lastLockInterruptTime > lockDebounceDelay) {
    gFlagRoLock = !gFlagRoLock;
    lastLockInterruptTime = currentTime;
  }
}

void isr_onoff_button() {
  unsigned long currentTime = micros();
  static unsigned long lastOnOffInterruptTime = 0;
  const unsigned long lockDebounceDelay = 50000; // 50ms

  if (currentTime - lastOnOffInterruptTime > lockDebounceDelay) {
    gFlagPower = !gFlagPower;
    lastOnOffInterruptTime = currentTime;
  }
}

// --- Timer2 ISR for button processing ---
ISR(TIMER2_COMPA_vect) {
  // For UNITS button: long press detection (3 seconds required)
  // Each Timer2 interrupt is ~4ms, so 750 counts equals roughly 3000ms.
  static uint16_t unitsHoldCounter = 0;
  if (digitalRead(UNITS_BUTTON) == HIGH) {
    unitsHoldCounter++;
    if (unitsHoldCounter >= 750) {  // 3 seconds held
      gFlagUnits = !gFlagUnits;
      unitsHoldCounter = 0; // Reset to avoid repeated toggles
    }
  } else {
    unitsHoldCounter = 0;
  }

  // Debounce LIGHT_BUTTON (as before)
  uint8_t currentLight = digitalRead(LIGHT_BUTTON);
  if (currentLight != lightStable) {
    lightCounter++;
    if (lightCounter >= debounceThreshold) {
      lightStable = currentLight;
      if (lightStable == HIGH) {
        gFlagLight = !gFlagLight;
      }
      lightCounter = 0;
    }
  } else {
    lightCounter = 0;
  }
}

// --- Timer1 ISR ---
// Handles calibration mode timing (1 second per interrupt)
ISR(TIMER1_COMPA_vect) {
  if (calMode) {
    calCounter++;
    if (calCounter >= 3) {  // 3 seconds have passed
      calMode = false;
      calCounter = 0;
    }
  }
}

// --- Helper functions ---
void rotation() {
  if (ax >= 1) {
    display.setRotation(0); // right-side up
  } else {
    display.setRotation(2); // upside down
  }
}

// Modified calibration function: sets calMode flag when the calibrate button is pressed.
void cal() {
  if (!calMode && digitalRead(CAL_BUTTON) == HIGH) {
    calMode = true;
    calCounter = 0;  // reset counter when entering calibration mode
  }
}

void setup() {
  // Button pin setup
  pinMode(LOCK_BUTTON, INPUT);
  pinMode(ONOFF_BUTTON, INPUT);
  pinMode(UNITS_BUTTON, INPUT);
  pinMode(CAL_BUTTON, INPUT);
  pinMode(LIGHT_BUTTON, INPUT);

  // LED pin setup
  pinMode(LED, OUTPUT);

  // Pot setup
  pinMode(A0, INPUT);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for (;;) ; // Loop forever if OLED init fails
  }
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 8);
  display.display();
  delay(2000);
  display.clearDisplay();

  // Initialize temperature sensor
  sensors.begin();

  // Initialize gyro
  accelgyro.initialize();

  // Disable interrupts during timer configuration
  noInterrupts();

  // --- Timer1 setup (unchanged except for calibration mode support) ---
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = TIMER1_COMPARE_VALUE;
  TCCR1B |= (1 << WGM12); // CTC mode
  TCCR1B |= (1 << CS12);  // Prescaler 256
  TIMSK1 |= (1 << OCIE1A); // Enable Timer1 Compare Match A interrupt

  // --- Timer2 setup for button processing ---
  TCCR2A = 0;
  TCCR2B = 0;
  OCR2A = TIMER2_COMPARE_VALUE;
  TCCR2B |= (1 << WGM21); // CTC mode
  TCCR2B |= (1 << CS21) | (1 << CS22); // Prescaler 256 (timer ticks ≈4ms)
  TIMSK2 |= (1 << OCIE2A); // Enable Timer2 Compare Match A interrupt

  // Attach external interrupts for LOCK and ONOFF buttons
  attachInterrupt(digitalPinToInterrupt(LOCK_BUTTON), isr_lock_button, RISING);
  attachInterrupt(digitalPinToInterrupt(ONOFF_BUTTON), isr_onoff_button, RISING);

  // Re-enable interrupts
  interrupts();
}

void loop() {
  // First, check if the calibrate button was pressed
  cal();

  int potValue = analogRead(A0);
  // Compute an offset: when potValue is 512, offset is 0;
  // when potValue is 1023, offset is about +0.5; when 0, offset is about -0.5.
  float offset = ((float)potValue - 512.0) / 1023.0; 

  // If calibration mode is active, show the calibration image and skip normal processing.
  if (calMode) {
    display.clearDisplay();
    // Show the calibration image (assumed to be images[4])
    display.drawBitmap(0, 0, images[4], SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
    display.display();
    return;
  }

  display.clearDisplay();
  display.setCursor(0, 8);

  // Only update the temperature if the rLock (gFlagRoLock) is not active.
  float temp;
  if (!gFlagRoLock) {
    // Use the last stored temperature reading.
    temp = lockedTemp;
  } else {
    // Read a new temperature value and store it.
    sensors.requestTemperatures();
    temp = sensors.getTempCByIndex(0);
    lockedTemp = temp;
  }

  // Adjust temperature using the potentiometer offset.
  temp += offset;

  // LED power control.
  digitalWrite(LED, gFlagLight ? HIGH : LOW);

  // Determine state based on gFlagRoLock and gFlagUnits.
  int stateVal = (gFlagRoLock << 1) | gFlagUnits;

  if (gFlagPower) {
    display.ssd1306_command(SSD1306_DISPLAYON);

    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    rotation();
    switch (stateVal) {
      case LOCKED_C:
        display.drawBitmap(0, 0, images[0], SCREEN_WIDTH, SCREEN_HEIGHT, WHITE); // Locked image
        display.drawBitmap(0, 0, images[2], SCREEN_WIDTH, SCREEN_HEIGHT, WHITE); // Celsius symbol
        break;
      case LOCKED_F:
        display.drawBitmap(0, 0, images[0], SCREEN_WIDTH, SCREEN_HEIGHT, WHITE); // Locked image
        display.drawBitmap(0, 0, images[3], SCREEN_WIDTH, SCREEN_HEIGHT, WHITE); // Fahrenheit symbol
        temp = temp * 9.0 / 5.0 + 32;
        break;
      case UNLOCKED_C:
        display.drawBitmap(0, 0, images[1], SCREEN_WIDTH, SCREEN_HEIGHT, WHITE); // Unlocked image
        display.drawBitmap(0, 0, images[2], SCREEN_WIDTH, SCREEN_HEIGHT, WHITE); // Celsius symbol
        break;
      case UNLOCKED_F:
        display.drawBitmap(0, 0, images[1], SCREEN_WIDTH, SCREEN_HEIGHT, WHITE); // Unlocked image
        display.drawBitmap(0, 0, images[3], SCREEN_WIDTH, SCREEN_HEIGHT, WHITE); // Fahrenheit symbol
        temp = temp * 9.0 / 5.0 + 32;
        break;
    }
  } else {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
  }

  display.print(temp, 1); // Display the temperature (1 decimal place)
  display.display();
  delay(100);
}

