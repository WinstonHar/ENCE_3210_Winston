#define REDLED 5
#define GREENLED 4
#define BUTTON1 3

volatile uint16_t task1_counter = 0;
volatile uint8_t button_pressed = 0;

// Timer1 ISR for 1 Hz (via software counter)
ISR(TIMER1_COMPA_vect) {
  static uint16_t tick_counter = 0;
  tick_counter++;

  if (tick_counter >= 61) {  // Divide 61 Hz to 1 Hz
    tick_counter = 0;

    static bool led_state = false;

    // Toggle Green LED state
    led_state = !led_state;

    if (led_state) {
      digitalWrite(GREENLED, HIGH); // Turn LED ON
    } else {
      digitalWrite(GREENLED, LOW);  // Turn LED OFF
    }

    // Increment the task1 counter for use in the main task
    task1_counter++;
  }
}

// Timer1 ISR for 10 Hz
ISR(TIMER2_COMPA_vect) {
  // Read the button state and control the Red LED
  if (digitalRead(BUTTON1) == LOW) {  // Assuming LOW means button pressed
    digitalWrite(REDLED, LOW);       // Turn LED off
    button_pressed = 1;
  } else {
    digitalWrite(REDLED, HIGH);        // Turn LED on
    button_pressed = 0;
  }
}

void setup() {
  // Setup serial communication
  Serial.begin(9600);

  // Setup LEDs as output
  pinMode(GREENLED, OUTPUT);
  pinMode(REDLED, OUTPUT);

  // Setup button as input
  pinMode(BUTTON1, INPUT_PULLUP);
  
  //I have to use the timers in this order(Timer 2 in hardware set as "Timer 1" in software) to accomidate for the fact that prescaler cannot fit the 10hz signal
  // Setup Timer1 for ~61 Hz
  TCCR2A = (1 << WGM21);  // CTC mode
  TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20); // Prescaler 1024
  OCR2A = 255;            // Timer reset value
  TIMSK2 = (1 << OCIE2A); // Enable Timer1 Compare Match A interrupt

  // Setup Timer2 for 10 Hz
  TCCR1B = (1 << WGM12);  // CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10); // Prescaler 1024
  OCR1A = 1562;           // Compare value for 10 Hz
  TIMSK1 = (1 << OCIE1A); // Enable Timer2 Compare Match A interrupt

  // Enable global interrupts
  sei();
}

void loop() {
  // Print message every 3 seconds
  if (task1_counter >= 3) {
    task1_counter = 0;

    Serial.println(F("3 seconds have passed. Task1 counter reset."));
  }
}
