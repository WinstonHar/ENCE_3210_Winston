#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Button pins
#define ONOFF_BUTTON 2  // Power button
#define SENS_BUTTON  3  // Sensitivity adjustment button

// Fan pin (PWM output)
#define FAN 9 // Simulated fan using LED (PWM control)

// Temperature sensor pin
#define TEMP_ANALOGUE A0  // Analog temperature sensor (e.g., potentiometer)

// Sensitivity Levels
#define SENS_LOW  1
#define SENS_MED  2
#define SENS_HIGH 3

// Timer settings
#define TIMER1_COMPARE_VALUE  64000  // Sets Timer1 for 250Hz PWM
#define TIMER2_COMPARE_VALUE  250    // ~4ms timer for debouncing

// Global Variables
volatile int fanSpeed = 0;   // PWM duty cycle (0-255)
volatile bool fanOn = false; // Fan ON/OFF state
volatile int sensitivity = SENS_MED; // Default sensitivity level
float temperature = 0.0;     // Average temperature

// --- Debouncing variables ---
volatile bool debounceOnOff = false;
volatile bool debounceSens = false;
volatile uint8_t onOffCounter = 0;
volatile uint8_t sensCounter = 0;
const uint8_t debounceThreshold = 13; // ~13 * 4ms = ~52ms debounce time

void setup() {
    Serial.begin(9600);

    // Initialize button pins
    pinMode(ONOFF_BUTTON, INPUT);
    pinMode(SENS_BUTTON, INPUT);
    pinMode(FAN, OUTPUT);

    // Setup Timer1 for 250Hz PWM
    setupPWM();

    // Setup Timer2 for button debouncing
    setupTimer2();

    // Attach external interrupts
    attachInterrupt(digitalPinToInterrupt(ONOFF_BUTTON), isr_onoff_button, RISING);
    attachInterrupt(digitalPinToInterrupt(SENS_BUTTON), isr_sens_button, RISING);

    Serial.println("Temperature-based Fan Controller Initialized.");
}

// ------------------- Timer1 PWM Setup (250Hz) -------------------
void setupPWM() {
    noInterrupts(); // Disable interrupts while configuring

    // Timer1 Setup for 250Hz PWM on pin 9
    TCCR1A = (1 << COM1A1) | (1 << WGM11); // Non-inverting mode, fast PWM mode
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // Prescaler 8, fast PWM mode
    ICR1 = TIMER1_COMPARE_VALUE; // Set PWM frequency to 250Hz
    OCR1A = 0; // Initially set to 0% duty cycle

    interrupts(); // Enable interrupts
}

// ------------------- Timer2 Setup for Debouncing -------------------
void setupTimer2() {
    noInterrupts();

    TCCR2A = 0;
    TCCR2B = 0;
    OCR2A = TIMER2_COMPARE_VALUE;
    TCCR2B |= (1 << WGM21); // CTC mode
    TCCR2B |= (1 << CS21) | (1 << CS22); // Prescaler 256 (≈4ms per tick)
    TIMSK2 |= (1 << OCIE2A); // Enable Timer2 Compare Match A interrupt

    interrupts();
}

// ------------------- Interrupt Service Routines -------------------
void isr_onoff_button() {
    debounceOnOff = true;
}

void isr_sens_button() {
    debounceSens = true;
}

// ------------------- Timer2 ISR for Debouncing -------------------
ISR(TIMER2_COMPA_vect) {
    // Debounce ON/OFF button
    if (debounceOnOff) {
        if (digitalRead(ONOFF_BUTTON) == HIGH) {
            onOffCounter++;
            if (onOffCounter >= debounceThreshold) {
                fanOn = !fanOn;
                Serial.print("Fan turned ");
                Serial.println(fanOn ? "ON" : "OFF");
                onOffCounter = 0;
                debounceOnOff = false;
            }
        } else {
            onOffCounter = 0;
            debounceOnOff = false;
        }
    }

    // Debounce Sensitivity button
    if (debounceSens) {
        if (digitalRead(SENS_BUTTON) == HIGH) {
            sensCounter++;
            if (sensCounter >= debounceThreshold) {
                if (sensitivity == SENS_HIGH) {
                    sensitivity = SENS_LOW; // Loop back
                } else {
                    sensitivity++;
                }
                Serial.print("Sensitivity set to ");
                Serial.println(sensitivity);
                sensCounter = 0;
                debounceSens = false;
            }
        } else {
            sensCounter = 0;
            debounceSens = false;
        }
    }
}

void loop() {
    // Read 100 samples over 5 seconds
    temperature = readTemperature();

    Serial.print("Avg Temp: ");
    Serial.print(temperature, 2);
    Serial.println(" C");

    // Control fan speed
    if (fanOn) {
        fanSpeed = calculateFanSpeed(temperature);
    } else {
        fanSpeed = 0; // Fan is off
    }

    // Set LED brightness (simulating fan speed)
    OCR1A = (fanSpeed * TIMER1_COMPARE_VALUE) / 255;

    Serial.print("Fan Speed (PWM): ");
    Serial.println(fanSpeed);

    delay(1000); // Allow time for readings
}

// ------------------- Temperature Sampling -------------------
float readTemperature() {
    float tempSum = 0;
    for (int i = 0; i < 100; i++) {
        tempSum += map(analogRead(TEMP_ANALOGUE), 0, 1023, 0, 100); // Simulated conversion
        delay(50); // Spread readings over 5 sec
    }
    return tempSum / 100.0; // Return average temperature
}

// ------------------- Fan Speed Calculation -------------------
int calculateFanSpeed(float temp) {
    int pwmValue = 0;

    switch (sensitivity) {
        case SENS_LOW:
            pwmValue = map(temp, 20, 50, 50, 200); // Lower range, slower speed
            break;
        case SENS_MED:
            pwmValue = map(temp, 20, 50, 80, 230); // Medium range
            break;
        case SENS_HIGH:
            pwmValue = map(temp, 20, 50, 120, 255); // More sensitive, full speed
            break;
    }

    return constrain(pwmValue, 0, 255); // Ensure PWM stays within 0-255
}
