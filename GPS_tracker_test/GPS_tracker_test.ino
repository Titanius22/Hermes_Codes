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
//#define ADDRESS 0x76 //0x77

// for GPS------------------------------------------------------------------------------------------
unsigned long age;
float balloonLat              = 40;//34.28889;//[degrees]
float balloonLon              = 50;//91.6458;//[degrees]
float balloonAlt              = 60;//10064.0;///5280.0;//[miles]
unsigned long longballoonLat             = 10;//34.28889;//[degrees] /////////////////////////////////////////////////////FIX THIS long
unsigned long longballoonLon             = 20;//91.6458;//[degrees]
unsigned long longballoonAlt             = 30;//10064.0;///5280.0;//[miles]



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
char endLine[3] = {'E', 'N', 'D'};
//char CharsToSend[24];
//char* CharsToSend = malloc(24);
unsigned char* CharsToSend;// = "HELLO!!!!!!!!!!!!!!!!!!!!!!!!!!!";// malloc(32);
unsigned char* writeTo=CharsToSend;
unsigned char* writeArray=CharsToSend;
unsigned char** wrPtr=&writeArray;

bool newdata = false;
unsigned short LineLength = 29; //excludes checksum byte

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
  
  //initial(ADDRESS);
  //GPSstuff(); 
  //SENSORstuff();
  //updateCharsToSend();
  
  Wire.onRequest(requestEvent); // register event
  
  updateCharsToSend();
  
}

void loop() {
  //Serial.print((char) CharsToSend[0]);
  //Serial.print((char) CharsToSend[1]);
  //Serial.print((char) CharsToSend[2]);
  //Serial.print((char) CharsToSend[3]);
  //Serial.print((char) CharsToSend[4]);
  //Serial.print((char) CharsToSend[5]);
  
  //delay(1000);
  GPSstuff();

  //updateCharsToSend();
  
  if(newdata){
    
    updateCharsToSend();
    
	  longballoonLat = (unsigned long) (balloonLat*10000);
	  longballoonLon = (unsigned long) (balloonLon*10000);
	  longballoonAlt = (unsigned long) (balloonAlt*100);
	  
	  writeArray=CharsToSend;
	  wrPtr=&writeArray;
    
    Serial.println((unsigned int)getIntFromByte(wrPtr,2));
  
    Serial.println((unsigned long)getIntFromByte(wrPtr,3));
  
    Serial.println((unsigned long)getIntFromByte(wrPtr,3));
  
    Serial.println((unsigned long)getIntFromByte(wrPtr,3));
  
    Serial.println((unsigned int)getIntFromByte(wrPtr,2));
  
    Serial.println((unsigned int)getIntFromByte(wrPtr,2));
	  
	  Serial.println((unsigned short)getIntFromByte(wrPtr,1));
  
    Serial.println((unsigned short)getIntFromByte(wrPtr,1));
  
    Serial.println((unsigned short)getIntFromByte(wrPtr,1));
  
    Serial.println((unsigned short)getIntFromByte(wrPtr,1));
  
    Serial.println((unsigned short)getIntFromByte(wrPtr,1));
  
    Serial.println((unsigned short)getIntFromByte(wrPtr,1));
  
    Serial.println((unsigned long)getIntFromByte(wrPtr,3));
	
	  Serial.println((unsigned int)getIntFromByte(wrPtr,2));
	
	  Serial.println((char)getIntFromByte(wrPtr,1));
	
	  Serial.println((char)getIntFromByte(wrPtr,1));
	
	  Serial.println((char)getIntFromByte(wrPtr,1));
	  
	  //lineCount; //Wierd. This must be here for linecount to increment in the requestEvent()
	  //lineCount++; // increments in updateCharsToSend
  }

  Serial.print("LAT: ");
  Serial.println(balloonLat);
  Serial.print("LON: ");
  Serial.println(balloonLon);
  Serial.print("ALT: ");
  Serial.println(balloonAlt);
  Serial.println("");
  
  delay(500);
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  Serial.println("requestEvent");
  //GPSstuff();
  //SENSORstuff();
  //char* CharsToSend = updateCharsToSend();
  //updateCharsToSend();
  //lineCount++;
  //updateCharsToSend();

  Wire.write(CharsToSend, 30); // respond with message of 32 byte
  //Wire.write("W", 1); // respond with message of 32 byte
  
  //updateCharsToSend();
  //free(CharsToSend);
  //Wire.write("ftgyho04856000r57j0k?0");
  
  //GPSstuff();
  //SENSORstuff();
  
  
}

//char* updateCharsToSend(){
void updateCharsToSend(){
	free(CharsToSend);
	CharsToSend = malloc(LineLength+1);
	writeTo=CharsToSend;
	unsigned int sum = 0;
	short i;

	//Line counter-------------------------------------------
	unsigned int intBuflineCount = 4567;
	insertBytesFromInt(&intBuflineCount, &writeTo, 2);

	//Latitude * 10^5 positive only----------------should be 10^10-----------
	//longballoonLat = (unsigned long)(balloonLat * 100000);
  Serial.print("balloon lat: ");
  //Serial.println((long)longballoonLat);
	insertBytesFromInt(&longballoonLat, &writeTo, 3);

	//Longitude * 10^5 positive only max of 109 degrees---should be 10^10-----
	//longBuflongitude = (unsigned long long)(balloonLon * 100000);
	insertBytesFromInt(&longballoonLon, &writeTo, 3);

	//Altitude * 100--------------------------------------------
	//long intBufaltitude = 1000 * 100;
	insertBytesFromInt(&longballoonAlt, &writeTo, 3);
	
	//Time (seconds since UTC half day) --------------------------------------------
	unsigned int intBuftemperature1 = 450;
	insertBytesFromInt(&intBuftemperature1, &writeTo, 2);

	//Thermistor count------------------------------------------
	unsigned int intBuftemperature = 450;
	insertBytesFromInt(&intBuftemperature, &writeTo, 2);

	//Battery Voltage---------------------------------------------
	unsigned short intBufpressure = 120;
	insertBytesFromInt(&intBufpressure, &writeTo, 1);
	
	//Battery Current---------------------------------------------
	unsigned short intBufpressure12 = 140;
	insertBytesFromInt(&intBufpressure12, &writeTo, 1);

	//Magnotometer X---------------------------------------------
	unsigned short intBufpressure2 = 80;
	insertBytesFromInt(&intBufpressure2, &writeTo, 1);

	//Magnotometer Y---------------------------------------------
	unsigned short intBufpressure3 = 60;
	insertBytesFromInt(&intBufpressure3, &writeTo, 1);

	//Magnotometer Z---------------------------------------------
	unsigned short intBufpressure4 = 40;
	insertBytesFromInt(&intBufpressure4, &writeTo, 1);

	//Humidity---------------------------------------------
	unsigned short intBufpressure5 = 96;
	insertBytesFromInt(&intBufpressure5, &writeTo, 1);

	//Pressure---------------------------------------------
	unsigned long intBufpressure6 = 102300;
	insertBytesFromInt(&intBufpressure6, &writeTo, 3);

	//Internal Temperature---------------------------------------------
	unsigned int intBufpressure7 = 15;
	insertBytesFromInt(&intBufpressure7, &writeTo, 2);

	CharsToSend[LineLength-3] = endLine[0];
	CharsToSend[LineLength-2] = endLine[1];
	CharsToSend[LineLength-1] = endLine[2];
	
	for(i=0;i<LineLength;i++){
		sum += (unsigned char)CharsToSend[i];
	}
	CharsToSend[LineLength] = (sum%64);
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

// void initial(uint8_t address)
// {
// Serial.println();
// Serial.println("PROM COEFFICIENTS ivan");
// Wire.beginTransmission(address);
// Wire.write(0x1E); // reset
// Wire.endTransmission();
 
// delay(10);
// for (int i=0; i<6  ; i++) {
  // Wire.beginTransmission(address);
  // Wire.write(0xA2 + (i * 2));
  // Wire.endTransmission();
  // Wire.beginTransmission(address);
  // Wire.requestFrom(address, (uint8_t) 6);
  // delay(1);
  // if(Wire.available())
  // {
     // C[i+1] = Wire.read() << 8 | Wire.read();
  // }
  // else {
    // Serial.println("Error reading PROM 1"); // error reading the PROM or communicating with the device
  // }
  // Serial.println(C[i+1]);
// }
// Serial.println();
// }
