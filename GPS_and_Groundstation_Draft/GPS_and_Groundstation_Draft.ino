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

//#include <SoftwareSerial.h>
#include "math.h"
#include "TinyGPS.h"                 // Use local version of this library
TinyGPS gps;
unsigned long age;
const byte rxPin = 0;
const byte txPin = 1;
//float Re = 6371; //[km]
float Re = 3956.0 ; //[miles]
float pi = 3.14159265359 ; 

float distancefromballoontoGS = 0;   

float balloonLat              = 34.28889;//[degrees]
float balloonLon              = 117.6458;//[degrees]
float balloonAlt              = 10064.0/5280.0;//[miles]

//Hard coded for ERAU Daytona Beach Campus;//Test values
float groundStationlat        = 29.188157;//34.22389;//[degrees]
float groundStationlon        = -81.048182;//118.0603;//[degrees]
float groundStationAlt        = 0;//5710.0/5280.0;//[miles]

float d; 
float Azimuth; 
float Elevation; 
String command = "W000 000";


void setup() {
  delay(1000);
  Serial.begin(4800);
  Serial1.begin(9600);                     // Communicate at 9600 baud (default for PAM-7Q module)
  delay(1000);
  //Serial.print("Command to send to rotator: ");
  //Serial.println("");
  //Serial.println(command);
  //Serial.println("Reading GPS");
  //balloonLat = (balloonLat*pi)/180;              //[radian]
  //balloonLon = (balloonLon*pi)/180;              //[radian]
  //groundStationlat = (groundStationlat*pi)/180;  //[radian]
  //groundStationlon = (groundStationlon*pi)/180;  //[radian]
  
  //d = findDistance(balloonLon, balloonLat, groundStationlon, groundStationlat);
  //Serial.println(d);
  //Azimuth = findAzimuth(balloonLon, balloonLat, groundStationlon, groundStationlat, d);
  //Serial.println((Azimuth*180)/pi);
  //Elevation = findElevation(balloonAlt, groundStationAlt, d);
  //Serial.println((Elevation*180)/pi);
  

}

void loop() {
  Serial.print("hey");
  delay(1000);

/*
  while (Serial1.available()){
    char c=Serial1.read();
    //Serial.print(c);
    delay(1000);
    //if(gps.encode(c)){
      // Test things
      groundStationlat        = 34.22389;//[degrees]
      groundStationlon        = 118.0603;//[degrees]
      groundStationAlt        = 5710.0/5280.0;//[miles]
      
      balloonLat              = 34.28889;//[degrees]
      balloonLon              = 117.6458;//[degrees]
      balloonAlt              = 10064.0/5280.0;//[miles]
    
      Serial.println("LOCKED OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOONNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN");
      Serial.print("Balloon Altitude: ");
      Serial.println(gps.altitude());
      //gps.f_get_position(&balloonLat, &balloonLon, &age);
      Serial.print("Balloon Latitude: ");
      Serial.println(balloonLat);
      Serial.print("Balloon Longitude: ");
      Serial.println(balloonLon);   


       
      balloonLat = (balloonLat*pi)/180;              //[radian]
      balloonLon = (balloonLon*pi)/180;              //[radian]
      groundStationlat = (groundStationlat*pi)/180;              //[radian]
      groundStationlon = (groundStationlon*pi)/180;              //[radian]
      
      d = findDistance(balloonLon, balloonLat, groundStationlon, groundStationlat);
      Serial.print("Distance from Ground Station to Balloon: ");
      Serial.println(d);
      
      Azimuth = findAzimuth(balloonLon, balloonLat, groundStationlon, groundStationlat, d);
      Azimuth = (Azimuth*180)/pi;
      Serial.print("Azimuth of Ground Station: ");
      Serial.println(Azimuth);
      
      Elevation = findElevation(balloonAlt, groundStationAlt, d);
      Elevation = (Elevation*180)/pi;
      Serial.print("Elevation of Ground Station: ");
      Serial.println(Elevation);

      Azimuth = round(Azimuth);
      Elevation = round(Elevation);
      command = "W";
      if (Azimuth <= 9){
        command = command + "00" + String(int(Azimuth)) + " ";
      } else if (Azimuth <= 99){
        command = command + "0" + String(int(Azimuth)) + " ";
      } else {
        command = command + String(int(Azimuth)) + " ";
      }
      if (Elevation <= 9){
        command = command + "00" + String(int(Elevation));
      } else if (Azimuth <= 99){
        command = command + "0" + String(int(Elevation));
      } else {
        command = command + String(int(Elevation));
      }
      
      Serial.print("Command to send to rotator: ");
      Serial.println("");
      Serial.print(command + "\n\n");
    //}else{
      //Serial.println("Hasn't locked on satellite");
    //}
  }
*/
}

















// calculates distance along the surface of the Earth between the points at sea level below each of the points
float findDistance(float balloonLon, float balloonLat, float groundStationlon, float groundStationlat) {
  float dlon, dlat, a, c, d;
  // ground station = 1
  // balloon = 2
  dlon = balloonLon - groundStationlon;
  dlat = balloonLat - groundStationlat;
  a = pow((sin(dlat/2)),2) + cos(groundStationlat) * cos(balloonLat) * pow((sin(dlon/2)),2);
  c = 2 * asin(min(1,sqrt(a)));
  d = Re * c;
  return d;
}
// calculates distance along the surface of the Earth between the points at sea level below each of the points
float findAzimuth(float balloonLon, float balloonLat, float groundStationlon, float groundStationlat, float d) {
  float x;
  // ground station = 1
  // balloon = 2
  x = acos( (sin(balloonLat) - sin(groundStationlat)*cos(d/Re)) / (sin(d/Re)*cos(groundStationlat)) );
  if(sin(balloonLon-groundStationlon) > 0){
    x = (2*pi)-x;
  }
  return x;
}
// calculates distance along the surface of the Earth between the points at sea level below each of the points
float findElevation(float balloonAlt, float groundStationAlt, float d) {
  float el;
  // ground station = 1
  // balloon = 2
  //el = asin((balloonAlt - groundStationAlt)/d);
  el = asin(((balloonAlt - groundStationAlt)/d) - (d/(2.0*Re)));
  return el;
}

// Get and process GPS data
void gpsdump(TinyGPS &gps) {
  float flat, flon;
  unsigned long age;
  gps.f_get_position(&flat, &flon, &age);
  Serial.print(flat, 4); Serial.print(", "); 
  Serial.println(flon, 4);
}

// Feed data as it becomes available 
bool feedgps() {
  while (Serial1.available()) {
    //char temp=Serial1.read();
    //Serial.println("here2");
    //Serial.print(temp);
    delay(500);
    gpsdump(gps);

    //return true;
  }
  return false;
}
