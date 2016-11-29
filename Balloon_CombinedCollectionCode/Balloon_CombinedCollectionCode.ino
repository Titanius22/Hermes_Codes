/* 

Demonstration code for the Parallax PAM-7Q module, #28509
This code uses the default factory settings of the PAM-7Q module.

The GPS output is displayed in the Arduino Serial Terminal Window.
After uploading the sketch open this window to view the output. 
Make sure the baud rate is set to 9600.

Numeric output is shown as signed latitude and longitude degrees and
minutes. Values may be directly copied and pasted into the location bar
of Google Maps to visually show your location.

For best results use the PAM-7Q module outdoors, or near an open window.
Use indoors away from windows may result in inconsistent results.

This example code is for the Arduino Uno and direct compatible boards. 
It has not been tested, nor designed for, other Arduino boards, including
the Arduino Due.

Important: This version is intended for Arduino 1.0 or later IDE. It will
not compile in earlier versions. Be sure the following files are
present in the folder with this sketch:

TinyGPS.h
TinyGPS.cpp
keywords.txt

A revised version of the TinyGPS object library is included in the sketch folder
to avoid conflict with any earlier version you may have in the Arduino libraries 
location.

Connections:
PAM-7Q    Arduino
GND       GND
VDD       5V
TXD       Digital Pin 6
 
Reminder! Wait for the satellite lock LED to begin flashing before
taking readings. The flashing LED indicates satellite lock. Readings taken
before satellite lock may be inaccurate.

*/

// for GPS
#include <SoftwareSerial.h>
#include "math.h"
#include "./TinyGPS.h"                  // Use local version of this library
TinyGPS gps;
// for Sensor
#include <Wire.h>
#define ADDRESS 0x76 //0x77

// for GPS------------------------------------------------------------------------------------------
unsigned long age;
float balloonLat              = 34.28889;//[degrees]
float balloonLon              = 117.6458;//[degrees]
float balloonAlt              = 10064.0/5280.0;//[miles]
//const byte rxPin = 0;
//const byte txPin = 1;

// for Sensor------------------------------------------------------------------------------------------
uint32_t D1 = 0;
uint32_t D2 = 0;
int64_t dT = 0;
int32_t TEMP = 0;
int64_t OFF = 0; 
int64_t SENS = 0; 
int32_t P = 0;
uint16_t C[7];
int Temperature_cC; //in centiCelsius. *10^2
int Pressure_cP; //in centiPressure. *10^2;

void setup() {
  //for GPS------------------------------------------------------------------------------------------
  delay(1000);
  Serial.begin(9600);
  Serial1.begin(9600);                     // Communicate at 9600 baud (default for PAM-7Q module)
  delay(1000);

  // Test things - GPS
  //groundStationlat        = 34.22389;//[degrees]
  //groundStationlon        = 118.0603;//[degrees]
  //groundStationAlt        = 5710.0/5280.0;//[miles]

  // for Sensor------------------------------------------------------------------------------------------
  // Disable internal pullups, 10Kohms are on the breakout
  PORTC |= (1 << 4);
  PORTC |= (1 << 5);
  Wire.begin();
  //Serial.begin(9600); //9600 changed 'cos of timing?
  delay(100);
  initial(ADDRESS);
}

void loop() {
  //delay(1000);
  GPSstuff(); //delays 1 second
  SENSORstuff();
}


void GPSstuff() {
  bool newdata = false;
  unsigned long start = millis();       // starts a count of millisec since the code began 
  while (millis() - start < 1000) {     // Update every 1 seconds
    if (feedgps())                      // if serial1 is available and can read gps.encode
      newdata = true;
  }
  if (newdata) {  // if locked
    gpsdump(gps);
    
    //using GPS ------------------------------------------------------------------------------------------
    Serial.println("LOCKED ON");
    //Serial.print("Balloon Altitude: ");
    balloonAlt = gps.altitude();
    //Serial.println(balloonAlt);
    //gps.f_get_position(&balloonLat, &balloonLon, &age);
    //Serial.print("Balloon Latitude: ");
    //Serial.println(balloonLat);
    //Serial.print("Balloon Longitude: ");
    //Serial.println(balloonLon);   
  }else{          // if not locked
    Serial.println("Not Locked");
  }
}


void SENSORstuff() {
    D1 = getVal(ADDRESS, 0x48); // Pressure raw
    D2 = getVal(ADDRESS, 0x58);// Temperature raw
    // dT   = D2 - (C[5]*(2^8));
    dT   = D2 - ((uint32_t)C[5] << 8); //Difference between actual and reference temperature 
    //OFF  = ((int64_t)C[2] << 17) + ((dT * C[4]) >> 6);
    OFF  = ((int64_t)C[2] << 16) + ((dT * C[4]) >> 7); //Offset at actual temperature
    //SENS = ((int32_t)C[1] << 16) + ((dT * C[3]) >> 7);
    SENS = ((int32_t)C[1] << 15) + ((dT * C[3]) >> 8); //Sensitivity at actual temperature
    TEMP = (((int64_t)dT * (int64_t)C[6]) >> 23) + 2000;
    //TEMP = (int64_t)dT * (int64_t)C[6] / 8388608 + 2000; //Actual temperature
    if(TEMP < 2000) // if temperature lower than 20 Celsius 
    {
      int32_t T1    = 0;
      int64_t OFF1  = 0;
      int64_t SENS1 = 0;
      T1    = pow(dT, 2) / 2147483648;
      OFF1  = 5 * pow((TEMP - 2000), 2) / 2;
      SENS1 = 5 * pow((TEMP - 2000), 2) / 4;
      if(TEMP < -1500) // if temperature lower than -15 Celsius 
      {
        OFF1  = OFF1 + 7 * pow((TEMP + 1500), 2); 
        SENS1 = SENS1 + 11 * pow((TEMP + 1500), 2) / 2;
      }
      TEMP -= T1;
      OFF -= OFF1; 
      SENS -= SENS1;
    }
    //Temperature = (float)TEMP / 100;
    Temperature_cC = TEMP;
    //P  = ((int64_t)D1 * SENS / 2097152 - OFF) / 32768;
    P  = ((int64_t)D1 * SENS / 2097152 - OFF) / 16384;//32768;// instead of /(2^15) we /(2^14) to have realistic results of pressure
    //Pressure = (float)P / 100;
    Pressure_cP = P;
    //Serial.print("Tempurature = ");
    //Serial.print(Temperature);
    //Serial.print("      Actual PRESSURE= ");
    //Serial.print(Pressure);
}

// Get and process GPS data
void gpsdump(TinyGPS &gps) {
  unsigned long age;
  gps.f_get_position(&balloonLat, &balloonLon, &age);
  //Serial.print(balloonLat, 4); 
  //Serial.print(", "); 
  //Serial.println(balloonLon, 4);
}

// Feed data as it becomes available 
bool feedgps() {
  while (Serial1.available()) {
    if (gps.encode(Serial1.read()))
      return true;
  }
  return false;
}

// functions for Sensor------------------------------------------------------------------------------------------
long getVal(int address, byte code)
{
 unsigned long ret = 0;
 Wire.beginTransmission(address);
 Wire.write(code);
 Wire.endTransmission();
 delay(10);
 // start read sequence
 Wire.beginTransmission(address);
 Wire.write((byte) 0x00);
 Wire.endTransmission();
 Wire.beginTransmission(address);
 Wire.requestFrom(address, (int)3);
 if (Wire.available() >= 3)
 {
   ret = Wire.read() * (unsigned long)65536 + Wire.read() * (unsigned long)256 + Wire.read();
 }
 else {
   ret = -1;
 }
 Wire.endTransmission();
 return ret;
}

void initial(uint8_t address)
{
 Serial.println();
 Serial.println("PROM COEFFICIENTS ivan");
 Wire.beginTransmission(address);
 Wire.write(0x1E); // reset
 Wire.endTransmission();
 delay(10);
 for (int i=0; i<6  ; i++) {
   Wire.beginTransmission(address);
   Wire.write(0xA2 + (i * 2));
   Wire.endTransmission();
   Wire.beginTransmission(address);
   Wire.requestFrom(address, (uint8_t) 6);
   delay(1);
   if(Wire.available())
   {
      C[i+1] = Wire.read() << 8 | Wire.read();
   }
   else {
     Serial.println("Error reading PROM 1"); // error reading the PROM or communicating with the device
   }
   Serial.println(C[i+1]);
 }
 Serial.println();
}
