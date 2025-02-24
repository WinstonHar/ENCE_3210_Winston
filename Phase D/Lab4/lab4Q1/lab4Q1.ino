#define BITSIZE 14 // 14 bit SAR ADC

float Vref = 3.0; //reference voltage
float test_Voltages[] = {0.42, 0.83, 1.65, 2.752}; //test voltages

void SAR_ADC(float Vin){
  float thresh = Vref / 2;  // Initial threshold (mid-point)
  float step = Vref / 2;  // Step size for SAR iterations
  int bits[BITSIZE] = {0};  // Store SAR ADC result
  int digital_output = 0;  // Digital equivalent of Vin

  //conversion loop, same as example but with different variables this time
  for (int count = 0; count < BITSIZE; count++) {
      step /= 2;  // Halve step size each iteration
      
      if (Vin >= thresh) {
          bits[count] = 1;
          thresh += step;
      } else {
          bits[count] = 0;
          thresh -= step;
      }

      // Construct digital output from bits
      digital_output = (digital_output << 1) | bits[count];
  }

  // Print Results to Serial Monitor
  Serial.print("Analog Input: ");
  Serial.print(Vin, 3);
  Serial.print(" V -> Digital Output: ");
  Serial.print(digital_output);
  Serial.print(" (Binary: ");

  for (int i = 0; i < BITSIZE; i++) {
      Serial.print(bits[i]);
  }
  
  Serial.println(")");
}

void setup(){
  Serial.begin(9600);
  delay(1000);
  
  for (int i = 0; i<4;i++){
    SAR_ADC(test_Voltages[i]);
  }
}

void loop(){
  
}