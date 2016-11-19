char c;
String recievedStr;

#include <Wire.h>
// CURRENTLY Wire Slave Sender
void setup() {
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  //Wire.onRequest(requestEvent); // register event
}

void loop() {
  delay(500);
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
//void requestEvent() {
//  Wire.write("hello "); // respond with message of 6 bytes
//  // as expected by master
//}

void receiveEvent(int howMany) {
  recievedStr = "";
  while (Wire.available()) { // slave may send less than requested
    c = Wire.read(); // receive a byte as character
    //Serial.println(c);
    //Wire.write(c);         // print the character
    recievedStr = String(recievedStr + c);
  }
  Serial.println(recievedStr);
  //Wire.write("hello "); // respond with message of 6 bytes
  // as expected by master
}
