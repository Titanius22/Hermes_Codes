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
#include <stdbool.h>

//Prototyping
unsigned long getIntFromByte(unsigned char** ,short);
void insertBytesFromInt(void* ,unsigned char** , short);
void tryNewSocketConnection();
void SetNewData();
void TripleData();
void updateLineCounter();
bool CheckSumMatches(char* , short);

// The slave Arduino address
#define ADDRESS 0x04

// The I2C bus: This is for V2 pi's. For V1 Model B you need i2c-0
static const char *devName = "/dev/i2c-1";
static const char *SocketNumFileName = "SocketNumber.txt";

//Globals
char recvBuf[100];
unsigned short startingSocketNum;
short madeConnection = 0; //becomes true when connection is made. If connection is lost afterwards (meaning when madeConnection is true), the port number is incremented and madeConnection is set to false till another connection is found.
FILE *SocketNumFile;
char SocketNumFileData[5];
int ServerFileNum = 0;
unsigned int counter = 0;
int i2cReadStatus;
short dataLineLength = 29


int main(int argc, char *argv[])
{
    struct timespec req={0},rem={0};
	short connectionError = 0;
	char i2cDataPrechecked[33];
	
	//I2C STUFF. setting up i2c for communication
	printf("I2C: Connecting\n");
	int i2cfile;

	if ((i2cfile = open(devName, O_RDWR)) < 0) {
		fprintf(stderr, "I2C: Failed to access %d\n", devName);
		exit(1);
	}

	printf("I2C: acquiring buss to 0x%x\n", ADDRESS);

	if (ioctl(i2cfile, I2C_SLAVE, ADDRESS) < 0) {
		fprintf(stderr, "I2C: Failed to acquire bus access/talk to slave 0x%x\n", ADDRESS);
		exit(1);
	}
	
	//set sleep duration
	req.tv_nsec = 5000000; //5ms
	
	//look at SocketNum file to check what number to start with
	SocketNumFile = fopen(SocketNumFileName, "r");
	fread(SocketNumFileData, 2, 1, SocketNumFile);
	fclose(SocketNumFile);
	startingSocketNum = SocketNumFileData[0] << 8 | SocketNumFileData[1];
	
	///////////////////////////////////////////////REMOVE AFTER TEST///////////////////////////////////////////////////////////////////
	SetNewData();
	/////////////////////////////////////////////////////////REMOVE AFTER TEST////////////////////////////////////////////////////////////

    while(1)
    {
        tryNewSocketConnection();
		
		while (connectionError >= 0){
			
			if(counter%200 == 0){
				i2cReadStatus = read(i2cfile, i2cDataPrechecked, dataLineLength+1); //The +1 is to also read the checksum
				if(CheckSumMatches(i2cDataPrechecked, dataLineLength)){
					strncpy(recvBuf, i2cDataPrechecked, dataLineLength);
					TripleData();
				}
				else{
					printf("i2cData dropped");
				}
				
			}		
			
			connectionError = write(ServerFileNum, recvBuf, dataLineLength*3);
			
			counter++;
			updateLineCounter();
			
			
			/*if(lineCounter%10 == 0){
				char* writeArray=buf;
				char** wrPtr=&writeArray;
				printf("%d ", (unsigned int)getIntFromByte(wrPtr,3));
			
				printf("%d ", (unsigned int)getIntFromByte(wrPtr,4));
		  
				printf("%d ", (unsigned int)getIntFromByte(wrPtr,4));

				printf("%d ", (unsigned int)getIntFromByte(wrPtr,3));

				printf("%d ", (unsigned int)getIntFromByte(wrPtr,2));
		  
				printf("%d ", (unsigned int)getIntFromByte(wrPtr,3));

				printf("%c", (char)getIntFromByte(wrPtr,1));

				printf("%c", (char)getIntFromByte(wrPtr,1));

				printf("%c\n", (char)getIntFromByte(wrPtr,1));
			}*/
		
			//printf("%s\n", buf);
			//nanosleep(&req,&rem);
			//lineCounter++;
		}
		
		//delay
		nanosleep(&req,&rem);
		
		
    }
	fprintf(stderr, "Finished sending");
    close(ServerFileNum);
}


void SetNewData(){
	
	unsigned char* writeTo=recvBuf;
	
	//Line counter-------------------------------------------
	int intBuflineCount = 150;
	insertBytesFromInt(&intBuflineCount, &writeTo, 2);

	//Latitude 
	//unsigned int longBuflatitude = (unsigned int)(29.210592 * 1000000); //East Ocean
	//unsigned int longBuflatitude = (unsigned int)(28.528328 * 1000000); //Orlando
	unsigned int longBuflatitude = (unsigned int)(30.327081 * 1000000); //Jacksonville
	insertBytesFromInt(&longBuflatitude, &writeTo, 3);

	//Longitude
	//unsigned int longBuflongitude = (unsigned int)(81.001407 * 1000000); //East Ocean
	//unsigned int longBuflongitude = (unsigned int)(81.385902 * 1000000); //Orlando 
	unsigned int longBuflongitude = (unsigned int)(81.641377 * 1000000); //Jacksonville
	insertBytesFromInt(&longBuflongitude, &writeTo, 3);

	//Altitude (meters) * 100 --------------------------------------------
	unsigned int intBufaltitude = 1000 * 100;
	insertBytesFromInt(&intBufaltitude, &writeTo, 3);
	
	//Time(secs since UTC half day)--------------------------------------------
	unsigned int intBufaltitude = 1000;
	insertBytesFromInt(&intBufaltitude, &writeTo, 2);

	//Thermistor count------------------------------------------
	unsigned short intBuftemperature = 450;
	insertBytesFromInt(&intBuftemperature, &writeTo, 2);

	//Battery Voltage---------------------------------------------
	unsigned short intBufpressure = 120;
	insertBytesFromInt(&intBufpressure, &writeTo, 1);
	
	//Battery Current---------------------------------------------
	unsigned short intBufpressure = 120;
	insertBytesFromInt(&intBufpressure, &writeTo, 1);

	//Magnotometer X---------------------------------------------
	short intBufpressure2 = 80;
	insertBytesFromInt(&intBufpressure2, &writeTo, 1);

	//Magnotometer Y---------------------------------------------
	short intBufpressure3 = 60;
	insertBytesFromInt(&intBufpressure3, &writeTo, 1);

	//Magnotometer Z---------------------------------------------
	short intBufpressure4 = 40;
	insertBytesFromInt(&intBufpressure4, &writeTo, 1);

	//Humidity---------------------------------------------
	unsigned short intBufpressure5 = 96;
	insertBytesFromInt(&intBufpressure5, &writeTo, 1);

	//Pressure---------------------------------------------
	unsigned int intBufpressure6 = 102300;
	insertBytesFromInt(&intBufpressure6, &writeTo, 3);

	//Internal Temperature---------------------------------------------
	short intBufpressure7 = 15;
	insertBytesFromInt(&intBufpressure7, &writeTo, 2);

	//End of line chars-------------------------------------------

	recvBuf[dataLineLength-3] = 'E';
	recvBuf[dataLineLength-2] = 'N';
	recvBuf[dataLineLength-1] = 'D';
	
	// repeat data for the second 2 lines of the 87 byte (3 x 29) transmission
	int i;
	for(i=0;i<dataLineLength;i++){
		recvBuf[i+dataLineLength] = recvBuf[i];
		recvBuf[i+(dataLineLength*2)] = recvBuf[i];
	}
}


void TripleData(){
	
	// repeat data for the second 2 lines of the 96 byte (3 x 32) transmission
	for(i=0;i<dataLineLength;i++){
		recvBuf[i+dataLineLength] = recvBuf[i];
		recvBuf[i+(dataLineLength*2)] = recvBuf[i];
	}
}


void updateLineCounter(){
	
	unsigned char* writeTo=recvBuf;
	
	//Line counter-------------------------------------------
	insertBytesFromInt(&counter, &writeTo, 3);
	
	// repeat data for the second 2 lines of the 96 byte (3 x 32) transmission
	recvBuf[dataLineLength] = recvBuf[0];
	recvBuf[dataLineLength+1] = recvBuf[1];
	
	recvBuf[(dataLineLength*2)] = recvBuf[0];
	recvBuf[(dataLineLength*2)+1] = recvBuf[1];
}


bool CheckSumMatches(char* arrayToCheck, short dataLength){ //dataLength excludes the checksum byte which is the next byte
	bool passesCheck = false;
	short i;
	unsigned short summedAmount = 0;
	for(i=0;i<dataLength;i++){
		summedAmount += (unsigned char)arrayToCheck[i];
	}
	if((unsigned char)arrayToCheck[dataLength] == (unsigned char)(summedAmount%64)){
		passesCheck = true;
	}
	
	return passesCheck;
}


//SERVER STUFF. setting up socket
void tryNewSocketConnection(){
	
	//if connection was already made but then was broken and tryNewSocketConnection() was called again, this if statment will increment the socketnumber and reset the connecting flag (madeConnection) before continuing
	if (madeConnection == 1){
		close(ServerFileNum);
		startingSocketNum++;
		madeConnection = 0;
	}
	
	int listenfd = 0;
    struct sockaddr_in serv_addr;
	
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(startingSocketNum); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(listenfd, 10);
	ServerFileNum = accept(listenfd, (struct sockaddr*)NULL, NULL);
	
	//Only makes it this far if none of the above errors have occured.
	//Connection was made therefor the SocketNumber file but be updated
	SocketNumFileData[1] = (char)(((unsigned short)SocketNumFileData[1]) + 1); //increments the socket number by 1 
	SocketNumFile = fopen(SocketNumFileName, "w");
	fwrite(SocketNumFileData, 2, 1, SocketNumFile);
	fclose(SocketNumFile);
	
	madeConnection = 1;
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