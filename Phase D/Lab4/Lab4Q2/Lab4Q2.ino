#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//OLED Stuff
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Voltage threshold
#define THRESHOLD_VOLTAGE 2.5 //min for solar to power system

//simulation variables (in lieu of actual hardware)
float solarVoltage = 3.0;
float batteryVoltage = 3.5;
bool isUsingSolar = false; //track pwr srs

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

void loop(){
  //simulate voltage fluxuations
  solarVoltage = random(200,400) / 100.00; //from 2.0V to 4.0v
  batteryVoltage = random(250,380) / 100.00; //from 2.5 to 3.8 v

  display.clearDisplay();
  // Show simulated voltages
  display.setCursor(0, 0);
  display.print("Sol: ");
  display.print(solarVoltage, 2);
  display.print("V");

  display.print(" Bat: ");
  display.print(batteryVoltage, 2);
  display.print("V");

  //refresh display
  display.setCursor(0,10);

  //choose power source
  if (solarVoltage > THRESHOLD_VOLTAGE) {
    isUsingSolar = true;
    display.print("Power Source: SOLAR");
  } else {
    isUsingSolar = false;
    display.print("Power Source: BATTERY");
  }

  //charge battery if solar v is higher than battery v
  if (batteryVoltage < solarVoltage) {
    display.setCursor(0,20);
    display.print("Battery Charging...");
  }

  display.display();
  delay(2000);
}