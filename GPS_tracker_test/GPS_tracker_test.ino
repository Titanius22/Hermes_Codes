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
#include "./TinyGPS.h"                  // Use local version of this library
TinyGPS gps;
// for Sensor
#include <Wire.h>
#define ADDRESS 0x76 //0x77

// for GPS------------------------------------------------------------------------------------------
unsigned long age;
float balloonLat              = 34.28889;//[degrees]
float balloonLon              = 91.6458;//[degrees]
float balloonAlt              = 10064.0;///5280.0;//[miles]
unsigned long long loloballoonLat             = 34.28889;//[degrees] /////////////////////////////////////////////////////FIX THIS lolo
unsigned long long loloballoonLon             = 91.6458;//[degrees]
unsigned long longballoonAlt             = 10064.0;///5280.0;//[miles]



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
unsigned int Temperature_cC; //RealTemp +60C (to remove negative) then *10^2 to get temp in centiCelsius. 
unsigned long Pressure_dP; //in decaPascels *10^2;

//For sending
unsigned long lineCount = 0;
char endLine[3] = {'E', 'N', 'D'};
//char CharsToSend[24];
//char* CharsToSend = malloc(24);
unsigned char* CharsToSend = malloc(22);
unsigned char* writeTo=CharsToSend;

unsigned char* writeArray=CharsToSend;
unsigned char** wrPtr=&writeArray;

bool newdata = false;

void setup() {
  // for communication with Pi
  Wire.begin(4);
  
  
  //for GPS------------------------------------------------------------------------------------------
  Serial.begin(74880);
  Serial1.begin(9600);                     // Communicate at 9600 baud (default for PAM-7Q module)
  
  delay(200);
  
  // for Sensor------------------------------------------------------------------------------------------
  // Disable internal pullups, 10Kohms are on the breakout
  PORTC |= (1 << 4);
  PORTC |= (1 << 5);
  delay(100);
  
  initial(ADDRESS);
  //GPSstuff(); 
  //SENSORstuff();
  //updateCharsToSend();
  
  Wire.onRequest(requestEvent); // register event
}

void loop() {
  //delay(1000);
  GPSstuff();
  
  if(newdata){
  
	  loloballoonLat = (long) (balloonLat*1000000)
	  loloballoonLon = (long) (balloonLon*1000000)
	  loloballoonAlt = (long) (balloonAlt*100)
	  
	  
	  
	  updateCharsToSend();
	  
	//  writeArray=CharsToSend;
	//  wrPtr=&writeArray;

	//  Serial.println((unsigned long)getIntFromByte(wrPtr,3));
	//
	//  Serial.println((unsigned long)getIntFromByte(wrPtr,4));
	//  
	//  Serial.println((unsigned long)getIntFromByte(wrPtr,4));
	//
	//  Serial.println((unsigned long)getIntFromByte(wrPtr,3));
	//
	//  Serial.println((unsigned int)getIntFromByte(wrPtr,2));
	//  
	//  Serial.println((unsigned long)getIntFromByte(wrPtr,3));
	//
	//  Serial.println((char)getIntFromByte(wrPtr,1));
	//
	//  Serial.println((char)getIntFromByte(wrPtr,1));
	//
	//  Serial.println((char)getIntFromByte(wrPtr,1));
	  
	  //lineCount; //Wierd. This must be here for linecount to increment in the requestEvent()
	  //lineCount++; // increments in updateCharsToSend
  }
  delay(300);
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  //GPSstuff();
  //SENSORstuff();
  //char* CharsToSend = updateCharsToSend();
  //updateCharsToSend();
  //lineCount++;
  //updateCharsToSend();
  Wire.write(CharsToSend, 32); // respond with message of 32 byte
  //updateCharsToSend();
  //free(CharsToSend);
  //Wire.write("ftgyho04856000r57j0k?0");
  
  //GPSstuff();
  //SENSORstuff();
  
  
}

//char* updateCharsToSend(){
void updateCharsToSend(){
  free(CharsToSend);
  CharsToSend = malloc(22);
  writeTo=CharsToSend;
  unsigned int intBuflineCount;
  ////////////////////////unsigned long longBuflatitude;
  ////////////////////////unsigned long longBuflongitude;
  ////////////////////////unsigned long intBufaltitude;
  unsigned int intBuftemperature;
  unsigned int intBufpressure;

  //Line counter-------------------------------------------
  intBuflineCount = lineCount;
  insertBytesFromInt(&intBuflineCount, &writeTo, 3);

  //Latitude * 10^5 positive only----------------should be 10^10-----------
  longBuflatitude = (unsigned long)(balloonLat * 100000);
  insertBytesFromInt(&longBuflatitude, &writeTo, 4);

  //Longitude * 10^5 positive only max of 109 degrees---should be 10^10-----
  longBuflongitude = (unsigned long)(balloonLon * 100000);
  insertBytesFromInt(&longBuflongitude, &writeTo, 4);

  //Altitude * 100--------------------------------------------
  intBufaltitude = balloonAlt * 100;
  insertBytesFromInt(&intBufaltitude, &writeTo, 3);

  //Temperature count------------------------------------------
  intBuftemperature = 4565;
  insertBytesFromInt(&intBuftemperature, &writeTo, 2);

  //Pressure count---------------------------------------------
  intBufpressure = Pressure_dP;
  insertBytesFromInt(&intBufpressure, &writeTo, 3);

  //End of line chars-------------------------------------------

  CharsToSend[19] = endLine[0];
  CharsToSend[20] = endLine[1];
  CharsToSend[21] = endLine[2];

  //lineCount++;
}


void insertBytesFromInt(void* value,unsigned char** byteStart, short numberBytesToCopy){

  unsigned char* valueBytes=value;
  short loopCount=0;
  for(loopCount=0;loopCount<numberBytesToCopy;loopCount++){
    (*byteStart)[loopCount]=valueBytes[loopCount];
  }
  *byteStart+=(short)numberBytesToCopy;
}

unsigned long getIntFromByte(unsigned char** arrayStart, short bytes){
//unsigned long long getIntFromByte(unsigned char** arrayStart, short bytes){

  //Allocating array to read into
  unsigned char* intPtr=malloc (sizeof(unsigned long long));
  unsigned long long temp;
  //Void pointer to same location to return

   //Loop Counter
  short loopCount;
  for(loopCount=0;loopCount<bytes;loopCount++){

    //Copying bytes from one array to the other
    if(loopCount<bytes){
      intPtr[loopCount]=(*arrayStart)[loopCount];
    }
  }
  *arrayStart+=(short)bytes;
  temp=*((unsigned long long*)intPtr);
  free(intPtr);
  //Returning void pointer (Pointer to an integer with the designated of the number of bytes)
  return temp;
}

void GPSstuff() {
  int64_t start = millis();       // starts a count of millisec since the code began 
  newdata = false;
  while (millis() - start < 250) {     // Update every 1 seconds
    if (feedgps())                      // if serial1 is available and can read gps.encode
      newdata = true;
  }
  if (newdata) {  // if locked
    gpsdump(gps);
     
    
    //using GPS ------------------------------------------------------------------------------------------
    Serial.println("LOCKED ON");
    //Serial.print("Balloon Altitude: ");
    balloonAlt = gps.altitude(); 
  }else{          // if not locked
    Serial.println("Not Locked");
  }
}

// Get and process GPS data
void gpsdump(TinyGPS &gps) {
  //unsigned long age;
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
