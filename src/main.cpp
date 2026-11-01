#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  pinMode(PB4, OUTPUT);
}

void loop() {
  digitalWrite(PB4, HIGH);  
  delay(1000);
  digitalWrite(PB4, LOW);
  delay(1000);
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}