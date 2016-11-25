/*
 SimpleSat Rotor Control Program  -  73 de W9KE Tom Doyle
 January 2012
 
 Written for Arduino 1.0
 
 This program was written for the Arduino boards. It has been tested on the
 Arduino UNO and Mega2560 boards. 
 
 Pin 7 on the Arduino is used as a serial tx line. It is connected to a Parallax 
 27977 2 X 16 backlit serial LCD display - 9600 baud. WWW.Parallax.com
 It is not required but highly recommended. You might want to order a
 805-00011 10-inch Extension Cable with 3-pin Header at the same time.
 The first row on the display will display the current rotor azimuth on
 the left hand side of the line. When the azimuth rotor is in motion 
 a L(eft) or R(ight) along with the new azimuth received from the tracking 
 program is displayed on the right side of the line. The second line will do
 the same thing for the elevation with a U(p) or D(own) indicating the 
 direction of motion.
 
 The Arduino usb port is set to 9600 baud and is used for receiving
 data from the tracking program in GS232 format.
 In SatPC32 set the rotor interface to Yaesu_GS-232.
 
 These pin assignments can be changed
 by changting the assignment statements below.
 G-5500 analog azimuth to Arduino pin A0
 G-5500 analog elevation to Arduino pin A1
 Use a small signal transistor switch or small reed relay for these connections
 G-5500 elevation rotor up to Arduino pin 8
 G-5500 elevation rotor down to Arduino pin 9
 G-5500 azimuth rotor left to Arduino pin 10
 G-5500 azimuth rotor right to Arduino pin 11
 
 The Arduino resets when a connection is established between the computer
 and the rotor controller. This is a characteristic of the board. It makes
 programming the chip easier. It is not a problem but is something you
 should be aware of.
 
 The program is set up for use with a Yaesu G-5500 rotor which has a max
 azimuth of 450 degrees and a max elevation of 180 degrees. The controller
 will accept headings within this range. If you wish to limit the rotation
 to 360 and/or limit the elevation to 90 set up SatPC32 to limit the rotation
 in the rotor setup menu. You should not have to change the rotor controller.
 
            - For additional information check -
 
      http://www.tomdoyle.org/SimpleSatRotorController/ 
*/

/* 
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
    INCLUDING BUT NOT LIMITED TO THE'WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
    PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 'COPYRIGHT HOLDERS BE 
    LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
    OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/ 

// ------------------------------------------------------------
// ---------- you may wish to adjust these values -------------
// ------------------------------------------------------------

// A/D converter parameters 
/*
   AFTER you have adjusted your G-5500 control box as per the manual
   adjust the next 4 parameters. The settings interact a bit so you may have
   to go back and forth a few times. Remember the G-5500 rotors are not all that
   accurate (within 4 degrees at best) so try not to get too compulsive when 
   making these adjustments. 
*/

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "Wire.h"
#include <SoftwareSerial.h> // use software uart library

#define SLAVE_ADDRESS 0x04
int i;

const long _azAdZeroOffset   =   0;//325;//-3;   // adjust to zero out lcd az reading when control box az = 0 
const long _elAdZeroOffset   =   0;//7;   // adjust to zero out lcd el reading when control box el = 0

/*  
    10 bit A/D converters in the Arduino have a max value of 1023
    for the azimuth the A/D value of 1023 should correspond to 450 degrees
    for the elevation the A/D value of 1023 should correspond to 180 degrees
    integer math is used so the scale value is multiplied by 100 to maintain accuracy
    the scale factor should be 100 * (1023 / 450) for the azimuth
    the scale factor should be 100 * (1023 / 180) for the elevation    
*/

const long _azScaleFactor =  232;  //  adjust as needed
const long _elScaleFactor =  568;  //  adjust as needed 
/////
const long _dispazScaleFactor =  500;  //  adjust as needed
const long _dispelScaleFactor =  500;  //  adjust as needed 

// pins
const byte _elevationInputPin = A0; // elevation analog signal from G5500
const byte _azimuthInputPin = A1;   // azimuth analog signal from G5500
const byte _G5500UpPin = 8;        // elevation rotor up control line
const byte _G5500DownPin = 9;      // elevation rotor down control line
const byte _G5500LeftPin = 10;      // azimuth rotor left control line
const byte _G5500RightPin = 12;     // azimuth rotor right control line

const byte _LcdTxPin = 7;          // software uart lcd tx pin
const byte _LcdRxPin = 6;          // software uart lcd rx pin - pin not used

// take care if you lower this value -  wear or dirt on the pots in your rotors
// or A/D converter jitter may cause hunting if the value is too low. 
long _closeEnough = 100;   // tolerance for az-el match in rotor move in degrees * 100

// ------------------------------------------------------------
// ------ values from here down should not need adjusting -----
// ------------------------------------------------------------

// rotor
const long _maxRotorAzimuth = 45000L;  // maximum rotor azimuth in degrees * 100
const long _maxRotorElevation = 18000L; // maximum rotor elevation in degrees * 100

long _rotorAzimuth = 0L;       // current rotor azimuth in degrees * 100
long _rotorElevation = 0L;     // current rotor azimuth in degrees * 100
long _azimuthTemp = 0L;        // used for gs232 azimuth decoding
long _elevationTemp = 0L;      // used for gs232 elevation decoding  
long _newAzimuth = 0L;         // new azimuth for rotor move
long _newElevation = 0L;       // new elevation for rotor move
long _previousRotorAzimuth = 0L;       // previous rotor azimuth in degrees * 100
long _previousRotorElevation = 0L;     // previous rotor azimuth in degrees * 100

unsigned long _rtcLastDisplayUpdate = 0UL;      // rtc at start of last loop
unsigned long _rtcLastRotorUpdate = 0UL;        // rtc at start of last loop
unsigned long _displayUpdateInterval = 500UL;   // display update interval in mS
unsigned long _rotorMoveUpdateInterval = 100UL; // rotor move check interval in mS

boolean _gs232WActice = false;  // gs232 W command in process
boolean _AZjustmoved = false;  // gs232 W command in process
boolean _ELjustmoved = false;  // gs232 W command in process
int _gs232AzElIndex = 0;        // position in gs232 Az El sequence
long _gs232Azimuth = 0;          // gs232 Azimuth value
long _gs232Elevation = 0;        // gs232 Elevation value
boolean _azimuthMove = false;     // azimuth move needed
boolean _elevationMove = false;   // elevation move needed

String azRotorMovement;   // string for az rotor move display
String elRotorMovement;   // string for el rotor move display

// create instance of NewSoftSerial 
SoftwareSerial lcdSerial =  SoftwareSerial(_LcdRxPin, _LcdTxPin);

//Values for calculations
const float Re = 3956.0; //[miles]
const float pi = 3.14159265359; 
float distancefromballoontoGS = 0;   
                                //ICI//COA//KENNEL CLUB//TEST
float balloonLat              = 29.191585;//29.187366;//29.166656;//34.28889;//[degrees]
float balloonLon              = 81.046269;//81.049893;//81.080002;//117.6458;//[degrees]
float balloonAlt              = 0;//10064.0/5280.0;//[miles]
//Hard coded for ERAU Daytona Beach Campus;//Test values
float groundStationlat        = 29.188330;//34.22389;//[degrees]
float groundStationlon        = 81.048108;//118.0603;//[degrees]
float groundStationAlt        = 0;//5710.0/5280.0;//[miles]
float d; 
float Azimuth; 
float Elevation; 
String command = "";

int j;

//
// run once at reset
//
void setup()
{
    // initialize rotor control pins as outputs
    pinMode(_G5500UpPin, OUTPUT);
    pinMode(_G5500DownPin, OUTPUT);
    pinMode(_G5500LeftPin, OUTPUT);
    pinMode(_G5500RightPin, OUTPUT);
    // set all the rotor control outputs low
    digitalWrite(_G5500UpPin, LOW);
    digitalWrite(_G5500DownPin, LOW);
    digitalWrite(_G5500LeftPin, LOW);
    digitalWrite(_G5500RightPin, LOW);
    // initialize serial ports:
    Serial.begin(9600);  // control

    // set up rotor lcd display values
    readAzimuth(); // get current azimuth from G-5500
    _previousRotorAzimuth = _rotorAzimuth + 1000;
    readElevation(); // get current elevation from G-5500
    _previousRotorElevation = _rotorElevation + 1000; 
    delay(1000);

    Serial.print("Initial GS AZ: ");
    Serial.println(_rotorAzimuth/100);
    Serial.print("Initial GS EL: ");
    Serial.println(_rotorElevation/100); 
    Serial.println("");

    //I2C
    pinMode(13, OUTPUT);
    // initialize i2c as slave
    Wire.begin(SLAVE_ADDRESS);
    // define callbacks for i2c communication
    Wire.onReceive(receiveData);
    //Wire.onRequest(sendData);
}


//
// main program loop
//
void loop() 
{
    if (Serial.available() > 0){
      String temp=Serial.readString();
      command = CreateCommand(balloonLon, balloonLat, balloonAlt, groundStationlon, groundStationlat, groundStationAlt);//getCommand(temp.charAt(0));
      /////////////////////////////////////////
      command = "W000 000";                 ///
      /////////////////////////////////////////
      Serial.println(command);
      for (i=0; i<=command.length(); i++){
        decodeGS232(command.charAt(i));
        Serial.print(command.charAt(i));
      }
      Serial.println(""); 
    }
    unsigned long rtcCurrent = millis(); // get current rtc value
   
    // check for rtc overflow - skip this cycle if overflow
    if (rtcCurrent > _rtcLastDisplayUpdate){ // overflow if not true    _rotorMoveUpdateInterval
      // update rotor movement if necessary
      if (rtcCurrent - _rtcLastRotorUpdate > _rotorMoveUpdateInterval){
         _rtcLastRotorUpdate = rtcCurrent; // reset rotor move timer base
         
         // AZIMUTH       
         readAzimuth(); // get current azimuth from G-5500
         if ( (abs(_rotorAzimuth - _newAzimuth) > _closeEnough) && _azimuthMove ) { // see if azimuth move is required
            updateAzimuthMove();
            readAzimuth();
            Serial.print("Ground Station AZ: ");
            Serial.println(_rotorAzimuth/100);
            _AZjustmoved = true;
         }
        else{  // no move required - turn off azimuth rotor
           digitalWrite(_G5500LeftPin, LOW);
           digitalWrite(_G5500RightPin, LOW);
           _azimuthMove = false;
           azRotorMovement = "        ";
           if (_AZjustmoved == true){
              delay(1000);
              Serial.println("");
              Serial.print("Initial Command Given: ");
              Serial.println(command);
              Serial.print("Final Ground Station AZ: ");
              Serial.println(_rotorAzimuth/100);
              _AZjustmoved = false;
           }
         }
         
         // ELEVATION       
         readElevation(); // get current elevation from G-5500
         // see if aelevation move is required
         if ( abs(_rotorElevation - _newElevation) > _closeEnough && _elevationMove ){ // move required{
            updateElevationMove();
            readElevation();
            Serial.print("Ground Station EL: ");
            Serial.println(_rotorElevation/100); 
            Serial.println(""); 
         }
        else{  // no move required - turn off elevation rotor
            digitalWrite(_G5500UpPin, LOW);
            digitalWrite(_G5500DownPin, LOW);
            _elevationMove = false;
            elRotorMovement = "        ";
            
            if (_ELjustmoved == true){
              delay(1000);
              Serial.println("");
              Serial.print("Initial Command Given: ");
              Serial.println(command);
              Serial.print("Final Ground Station EL: ");
              Serial.println(_rotorElevation/100);
              _ELjustmoved = false;
           }
         }            
      } // end of update rotor move
   }
}


//MAIN LOOP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//FUNCTIONS


// callback for received data
void receiveData(String data){
 int c, valueindex;
 String temp;
 float lat, lon, el;
 while(Wire.available()) {
  data = Wire.read(); /////////////////////////////////////
 }
 valueindex = 1;
  for (c=2; c<=data.length(); c++){
    if(data.charAt(1)=='1'){/////////////////////for GS values
      if(data.charAt(c)==' '){
        if(valueindex=='1'){groundStationlat = temp.toFloat();}
        else if(valueindex=='2'){groundStationlon = temp.toFloat();}
        else if(valueindex=='3'){groundStationAlt = temp.toFloat();}
        temp = "";
        valueindex = valueindex + 1;
      }
      temp = temp + data.charAt(c);
    }else if(data.charAt(1)=="2") {/////////////////////for balloon values
      if(data.charAt(c)==' '){
        if(valueindex=='1'){balloonLat = temp.toFloat();}
        else if(valueindex=='2'){balloonLon = temp.toFloat();}
        else if(valueindex=='3'){balloonAlt = temp.toFloat();}
        temp = "";
        valueindex = valueindex + 1;
      }
      temp = temp + data.charAt(c);
    }else if(data.charAt(1)=="3") {/////////////////////for balloon values
      sendData(data);
    }
  }
}


// callback for sending data
void sendData(String data){
 //String data;
 data = String(_rotorAzimuth/100) + " " + String(_rotorElevation/100);
 Wire.write(data);
}


// update elevation rotor move
//
void updateElevationMove()
{          
   // calculate rotor move 
   long rotorMoveEl = _newElevation - _rotorElevation;
   if (rotorMoveEl > 0){
      elRotorMovement = "  U ";
      elRotorMovement = elRotorMovement + String(_newElevation / 100);
      digitalWrite(_G5500DownPin, LOW);
      digitalWrite(_G5500UpPin, HIGH);      
   }
   else{           
     if (rotorMoveEl < 0){
       elRotorMovement = "  D ";
       elRotorMovement = elRotorMovement + String(_newElevation / 100);
       digitalWrite(_G5500UpPin, LOW);
       digitalWrite(_G5500DownPin, HIGH);       
     } 
   } 
 }


//
// update azimuth rotor move
//
void updateAzimuthMove()
{          
     // calculate rotor move 
     long rotorMoveAz = _newAzimuth - _rotorAzimuth;
     // adjust move if necessary
     if (rotorMoveAz > 18000){ 
        rotorMoveAz = rotorMoveAz - 180; // adjust move if > 180 degrees
     }
     else{           
       if (rotorMoveAz < -18000){ 
         rotorMoveAz = rotorMoveAz + 18000; // adjust move if < -180 degrees
       }
     }
     
     if (rotorMoveAz > 0){
        azRotorMovement = "  R ";
        azRotorMovement = azRotorMovement + String(_newAzimuth / 100);
        digitalWrite(_G5500LeftPin, LOW);
        digitalWrite(_G5500RightPin, HIGH);        
     }
     else{           
       if (rotorMoveAz < 0){
         azRotorMovement = "  L ";
         azRotorMovement = azRotorMovement + String(_newAzimuth / 100);
         digitalWrite(_G5500RightPin, LOW); 
         digitalWrite(_G5500LeftPin, HIGH);         
      } 
   }            
}


//
// read azimuth from G5500
//
void readElevation()
{
   long sensorValue = analogRead(_elevationInputPin);
   _rotorElevation = ((sensorValue * 10000) / _elScaleFactor) - _elAdZeroOffset;
}


//
// read azimuth from G5500
//
void readAzimuth()
{
  long sensorValue = analogRead(_azimuthInputPin);
  _rotorAzimuth = ((sensorValue * 10000) / _azScaleFactor) - _azAdZeroOffset;
}


//
// decode gs232 commands
//
void decodeGS232(char character)
{
    switch (character){
       case 'w':  // gs232 W command
       case 'W':
       {
          {
            _gs232WActice = true;
            _gs232AzElIndex = 0;
          }
          break;
       }
       
       // numeric - azimuth and elevation digits
       case '0':  case '1':   case '2':  case '3':  case '4': 
       case '5':  case '6':   case '7':  case '8':  case '9':
       {
          if ( _gs232WActice){
            processAzElNumeric(character);          
          }
       }   
       
       default:{
          // ignore everything else
       }
     }
}


//
// process az el numeric characters from gs232 W command
//
void processAzElNumeric(char character)
{
      switch(_gs232AzElIndex){
         case 0:{ // first azimuth character
            _azimuthTemp =(character - 48) * 100;
            _gs232AzElIndex++;
            break;
        } 
        case 1:{
            _azimuthTemp = _azimuthTemp + (character - 48) * 10;
            _gs232AzElIndex++;
            break;
        } 
        case 2:{ // final azimuth character
            _azimuthTemp = _azimuthTemp + (character - 48);
            _gs232AzElIndex++;
            
            // check for valid azimuth 
            if ((_azimuthTemp * 100) > _maxRotorAzimuth){
              _gs232WActice = false;
              _newAzimuth = 0L;
              _newElevation = 0L;
            }           
            break;
        }  
        case 3:{ // first elevation character
            _elevationTemp =(character - 48) * 100;
            _gs232AzElIndex++;
            break;
        } 
        case 4:{
            _elevationTemp = _elevationTemp + (character - 48) * 10;
            _gs232AzElIndex++;
            break;
        } 
        case 5:{ // last elevation character
            _elevationTemp = _elevationTemp + (character - 48);
            _gs232AzElIndex++;
            
            // check for valid elevation 
            if ((_elevationTemp * 100) > _maxRotorElevation){
              _gs232WActice = false;
              _newAzimuth = 0L;
              _newElevation = 0L;
            }
            else{ // both azimuth and elevation are ok
              // set up for rotor move
              _newAzimuth = _azimuthTemp * 100;
              _newElevation = _elevationTemp * 100;
              _azimuthMove = true;
              _elevationMove = true;
            }            
            break;
        }             
        
        default:{
           // should never get here
        }         
    } 
}


// calculates distance along the surface of the Earth between the points at sea level below each of the points
float findDistance(float balloonLon, float balloonLat, float groundStationlon, float groundStationlat) {
  float dlon, dlat, a, c, d;  // ground station = 1  // balloon = 2
  dlon = balloonLon - groundStationlon;
  dlat = balloonLat - groundStationlat;
  a = pow((sin(dlat/2)),2) + cos(groundStationlat) * cos(balloonLat) * pow((sin(dlon/2)),2);
  c = 2 * asin(min(1,sqrt(a)));
  d = Re * c;
  return d;
}
// calculates Azimuth from ground station to balloon
float findAzimuth(float balloonLon, float balloonLat, float groundStationlon, float groundStationlat, float d) {
  float x;  // ground station = 1  // balloon = 2
  x = acos( (sin(balloonLat) - sin(groundStationlat)*cos(d/Re)) / (sin(d/Re)*cos(groundStationlat)) );
  if(sin(balloonLon-groundStationlon) > 0){
    x = (2*pi)-x;
  }
  return x;
}
// calculates Elevation from ground station to balloon
float findElevation(float balloonAlt, float groundStationAlt, float d) {
  float el;  // ground station = 1  // balloon = 2
  el = asin(((balloonAlt - groundStationAlt)/d) - (d/(2.0*Re)));
  return el;
}

String CreateCommand(float balloonLon, float balloonLat, float balloonAlt, float groundStationlon, float groundStationlat, float groundStationAlt){
  String command;
  float d, Azimuth, Elevation;
  
  balloonLat = (balloonLat*pi)/180;              //[radian]
  balloonLon = (balloonLon*pi)/180;              //[radian]
  groundStationlat = (groundStationlat*pi)/180;  //[radian]
  groundStationlon = (groundStationlon*pi)/180;  //[radian]
      
  d = findDistance(balloonLon, balloonLat, groundStationlon, groundStationlat);
      
  Azimuth = findAzimuth(balloonLon, balloonLat, groundStationlon, groundStationlat, d);
  Azimuth = (Azimuth*180)/pi;
      
  Elevation = findElevation(balloonAlt, groundStationAlt, d);
  Elevation = (Elevation*180)/pi;
  
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
  return command;
}

