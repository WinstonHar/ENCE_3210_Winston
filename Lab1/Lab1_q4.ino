#define myPORTB (*(volatile uint8_t*) 0x25)
#define myDDRB (*(volatile uint8_t*) 0x24)

#define PORTB5 0b00010000 //red
#define PORTB4 0b00001000 //green
#define PORTB3 0b00000100 //blue

char letters[] = {'A','R','X','B','G','R','G','B','Q','R'};



void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  //get size of array
  int arraySize  = sizeof(letters) / sizeof(letters[0]);

  //get output mask
  myDDRB |= (PORTB5 | PORTB4 | PORTB3);

  //create count for serial out
  int countR = 0;
  int countG = 0;
  int countB = 0;

  //get count of letters in array
  for (int i = 0; i < arraySize; i++) {
    if (letters[i] == 'R') {
        countR++;
    } else if (letters[i] == 'G') {
        countG++;
    } else if (letters[i] == 'B') {
        countB++;
    }
  }

  //output to serial and led
  if (countR > 0) {
    Serial.print("Red: ");
    Serial.println(countR);
    myPORTB |= PORTB5;
  }
  if (countG > 0) {
    Serial.print("Green: ");
    Serial.println(countG);
    myPORTB |= PORTB4;
  }
  if (countB > 0) {
    Serial.print("Blue: ");
    Serial.println(countB);
    myPORTB |= PORTB3;
  }



}

void loop() {
  // put your main code here, to run repeatedly:

}
