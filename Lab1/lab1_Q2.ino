#include <math.h>
#define ARRAY_SIZE 360


float result[ARRAY_SIZE] = {0};

void setup(){
  Serial.begin(9600);
  // put your main code here, to run repeatedly:
  for (int i = 0; i < ARRAY_SIZE; i++){
    //for num in array (0 to 360) get cosine
    result[i] = cos(i);

    //just print to serial
    Serial.print(i);
    Serial.print(":");
    Serial.println(result[i]);
  }
}

void loop(){
  //empty
}