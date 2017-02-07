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
int tryNewSocketConnection(int);

//Globals
int ServerFileNum;
struct sockaddr_in serv_addr; 

// The slave Arduino address
#define ADDRESS 0x04

// The I2C bus: This is for V2 pi's. For V1 Model B you need i2c-0
static const char *devName = "/dev/i2c-1";

int main(int argc, char *argv[])
{
    char recvBuff[200];
	int n = 0;
	int startingSocketNum = 5000;
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
	char fullFilePath[60];
	FILE *filePointer;
	char leftOvers[25];
	char* writeArray;
	char** wrPtr;
	char command = 1;
	
	while(1){ 
		
		// If connection was made properly
		if(ServerFileNum = tryNewSocketConnection(startingSocketNum) >= 0){
		
		
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
					
					printf("%d ", (unsigned int)getIntFromByte(wrPtr,3)); // Line Counter
			  
					printf("%lu ", (unsigned long)getIntFromByte(wrPtr,5)); // Longitude
			  
					printf("%lu ", (unsigned long)getIntFromByte(wrPtr,5)); // Latitude

					printf("%d ", (unsigned int)getIntFromByte(wrPtr,3)); // Altitude

					printf("%d ", (unsigned int)getIntFromByte(wrPtr,2)); // External Thermistor
			  
					printf("%d ", (unsigned int)getIntFromByte(wrPtr,1)); // Battery Voltage

					printf("%d ", (unsigned int)getIntFromByte(wrPtr,1)); // Magnotometer X

					printf("%d ", (unsigned int)getIntFromByte(wrPtr,1)); // Magnotometer Y

					printf("%d ", (unsigned int)getIntFromByte(wrPtr,1)); // Magnotometer Z

					printf("%d ", (unsigned int)getIntFromByte(wrPtr,1)); // Humidity

					printf("%d ", (unsigned int)getIntFromByte(wrPtr,4)); // Pressure

					printf("%d ", (unsigned int)getIntFromByte(wrPtr,2)); // Internal Temperature
					
					printf("%c", (char)getIntFromByte(wrPtr,1)); // 'E'

					printf("%c", (char)getIntFromByte(wrPtr,1)); // 'N'

					printf("%c\n", (char)getIntFromByte(wrPtr,1)); // 'D'
					
					// Send data over I2C
					write(i2cFile, command, 1);
					write(i2cFile, recvBuff+3, 13);
					
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
void tryNewSocketConnection(int socketNum){
		
    if((ServerFileNum = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return -1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(socketNum); 

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
