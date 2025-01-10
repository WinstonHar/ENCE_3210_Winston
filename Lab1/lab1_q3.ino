#define sizeOfArr 50
int testArr[50] = {
  10, 5, 3, 99, 46, 7, 32, 99, 11, 15,
  22, 47, 88, 99, 65, 12, 34, 55, 100, 24,
  1, 16, 77, 88, 42, 56, 92, 13, 60, 99,
  150, 4, 3, 5, 82, 91, 45, 78, 55, 44,
  33, 92, 100, 28, 18, 99, 56, 72, 88, 96
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  int secondMax = findSecondMax(testArr, sizeOfArr);

  //puts result in memorylocation 50 spaces past location pointer of array
  *(testArr +50) = secondMax;

  Serial.print("Second Max = ");
  Serial.println(secondMax);
  Serial.println("Value stored at: ");
  Serial.println((uintptr_t)(testArr + 50), HEX);
}

void loop() {
  // put your main code here, to run repeatedly:

}

int findSecondMax(int* arr, int size){
  //default values
  int max1 = arr[0];
  int max2 = arr[1];

  //make sure max1 is actually larger than max2
  if (max2 > max1) {
    int temp = max1;
    max1 = max2;
    max2 = temp;
  }

  for (int i = 2; i < size; i++){
    //is the number you are traversing is largest then reset both
    if (arr[i] > max1) {
      // Update both max1 and max2
      max2 = max1;
      max1 = arr[i];
    }
    //if number you are traversing is larger than second max then reset only second max
    else if (arr[i] > max2 && arr[i] != max1) {
      // Update only max2
      max2 = arr[i];
    }
    //else ignore and iterate

  }
  return max1;
}