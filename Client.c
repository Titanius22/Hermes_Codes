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

//prototyping
unsigned long getIntFromByte(unsigned char** , short);
void insertBytesFromInt(void* ,unsigned char** , short);
int tryNewSocketConnection();

//Globals
int ServerFileNum;
struct sockaddr_in serv_addr;
int startingSocketNum = 5000;
short madeConnection = 0; //becomes true when connection is made. If connection is lost afterwards (meaning when madeConnection is true), the port number is incremented and madeConnection is set to false till another connection is found.

// The slave Arduino address
#define ADDRESS 0x04

// The I2C bus: This is for V2 pi's. For V1 Model B you need i2c-0
static const char *devName = "/dev/i2c-1";

int main(int argc, char *argv[])
{
    char recvBuff[200];
	int n = 0;
	struct timespec req={0},rem={0};
	req.tv_nsec = 500000000; //500ms

    /////For when the ip address was a second argument
	// Checks that command is correct
	//if(argc != 2)
    //{
    //    printf("\n Usage: %s <ip of server> \n",argv[0]);
    //    return 1;
    //}
	
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

	int fileCount = 1;
	//int packetCount = 1;
	int fileLineCount = 1;
	char fileCounter[8];
	char filepath[] =  "/home/alarm/randomJunk/cCodeTests/DistanceTest/";
	char fileName[] = "DistanceTest";
	//char mostFilePath[] = "/home/alarm/randomJunk/cCodeTests/";
	char fileExt[] = ".txt";
	char fullFilePath[80];
	FILE *filePointer;
	char leftOvers[25];
	char* writeArray;
	char** wrPtr;
	char command[25];
	
	unsigned int DataLineCounter;
	unsigned long DataGPS[3]; // Longitude, Latitude, Altitude
	unsigned int DataSensors[8]; // External Thermistor, Battery Voltage, Magnotometer X, Y, Z, Humidity, Pressure, Internal Temperature.
	char DataEndLine[3];
	
	while(1){ 
		
		// If connection was made properly
		if(tryNewSocketConnection() == 0){
		
		
			//while ( (n = recv(ServerFileNum, recvBuff, 32 , 0)) > 0) same as read if last argument is 0
			while ( (n = read(ServerFileNum, recvBuff, 32)) > 0)
			{
				
				// At the start of every new "page", it creates and opens a new file
				if (fileLineCount == 1){
					sprintf(fileCounter, "%04d", fileCount);
				
					strcpy(fullFilePath, filepath);
					strcat(fullFilePath, fileName);
					//strcpy(fullFilePath, mostFilePath);
					strcat(fullFilePath, fileCounter);
					strcat(fullFilePath, fileExt);
					
					filePointer = fopen(fullFilePath, "a");
				}
				
				// Writes data to the document unconverted
				if (filePointer != NULL)
				{
					fwrite(recvBuff, n, 1, filePointer);
				}
				
				
				recvBuff[n] = 0;
				
				
				// Every 20 lines
				if(fileLineCount == 80){
					
					char* writeArray=recvBuff;
					char** wrPtr=&writeArray;
					
					DataLineCounter = (unsigned int)getIntFromByte(wrPtr,3);
					printf("%d ", DataLineCounter); // Line Counter
			  
					DataGPS[0] = (unsigned long)getIntFromByte(wrPtr,5);
					printf("%lu ", DataGPS[0]); // Longitude
			  
					DataGPS[1] = (unsigned long)getIntFromByte(wrPtr,5);
					printf("%lu ", DataGPS[1]); // Latitude

					DataGPS[2] = (unsigned int)getIntFromByte(wrPtr,3);
					printf("%d ", DataGPS[2]); // Altitude

					DataSensors[0] = (unsigned int)getIntFromByte(wrPtr,2);
					printf("%d ", DataSensors[0]); // External Thermistor
			  
					DataSensors[1] = (unsigned int)getIntFromByte(wrPtr,1);
					printf("%d ", DataSensors[1]); // Battery Voltage

					DataSensors[2] = (unsigned int)getIntFromByte(wrPtr,1);
					printf("%d ", DataSensors[2]); // Magnotometer X

					DataSensors[3] = (unsigned int)getIntFromByte(wrPtr,1);
					printf("%d ", DataSensors[3]); // Magnotometer Y

					DataSensors[4] = (unsigned int)getIntFromByte(wrPtr,1);
					printf("%d ", DataSensors[4]); // Magnotometer Z

					DataSensors[5] = (unsigned int)getIntFromByte(wrPtr,1);
					printf("%d ", DataSensors[5]); // Humidity

					DataSensors[6] = (unsigned int)getIntFromByte(wrPtr,4);
					printf("%d ", DataSensors[6]); // Pressure

					DataSensors[7] = (unsigned int)getIntFromByte(wrPtr,2);
					printf("%d ", DataSensors[7]); // Internal Temperature
					
					DataEndLine[0] = (char)getIntFromByte(wrPtr,1);
					printf("%c", DataEndLine[0]); // 'E'

					DataEndLine[1] = (char)getIntFromByte(wrPtr,1);
					printf("%c", DataEndLine[1]); // 'N'

					DataEndLine[2] = (char)getIntFromByte(wrPtr,1);
					printf("%c\n", DataEndLine[2]); // 'D'
					
					// Send data over I2C
					sprintf(command, "1 %lu %lu %d ", DataGPS[0], DataGPS[1], DataGPS[2]);
					write(i2cFile, command, strlen(command));
					
					// File tracking and counting
					fileLineCount = 0;
					fileCount++;
					fclose(filePointer);
				}
				fileLineCount++;
				//packetCount++;
			}
			fclose(filePointer);
		}
		
		//delay
		nanosleep(&req,&rem);
	}

    if(n < 0)
    {
        printf("\n Read error \n");
    } 

    return 0;
}

//SERVER STUFF. setting up socket
int tryNewSocketConnection(){
	
	if (madeConnection == 1){
		startingSocketNum++;
		madeConnection = 0;
	}
	
	// Makes varible free
	//////////////////////////////////////////////////////////////////////////serv_addr = {0};
	
	if((ServerFileNum = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(startingSocketNum); 

    // The arv[1] was originally the first trminal argument which was the ip address 
	//if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
	if(inet_pton(AF_INET, "10.1.1.232", &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return -1;
    }
	
	if( connect(ServerFileNum, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\n Error : Connect Failed \n");
		return -1;
	}
	
	madeConnection = 1;
	
	return 0;
}
		
unsigned long getIntFromByte(unsigned char** arrayStart, short bytes){

  //Allocating array to read into
  unsigned char* intPtr = malloc (sizeof(unsigned long));
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

void insertBytesFromInt(void* value,unsigned char** byteStart, short numberBytesToCopy){

  unsigned char* valueBytes=value;
  short loopCount=0;
  for(loopCount=0;loopCount<numberBytesToCopy;loopCount++){
    (*byteStart)[loopCount]=valueBytes[loopCount];
  }
  *byteStart+=(short)numberBytesToCopy;
}
