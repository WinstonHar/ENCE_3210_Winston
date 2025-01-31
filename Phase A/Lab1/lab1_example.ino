#define LED_PIN 13

//defined macro directly to portb 5 register
#define PORTB5 0x20

//data direction register for port B
#define myDDRB (*(volatile uint8_t*) 0x24)

//data register for port B
#define myPORTB (*(volatile uint8_t*) 0x25)

//#define "a macro"

void setup() {
  // put your setup code here, to run once:
  //pinMode(LED_PIN, OUTPUT);
  myDDRB |= PORTB5;

}

void loop() {
  // put your main code here, to run repeatedly:
  //digitalWrite(LED_PIN, HIGH);
  myPORTB |= PORTB5;
  delay(200);
  //digitalWrite(LED_PIN, LOW);
  myPORTB &= ~PORTB5;
  delay(200);

}
