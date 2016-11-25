String Data;
int i;
int c;

#include <Wire.h>
// CURRENTLY Wire Master Reader
void setup() {
  Wire.begin(3);        // join i2c bus (address optional for master)
  Serial1.begin(9600);  // start serial for output
  c = 0;
}

void loop() {
//  Wire.requestFrom(8, 6);    // request 6 bytes from slave device #8
//
//  while (Wire.available()) { // slave may send less than requested
//    char c = Wire.read(); // receive a byte as character
//    Serial.print(c);         // print the character
//  }
//
//  delay(500);
  
  delay(500);
  Data = String(c) + ", 0000, 1111, 2222, 3333, 4444";
  
  Wire.beginTransmission(8);
    for(i=0; i < Data.length(); i++){
      Wire.write(Data.charAt(i));
    }
  Wire.endTransmission();
  
  c = c + 1;
}
