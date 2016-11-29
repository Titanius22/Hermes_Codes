// Wire Slave Sender
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Sends data as an I2C/TWI slave device
// Refer to the "Wire Master Reader" example for use with this

// Created 29 March 2006

// This example code is in the public domain.

#include <Wire.h>

unsigned int lineCount = 0;
unsigned long latitude;
unsigned long longitude;
unsigned int altitude;
unsigned int temperature;
unsigned int pressure;
char endLine[3] = {'E', 'N', 'D'};
char CharsToSend[25];

void setup() {
  Wire.begin(4);                // join i2c bus with address #8
  Wire.onRequest(requestEvent); // register event
}

void loop() {
  delay(1);
  lineCount;
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  updateCharsToSend();
  Wire.write(CharsToSend, 25); // respond with message of 25 byte
  lineCount++;
}

void updateCharsToSend(){
  unsigned int intBuflineCount = 0;
  unsigned long longBuflatitude;
  unsigned long longBuflongitude;
  unsigned int intBufaltitude;
  unsigned int intBuftemperature;
  unsigned int intBufpressure;

  char charintBuflineCount[4];
  char charlongBuflatitude[8];
  char charlongBuflongitude[8];
  char charintBufaltitude[4];
  char charintBuftemperature[4];
  char charintBufpressure[4];

  //Line counter-------------------------------------------
  intBuflineCount = lineCount;
  charintBuflineCount[0] = &intBuflineCount;
  
  CharsToSend[0] = charintBuflineCount[1];
  CharsToSend[1] = charintBuflineCount[2];
  CharsToSend[2] = charintBuflineCount[3];


  //Latitude * 10^10 positive only---------------------------
  longBuflatitude = latitude * 10000000000;
  charlongBuflatitude[0] = &longBuflatitude;

  CharsToSend[3] = charlongBuflatitude[3];
  CharsToSend[4] = charlongBuflatitude[4];
  CharsToSend[5] = charlongBuflatitude[5];
  CharsToSend[6] = charlongBuflatitude[6];
  CharsToSend[7] = charlongBuflatitude[7];
  

  //Longitude * 10^10 positive only max of 109 degrees--------
  longBuflongitude = longitude * 10000000000;
  charlongBuflongitude[0] = &longBuflongitude;

  CharsToSend[8] = charlongBuflongitude[3];
  CharsToSend[9] = charlongBuflongitude[4];
  CharsToSend[10] = charlongBuflongitude[5];
  CharsToSend[11] = charlongBuflongitude[6];
  CharsToSend[12] = charlongBuflongitude[7];


  //Altitude * 100--------------------------------------------
  intBufaltitude = altitude * 100;
  charintBufaltitude[0] = &intBufaltitude;

  CharsToSend[13] = charintBufaltitude[1];
  CharsToSend[14] = charintBufaltitude[2];
  CharsToSend[15] = charintBufaltitude[3];


  //Temperature count------------------------------------------
  intBuftemperature = temperature;
  charintBuftemperature[0] = &intBuftemperature;

  CharsToSend[16] = charintBuftemperature[1];
  CharsToSend[17] = charintBuftemperature[2];
  CharsToSend[18] = charintBuftemperature[3];


  //Pressure count---------------------------------------------
  intBufpressure = pressure;
  charintBufpressure[0] = &intBufpressure;

  CharsToSend[19] = charintBufpressure[1];
  CharsToSend[20] = charintBufpressure[2];
  CharsToSend[21] = charintBufpressure[3];


  //End of line chars-------------------------------------------

  CharsToSend[22] = endLine[0];
  CharsToSend[23] = endLine[1];
  CharsToSend[24] = endLine[2];
}





















