//http://www.thegeekstuff.com/2011/12/c-socket-programming/?utm_source=feedburner
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>

unsigned long getIntFromByte(unsigned char** , short );

// The slave Arduino address
#define ADDRESS 0x04

// The I2C bus: This is for V2 pi's. For V1 Model B you need i2c-0
static const char *devName = "/dev/i2c-1";

int main(int argc, char *argv[])
{
    char recvBuff[100];
	struct timespec req={0},rem={0};
	req.tv_nsec = 500000000; //500ms
	
	// I2C STUFF. setting up i2c for communication
	printf("I2C: Connecting\n");
	int i2cFile;

	if ((i2cFile = open(devName, O_RDWR)) < 0) {
		fprintf(stderr, "I2C: Failed to access %d\n", devName);
		exit(1);
	}

	printf("I2C: acquiring buss to 0x%x\n", ADDRESS);

	if (ioctl(i2cFile, I2C_SLAVE, ADDRESS) < 0) {
		fprintf(stderr, "I2C: Failed to acquire bus access/talk to slave 0x%x\n", ADDRESS);
		exit(1);
	}

	unsigned char* writeArray;
	unsigned char** wrPtr;
	char command[30];
	
	unsigned int DataLineCounter;
	unsigned long DataGPS[3]; // Longitude, Latitude, Altitude
	unsigned int DataSensors[8]; // External Thermistor, Battery Voltage, Magnotometer X, Y, Z, Humidity, Pressure, Internal Temperature.
	char DataEndLine[3];
	
	//set sleep duration
	req.tv_nsec = 500000000; //500ms
	
	while(1){ 

		printf("\n\nWorking Bro!!!\n");
		
		if(read(i2cFile, recvBuff, 32) == 32){
		
			writeArray=recvBuff;
			wrPtr=&writeArray;

			DataLineCounter = (unsigned short)getIntFromByte(wrPtr,2);
			printf("%d ", DataLineCounter); // Line Counter
	  
			DataGPS[0] = (unsigned int)getIntFromByte(wrPtr,3);
			printf("%d ", DataGPS[0]); // Longitude
	  
			DataGPS[1] = (unsigned int)getIntFromByte(wrPtr,3);
			printf("%d ", DataGPS[1]); // Latitude

			DataGPS[2] = (unsigned int)getIntFromByte(wrPtr,3);
			printf("%d ", DataGPS[2]); // Altitude
			
			DataGPS[3] = (unsigned int)getIntFromByte(wrPtr,2);
			printf("%d ", DataGPS[3]); // Seconds since half UTC day

			DataSensors[0] = (unsigned int)getIntFromByte(wrPtr,2);
			printf("%d ", DataSensors[0]); // External Thermistor
	  
			DataSensors[1] = (unsigned int)getIntFromByte(wrPtr,1);
			printf("%d ", DataSensors[1]); // Battery Voltage
			
			DataSensors[2] = (unsigned int)getIntFromByte(wrPtr,1);
			printf("%d ", DataSensors[2]); // Battery Current
			
			DataSensors[3] = (unsigned int)getIntFromByte(wrPtr,1);
			printf("%d ", DataSensors[3]); // Magnotometer X

			DataSensors[4] = (unsigned int)getIntFromByte(wrPtr,1);
			printf("%d ", DataSensors[4]); // Magnotometer Y

			DataSensors[5] = (unsigned int)getIntFromByte(wrPtr,1);
			printf("%d ", DataSensors[5]); // Magnotometer Z

			DataSensors[6] = (unsigned int)getIntFromByte(wrPtr,1);
			printf("%d ", DataSensors[6]); // Humidity

			DataSensors[7] = (unsigned int)getIntFromByte(wrPtr,3);
			printf("%d ", DataSensors[7]); // Pressure

			DataSensors[8] = (unsigned int)getIntFromByte(wrPtr,2);
			printf("%d ", DataSensors[8]); // Internal Temperature
			
			DataEndLine[0] = (char)getIntFromByte(wrPtr,1);
			printf("%c", DataEndLine[0]); // 'E'

			DataEndLine[1] = (char)getIntFromByte(wrPtr,1);
			printf("%c", DataEndLine[1]); // 'N'

			DataEndLine[2] = (char)getIntFromByte(wrPtr,1);
			printf("%c\n", DataEndLine[2]); // 'D'
		}

		//delay
		nanosleep(&req,&rem);
	}

    return 0;
}

		
unsigned long getIntFromByte(unsigned char** arrayStart, short bytes){

  //Allocating array to read into
  char* intPtr = malloc (sizeof(unsigned long));
  unsigned long temp;
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
  temp=*((unsigned long*)intPtr);
  free(intPtr);
  //Returning void pointer (Pointer to an integer with the designated of the number of bytes)
  return temp;
}