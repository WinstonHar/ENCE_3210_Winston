#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Display Settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Voltage Thresholds
#define THRESHOLD_VOLTAGE 2.5  // Minimum voltage required for solar to power system
#define FULLY_CHARGED_VOLTAGE 3.8 // Battery is considered fully charged
#define CHARGING_THRESHOLD 3.5 // Battery needs to reach this level before switching to solar

// Simulation variables (in lieu of actual hardware)
float solarVoltage = 3.0;
float batteryVoltage = 3.5;
bool isUsingSolar = false;  // Tracks power source
bool isCharging = false;    // Tracks charging state

void setup() {
    Serial.begin(9600);

    // Initialize OLED Display
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println("OLED display failed to initialize.");
        while (1);
    }

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.clearDisplay();
    display.setCursor(0, 10);
    display.print("Solar Charge Ctrl");
    display.display();
    delay(2000); // Show startup message
}

void loop() {
    // Simulate voltage fluctuations
    solarVoltage = random(200, 400) / 100.00;   // From 2.0V to 4.0V
    batteryVoltage = random(300, 390) / 100.00; // From 3.0V to 3.9V

    display.clearDisplay();

    // Show simulated voltages
    display.setCursor(0, 0);
    display.print("Sol: ");
    display.print(solarVoltage, 2);
    display.print("V");

    display.print(" Bat: ");
    display.print(batteryVoltage, 2);
    display.print("V");

    display.setCursor(0, 10);

    // Power source decision logic
    if (solarVoltage > THRESHOLD_VOLTAGE) {
        if (batteryVoltage < CHARGING_THRESHOLD) {
            isCharging = true;
            display.print("Charging Battery...");
        } else if (batteryVoltage >= FULLY_CHARGED_VOLTAGE) {
            isCharging = false;
            isUsingSolar = true;
            display.print("Power: SOLAR");
        }
    } 
    
    display.setCursor(0, 20);
    if (solarVoltage <= THRESHOLD_VOLTAGE || isCharging) {
        isUsingSolar = false;
        display.print("Power: BATTERY");
    }

    display.display();
    delay(2000); // Update every 2 seconds
}
