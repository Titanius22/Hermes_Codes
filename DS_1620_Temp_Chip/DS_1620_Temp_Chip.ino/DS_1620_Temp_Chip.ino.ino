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

// Define all the pins used in program
#define dqPIN 10
#define clkPIN 11
#define resetPIN 12

//This function sends the command to the 
//chip, bit by bit across the DQ Pin
void sendCommand (int cmd) {
  int i;
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

//This function extracts the temperature
//value from the DS1620 once it has been
//requested
float getTemperatureValue() {
  
  int i;
  int dqValue = 0;
  int temperatureValue = 0;
  
  //Request the temperature value just produced
  //Reset and Clock must be driven high before a command is sent
  digitalWrite(clkPIN, HIGH);
  digitalWrite(resetPIN, HIGH);
  sendCommand(0xAA);    //Command to request temperature value
  
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
  
  //Checking if the 9th bit is a 1 or 0. 
  //1 means the temperature is negative
  //0 means the temperature is positive
  
  if (dqValue == 0) {    //(the 9th bit) it is a positive temperature
    //Value is in 1/2 degree increments
    //So must be divided in half to get the proper value
    return (temperatureValue/2.0);    
  }
  else {    //It is a negative temperature
   //This is a 2's complement value 
   //Convert to decimal then subtract (2^numofbits) 
   //(2^9) = 512
    return (((temperatureValue - 512)/2.0) * -1.0);    
  } 

}

//This function extracts the temperature
//value from the DS1620 once it has been
//requested
///http://datasheets.maximintegrated.com/en/ds/DS1620.pdf
int getTemp_Read() {
  
  int i;
  int dqValue = 0;
  int temperatureValue = 0;
  
  //Request the temperature value just produced
  //Reset and Clock must be driven high before a command is sent
  digitalWrite(clkPIN, HIGH);
  digitalWrite(resetPIN, HIGH);
  sendCommand(0xAA);    //Command to request temperature value
  
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

//This function extracts the temperature
//value from the DS1620 once it has been
//requested
///http://datasheets.maximintegrated.com/en/ds/DS1620.pdf
int getCount_Remain() {
  
  int i;
  int dqValue = 0;
  int temperatureValue = 0;

  //Request the temperature value just produced
  //Reset and Clock must be driven high before a command is sent
  digitalWrite(clkPIN, HIGH);
  digitalWrite(resetPIN, HIGH);
  sendCommand(0xA0);    //Command to read counter
  
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

//This function extracts the temperature
//value from the DS1620 once it has been
//requested
///http://datasheets.maximintegrated.com/en/ds/DS1620.pdf
int getCount_Per_C() {
  
  int i;
  int dqValue = 0;
  int temperatureValue = 0;

  //Request the temperature value just produced
  //Reset and Clock must be driven high before a command is sent
  digitalWrite(clkPIN, HIGH);
  digitalWrite(resetPIN, HIGH);
  sendCommand(0xA9);    //Command to read slope
  
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

void setup() {
  //Set pin modes to outputs
  pinMode(dqPIN, OUTPUT);
  pinMode(clkPIN, OUTPUT);
  pinMode(resetPIN, OUTPUT);
  
  //Configuring the DS1620 for use - see datasheet fig 4
  digitalWrite(resetPIN, LOW);
  digitalWrite(clkPIN, HIGH);  
  
  Serial.begin(9600);
}

void loop () {
  
  double Temp_Read = getTemp_Read();
  double Count_Remain = getCount_Remain();
  double Count_Per_C = getCount_Per_C();
  double Temperature = ((Temp_Read/2) - 0.25) + ((Count_Per_C-Count_Remain)/Count_Per_C);
  int steps = Temperature/0.0625;
  
  //Serial.print("Temperature = ");
  //Serial.print(Temperature, 1);
  //Serial.print(" Celcius \n");
//  Serial.print(Temp_Read);
//  Serial.print("    ");
//  Serial.print(Count_Remain);
//  Serial.print("    ");
//  Serial.print(Count_Per_C);
//  Serial.print("    ");
//  Serial.print(Temperature,4);
//  Serial.print("    ");
//  Serial.println(steps,4);


  float currentTemperature = getTemperatureValue();
  
  Serial.print("Temperature = ");
  Serial.print(currentTemperature, 1);
  Serial.print(" Celcius \n");
  
  //Waiting 5 seconds to get next reading
  delay(1000);
  
  
}
