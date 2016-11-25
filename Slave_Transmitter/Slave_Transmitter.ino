int temperature = 0;
int pressure;
int humidity;
int latitude;
int longitude;
int altitude;
int i;
char StrToSend[10];


#include <Wire.h>
#include <String.h>
// CURRENTLY Wire Slave Sender
void setup() {
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onRequest(requestEvent); // register event
}

void loop() {
  UpdateTemp();
  delay(500);

}

void requestEvent(int howMany) { 
  char fullSendSentance[36];
  i = i + 1; 
  sprintf(fullSendSentance,"%04d%04d",i,temperature);
  Wire.write(fullSendSentance);
}

void UpdateTemp() { 
  temperature = 1111;//temperature + 23;
}
