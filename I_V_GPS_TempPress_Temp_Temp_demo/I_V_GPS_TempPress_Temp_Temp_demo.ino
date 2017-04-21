/*
RedBoard (Arduino) DS1620 Temperature Measurements 

The Redboard sets up and shows current temperature over serial monitor. 
This code can measure negative and positive temperature values.

NOTE: This code was written to be easy to understand 
and optimization was not taken into particular consideration.
Please consider this if you are using this for a project

created April 2, 2014 
by Ronald Ramdhan (http://trinirobotics.com)

Copyright (c) 2014, Trinirobotics 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of TriniRobotics nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL TRINIROBOTICS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


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
#include <math.h>
#include <SoftwareSerial.h>
#include "./TinyGPS.h"                  // Use local version of this library
TinyGPS gps;
// for Sensor
#include <Wire.h>
#define ADDRESS 0x76 //0x77
#define ThermistorPIN 0
#define analogPinV 1    // potentiometer wiper (middle terminal) connected to analog pin 3outside leads to ground and +5V
#define analogPinI 2
////Internal temp
//#define DQ_INTERNAL_PIN 7
//#define CLK_INTERNAL_PIN 8
//#define RESET_INTERNAL_PIN 9
//External temp
#define DQ_EXTERNAL_PIN 10
#define CLK_EXTERNAL_PIN 11
#define RESET_EXTERNAL_PIN 12

// for GPS------------------------------------------------------------------------------------------
unsigned long age;
//float balloonLat              = 40;//34.28889;//[degrees]
//float balloonLon              = 50;//91.6458;//[degrees]
//float balloonAlt              = 60;//10064.0;///5280.0;//[miles]
long longBalloonLat             = 10;//34.28889;//[degrees] /////////////////////////////////////////////////////FIX THIS lolo
long longBalloonLon             = 20;//91.6458;//[degrees]
long longBalloonAlt             = 30;//10064.0;///5280.0;//[miles]
unsigned long longBalloonTime   = 40;
unsigned long longBalloonDate   = 50;

// for Housekeeping------------------------------------------------------------------------------------------
unsigned char Vcount;
unsigned char Icount;

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
unsigned long longBalloonIntTemp = 600; // in centi-kelvin
unsigned long longBalloonPressure = 700; //in decaPascels *10^2;
unsigned int intBalloonExtTemp = 800; // in 10-bit voltage count 
float Temp;
float Pressure;
// float TherTemp;
int intIntTempChip = 450; // signed count in 0.0625C increaments internal DS 1620 chip
int intExtTempChip = 412; // signed count in 0.0625C increaments external DS 1620 chip
double Temp_Read;
double Count_Remain;
double Count_Per_C;
double Temperature;
  

//For sending
char endLine[3] = {'E', 'N', 'D'};
//char CharsToSend[24];
//char* CharsToSend = malloc(24);
unsigned char* CharsToSend;// = "HELLO!!!!!!!!!!!!!!!!!!!!!!!!!!!";// malloc(32);
unsigned char* writeTo=CharsToSend;
//unsigned char* writeArray=CharsToSend;
//unsigned char** wrPtr=&writeArray;

float Vvoltread;    // read the input pin
float Ivoltread;    // read the input pin
float Vin;    // read the input pin
float Iin;    // read the input pin

bool newdata = false;
unsigned short LineLength = 29; //excludes checksum byte
int64_t start;

void setup() {
	// for communication with Pi
	Wire.begin(4);
	
	Serial.begin(74880);
	
	//for GPS------------------------------------------------------------------------------------------
	
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
  Initiate_Temp_Chips();

	Wire.onRequest(requestEvent); // register event

	updateCharsToSend();
  
}

void loop() {

	delay(1000);
	
	GPSstuff();
	HOUSEKEEPINGstuff();
	SENSORstuff();
	updateCharsToSend(); 	
	
	/*
	writeTo=CharsToSend;
	wrPtr=&writeTo;

	Serial.print((unsigned long)getIntFromByte(wrPtr,2));
	Serial.print(" ");
	Serial.print((unsigned long)getIntFromByte(wrPtr,3));
	Serial.print(" ");
	Serial.print((unsigned long)getIntFromByte(wrPtr,3));
	Serial.print(" ");
	Serial.print((unsigned long)getIntFromByte(wrPtr,3));
	Serial.print(" ");
	Serial.print((unsigned int)getIntFromByte(wrPtr,2));
	Serial.print(" ");
	Serial.print((unsigned int)getIntFromByte(wrPtr,2));
	Serial.print(" ");
	Serial.print((unsigned char)getIntFromByte(wrPtr,1));
	Serial.print(" ");
	Serial.print((unsigned char)getIntFromByte(wrPtr,1));
	Serial.print(" ");
	Serial.print((unsigned char)getIntFromByte(wrPtr,1));
	Serial.print(" ");
	Serial.print((unsigned char)getIntFromByte(wrPtr,1));
	Serial.print(" ");
	Serial.print((unsigned char)getIntFromByte(wrPtr,1));
	Serial.print(" ");
	Serial.print((unsigned char)getIntFromByte(wrPtr,1));
	Serial.print(" ");
	Serial.print((unsigned long)getIntFromByte(wrPtr,3));
	Serial.print(" ");
	Serial.print((unsigned int)getIntFromByte(wrPtr,2));
	Serial.print(" ");
	Serial.print((char)getIntFromByte(wrPtr,1));
	Serial.print(" ");
	Serial.print((char)getIntFromByte(wrPtr,1));
	Serial.print(" ");
	Serial.println((char)getIntFromByte(wrPtr,1));
	*/

	Serial.print("LAT: ");
	Serial.println(longBalloonLat);
	Serial.print("LON: ");
	Serial.println(longBalloonLon);
	Serial.print("ALT: ");
	Serial.println(longBalloonAlt);
	Serial.print("DATE: ");
	Serial.println(longBalloonDate);
	Serial.print("TIME: ");
	Serial.println(longBalloonTime);
	//Serial.println("");

	Serial.print("\nV Value: ");         
	Serial.print(Vcount);      
	Serial.print(", ");       
	Serial.print(Vvoltread);   
	Serial.print(", ");       
	Serial.print(Vin);          
	Serial.print("\nI Value: ");    
	Serial.print(Icount);
	Serial.print(", ");
	Serial.print(Ivoltread);    
	Serial.print(", ");      
	Serial.print(Iin);         
	Serial.print("\n");  

	Serial.print("Temperature = ");
	Serial.println(Temp);
	Serial.print("Pressure = ");
	Serial.println(Pressure);

// Serial.print("Int Temp Chip = ");
// Serial.println(((double)intIntTempChip)*0.0625);
// Serial.print("Ext Temp Chip = ");
// Serial.println(((double)intExtTempChip)*0.0625);
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
	Serial.println("requestEvent");
	Wire.write(CharsToSend, 30); // respond with message of 32 byte
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
	//longBuflatitude = (unsigned long long)(longBalloonLat * 100000);
	insertBytesFromInt(&longBalloonLat, &writeTo, 3);

	//Longitude * 10^5 positive only max of 109 degrees---should be 10^10-----
	//longBuflongitude = (unsigned long long)(longBalloonLon * 100000);
	insertBytesFromInt(&longBalloonLon, &writeTo, 3);

	//Altitude * 100--------------------------------------------
	//long intBufaltitude = 1000 * 100;
	insertBytesFromInt(&longBalloonAlt, &writeTo, 3);
	
	//Time (seconds since UTC half day) --------------------------------------------
	//unsigned int longBalloonTime = 450;
	insertBytesFromInt(&longBalloonTime, &writeTo, 2);

	//Thermistor count--------------------------------------------
	//unsigned int intBalloonExtTemp = 450;
	insertBytesFromInt(&intBalloonExtTemp, &writeTo, 2);

	//Battery Voltage---------------------------------------------
	//unsigned short Vcount = 120;
	insertBytesFromInt(&Vcount, &writeTo, 1);
	
	//Battery Current---------------------------------------------
	//unsigned short Icount = 140;
	insertBytesFromInt(&Icount, &writeTo, 1);

	//Internal Temp Chip------------------------------------------
	//intIntTempChip = 80;
	insertBytesFromInt(&intIntTempChip, &writeTo, 2);

	//Internal Temp Chip---------------------------------------------
	//short intBufpressure3 = 60;
	insertBytesFromInt(&intExtTempChip, &writeTo, 2);

	//Pressure---------------------------------------------
	//unsigned long longBalloonPressure = 102300;
	insertBytesFromInt(&longBalloonPressure, &writeTo, 3);

	//Internal Temperature---------------------------------------------
	//unsigned int longBalloonIntTemp = 15;
	insertBytesFromInt(&longBalloonIntTemp, &writeTo, 2);

	CharsToSend[LineLength-3] = endLine[0];
	CharsToSend[LineLength-2] = endLine[1];
	CharsToSend[LineLength-1] = endLine[2];
	
	for(i=0;i<LineLength;i++){
		sum += (unsigned char)CharsToSend[i];
	}
	CharsToSend[LineLength] = (sum%64) + 1;
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
	for(loopCount=0;loopCount<sizeof(unsigned long);loopCount++){

		//Copying bytes from one array to the other
		if(loopCount<bytes){
		  intPtr[loopCount]=(*arrayStart)[loopCount];
		}else{
            intPtr[loopCount]=0;
        }
	}
	*arrayStart+=(short)bytes;
	temp=*((unsigned long long*)intPtr);
	free(intPtr);
	//Returning void pointer (Pointer to an integer with the designated of the number of bytes)
	return temp;
}

void GPSstuff() {
	newdata = false;
	int64_t hack = millis();
	if (hack - start > 250) {     // Update every 1 seconds
		start = hack;
		if (feedgps()){                    // if serial1 is available and can read gps.encode
			newdata = true;
		}
	}
	if (newdata) {  // if locked
		gpsdump(gps);

		//using GPS ------------------------------------------------------------------------------------------
		Serial.println("LOCKED ON");
		//Serial.print("Balloon Altitude: ");
		
	}else{          // if not locked
		Serial.println("Not Locked");
	}
}

// Get and process GPS data
void gpsdump(TinyGPS &gps) {
	longBalloonAlt = gps.altitude();
	gps.get_position(&longBalloonLat, &longBalloonLon, &age);
  	longBalloonLon = -longBalloonLon;
	gps.get_datetime(&longBalloonDate, &longBalloonTime, &age);
	timeConvert(longBalloonTime);
}

// Feed data as it becomes available 
bool feedgps() {
	while (Serial1.available()) {
		if (gps.encode(Serial1.read()))
			return true;
	}
	return false;
}

// Feed data as it becomes available 
void HOUSEKEEPINGstuff() {
	float Vread = 0;           // variable to store the value read
	float Iread = 0;           // variable to store the value read

	Vread = round(analogRead( analogPinV )/4);    // read the input pin
	Iread = round(analogRead( analogPinI )/4);    // read the input pin
	Vvoltread = (5*Vcount)/255;    // read the input pin
	Ivoltread = (5*Icount)/255;    // read the input pin
	Vin = 3.4976*Vvoltread - 0.2541;    // read the input pin
	Iin = 0.1116*Ivoltread - 0.0009;    // read the input pin

	Vcount = (unsigned char) Vread;
	Icount = (unsigned char) Iread;
}

void SENSORstuff() {
	// Tempurature and pressure good
	D1 = getVal(ADDRESS, 0x48); // Pressure raw
	D2 = getVal(ADDRESS, 0x58);// Temperature raw
	// dT   = D2 - (C[5]*(2^8));
	dT   = D2 - ((uint32_t)C[5] << 8); //Difference between actual and reference temperature 
	OFF  = ((int64_t)C[2] << 16) + ((dT * C[4]) >> 7); //Offset at actual temperature
	SENS = ((int32_t)C[1] << 15) + ((dT * C[3]) >> 8); //Sensitivity at actual temperature
	TEMP = (int64_t)dT * (int64_t)C[6] / 8388608 + 2000; //Actual temperature
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
	////////Temp = (float)TEMP / 100;
	longBalloonIntTemp = TEMP + 27315; // Converts to to Kelvin
	P  = ((int64_t)D1 * SENS / 2097152 - OFF) / 16384;//32768;// instead of /(2^15) we used (2^14) to have realistic results of pressure
	longBalloonPressure = P;
	///////Pressure = (float)P / 100;

	intBalloonExtTemp = analogRead( ThermistorPIN );

  // DS 1620 chips
  // Internal temp
//  Temp_Read = getTemp_Read( CLK_INTERNAL_PIN , DQ_INTERNAL_PIN , RESET_INTERNAL_PIN );
//  Count_Remain = getCount_Remain( CLK_INTERNAL_PIN , DQ_INTERNAL_PIN , RESET_INTERNAL_PIN );
//  Count_Per_C = getCount_Per_C( CLK_INTERNAL_PIN , DQ_INTERNAL_PIN , RESET_INTERNAL_PIN );
//  Temperature = ((Temp_Read/2) - 0.25) + ((Count_Per_C-Count_Remain)/Count_Per_C);
//  intIntTempChip = Temperature/0.0625;
  // DS 1620 chips
  // External temp
  Temp_Read = getTemp_Read( CLK_EXTERNAL_PIN , DQ_EXTERNAL_PIN , RESET_EXTERNAL_PIN );
  Count_Remain = getCount_Remain( CLK_EXTERNAL_PIN , DQ_EXTERNAL_PIN , RESET_EXTERNAL_PIN );
  Count_Per_C = getCount_Per_C( CLK_EXTERNAL_PIN , DQ_EXTERNAL_PIN , RESET_EXTERNAL_PIN );
  Temperature = ((Temp_Read/2) - 0.25) + ((Count_Per_C-Count_Remain)/Count_Per_C);
  intExtTempChip = Temperature/0.0625;
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

float Thermistor(int RawADC) {
	// Inputs ADC Value from Thermistor and outputs Temperature in Celsius
	//  requires: include <math.h>
	// Utilizes the Steinhart-Hart Thermistor Equation:
	//    Temperature in Kelvin = 1 / {A + B[ln(R)] + C[ln(R)]^2 + D[ln(R)]^3}
	//    where A = 0.001026853381291383, B = 0.0002398699085819556 and C = -0.00000007884414548067204, D = 0.0000001594372457358320  (calculated using R-T table and Coeff program

	long Resistance;  float Temp;  // Dual-Purpose variable to save space.
	Resistance=10000.0*((1023.0/RawADC) - 1);  // Assuming a 10k Thermistor.  Calculation is actually: Resistance = (1023 /ADC -1) * BalanceResistor
	// For a GND-Thermistor-PullUp--Varef circuit it would be Rtherm=Rpullup/(1023.0/ADC-1)

	Temp = log(Resistance); // Saving the Log(resistance) so not to calculate it 4 times later. // "Temp" means "Temporary" on this line.
	Temp = 1 / (0.001026853381291383 + (0.0002398699085819556 * Temp) + (-0.00000007884414548067204 * Temp * Temp) + (0.0000001594372457358320*Temp*Temp*Temp));   // Now it means both "Temporary" and "Temperature"
	Temp = Temp - 273.15;  // Convert Kelvin to Celsius                                         // Now it only means "Temperature"

	// Uncomment this line for the function to return Fahrenheit instead.
	//Temp = (Temp * 9.0)/ 5.0 + 32.0; // Convert to Fahrenheit
	return Temp;  // Return the Temperature
}

void printDouble(float val, byte precision) {
	// prints val with number of decimal places determine by precision
	// precision is a number from 0 to 6 indicating the desired decimal places
	// example: printDouble(3.1415, 2); // prints 3.14 (two decimal places)
	Serial.print (int(val));  //prints the int part
	if( precision > 0) {
		Serial.print("."); // print the decimal point
		unsigned long frac, mult = 1;
		byte padding = precision -1;
		while(precision--) mult *=10;
		if(val >= 0) frac = (val - int(val)) * mult; else frac = (int(val) - val) * mult;
		unsigned long frac1 = frac;
		while(frac1 /= 10) padding--;
		while(padding--) Serial.print("0");
		Serial.print(frac,DEC) ;
	}
}

// Converts time from UTC hhmmsscc to the number of seconds since the last UTC half day (resets to 0 every 12 hours)
void timeConvert(unsigned long &timeVar){
	unsigned long buffVar = timeVar;
	unsigned short hours;
	unsigned short minutes;
	unsigned short seconds;
	
	buffVar = buffVar/100; // removes centi-seconds
	
	seconds = buffVar%100; // gets seconds
	buffVar = buffVar/100; // removes seconds
	
	minutes = buffVar%100; // gets minutes
	buffVar = buffVar/100; // removes minutes
	
	hours = buffVar%12; // only hours left, max of 24.
	
	timeVar = seconds + (60*minutes) + (3600*hours);
}

//This function sends the command to the 
//chip, bit by bit across the DQ Pin
void sendCommand (int cmd, byte clkPIN, byte dqPIN) {
  short i;
  int output;

  //Each bit in "cmd" is sent on a clock pulse
  //this means we must extract each bit and send
  //it across. 
  for( i = 0; i < 8; i++) {
    output = ((cmd >> i) & (0x01));    //Each bit is extracted and sent across one by one
    digitalWrite(dqPIN, output);      //Setting the DQ to the analog value
    digitalWrite(clkPIN, LOW);        //Clock cycle
    digitalWrite(clkPIN, HIGH);
  }  
}

//This function extracts the Temp_Read
//from the DS1620 according to the datasheet
//to get higher resolution temp
//http://datasheets.maximintegrated.com/en/ds/DS1620.pdf
int getTemp_Read(byte clkPIN, byte dqPIN, byte resetPIN) {
  
  int i;
  int dqValue = 0;
  int temperatureValue = 0;
  
  //Request the temperature value just produced
  //Reset and Clock must be driven high before a command is sent
  digitalWrite(clkPIN, HIGH);
  digitalWrite(resetPIN, HIGH);
  sendCommand(0xAA, clkPIN, dqPIN);    //Command to request temperature value
  
  pinMode(dqPIN, INPUT);      //DQ pin must be set high in order to operate as an input
  
  //This for loop reads 9 bits one by one. It shifts them 
  //into the temperatureValue integer on each iteration
  for(i = 0; i < 9; i++) {
    digitalWrite(clkPIN, LOW);      //Clock low to read first 'bit'
    dqValue = digitalRead(dqPIN);   //Store current value on dq pin
    digitalWrite(clkPIN, HIGH);     //Cycle clock to get the next value
    if (i != 8){
      temperatureValue = temperatureValue | (dqValue << i);   //Every iteration, a new value is shifted in
    }
  } 
  pinMode(dqPIN, OUTPUT);      //DQ pin mode set to output again, so data can be output  

  digitalWrite(resetPIN, LOW);    //Return reset pin low after command is sent

  return temperatureValue;
}

//This function extracts the Count_Remain
//from the DS1620 according to the datasheet
//to get higher resolution temp
//http://datasheets.maximintegrated.com/en/ds/DS1620.pdf
int getCount_Remain(byte clkPIN, byte dqPIN, byte resetPIN) {
  
  int i;
  int dqValue = 0;
  int temperatureValue = 0;

  //Request the temperature value just produced
  //Reset and Clock must be driven high before a command is sent
  digitalWrite(clkPIN, HIGH);
  digitalWrite(resetPIN, HIGH);
  sendCommand(0xA0, clkPIN, dqPIN);    //Command to read counter
  
  pinMode(dqPIN, INPUT);      //DQ pin must be set high in order to operate as an input
  
  //This for loop reads 9 bits one by one. It shifts them 
  //into the temperatureValue integer on each iteration
  for(i = 0; i < 9; i++) {
    digitalWrite(clkPIN, LOW);      //Clock low to read first 'bit'
    dqValue = digitalRead(dqPIN);   //Store current value on dq pin
    digitalWrite(clkPIN, HIGH);     //Cycle clock to get the next value
    temperatureValue = temperatureValue | (dqValue << i);   //Every iteration, a new value is shifted in
  } 
  pinMode(dqPIN, OUTPUT);      //DQ pin mode set to output again, so data can be output  

  digitalWrite(resetPIN, LOW);    //Return reset pin low after command is sent
  
  return temperatureValue;

}

//This function extracts the Count_Per_C
//from the DS1620 according to the datasheet
//to get higher resolution temp
//http://datasheets.maximintegrated.com/en/ds/DS1620.pdf
int getCount_Per_C(byte clkPIN, byte dqPIN, byte resetPIN) {
  
  int i;
  int dqValue = 0;
  int temperatureValue = 0;

  //Request the temperature value just produced
  //Reset and Clock must be driven high before a command is sent
  digitalWrite(clkPIN, HIGH);
  digitalWrite(resetPIN, HIGH);
  sendCommand(0xA9, clkPIN, dqPIN);    //Command to read slope
  
  pinMode(dqPIN, INPUT);      //DQ pin must be set high in order to operate as an input
  
  //This for loop reads 9 bits one by one. It shifts them 
  //into the temperatureValue integer on each iteration
  for(i = 0; i < 9; i++) {
    digitalWrite(clkPIN, LOW);      //Clock low to read first 'bit'
    dqValue = digitalRead(dqPIN);   //Store current value on dq pin
    digitalWrite(clkPIN, HIGH);     //Cycle clock to get the next value
    temperatureValue = temperatureValue | (dqValue << i);   //Every iteration, a new value is shifted in
  } 
  pinMode(dqPIN, OUTPUT);      //DQ pin mode set to output again, so data can be output  

  digitalWrite(resetPIN, LOW);    //Return reset pin low after command is sent
  
  return temperatureValue;

}


void Initiate_Temp_Chips(){
  /*---INTERNAL sensor---*/
  //Set pin modes to outputs
//  pinMode(DQ_INTERNAL_PIN, OUTPUT);
//  pinMode(CLK_INTERNAL_PIN, OUTPUT);
//  pinMode(RESET_INTERNAL_PIN, OUTPUT);
//  
//  //Configuring the DS1620 for use - see datasheet fig 4
//  digitalWrite(RESET_INTERNAL_PIN, LOW);
//  digitalWrite(CLK_INTERNAL_PIN, HIGH); 

  /*---EXTERNAL sensor---*/
  //Set pin modes to outputs
  pinMode(DQ_EXTERNAL_PIN, OUTPUT);
  pinMode(CLK_EXTERNAL_PIN, OUTPUT);
  pinMode(RESET_EXTERNAL_PIN, OUTPUT);
  
  //Configuring the DS1620 for use - see datasheet fig 4
  digitalWrite(RESET_EXTERNAL_PIN, LOW);
  digitalWrite(CLK_EXTERNAL_PIN, HIGH); 
}

