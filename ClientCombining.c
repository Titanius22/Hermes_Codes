//http://www.thegeekstuff.com/2011/12/c-socket-programming/
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

#define N_ELEMENT_INDEX 87 // (3*29-1)+1 bytes
#define NEW_FILE_RATE 900 // number of seconds (15 min * 60 sec) before data saving switches to a new file.
#define DATA_TO_MOUNT_RATE 4 // number of seconds between data sent to mount
#define LINE_LENGTH 29
#define PACKET_LENGTH 87 //3*29
#define NUM_COL_RECV_BUFF_ARRAY 55
#define RECV_BUFF_ARRAY_LENGTH 4785

//prototyping
unsigned long getIntFromByte(unsigned char** , short);
void insertBytesFromInt(void* , unsigned char** , short);
int tryNewSocketConnection();
short findOffset(char[] , short , short);
unsigned short getNumberOfFullElements(char (*)[ PACKET_LENGTH] , unsigned short , unsigned short , unsigned short, unsigned short * );

//Globals
int ServerFileNum;
struct sockaddr_in serv_addr;
unsigned short startingSocketNum; // = 5000;
short madeConnection = 0; //becomes true when connection is made. If connection is lost afterwards (meaning when madeConnection is true), the port number is incremented and madeConnection is set to false till another connection is found.
short lineLength = 32;
char SocketNumFileData[5];
FILE *SocketNumFile;

// The slave Arduino address
#define ADDRESS 0x04

// The I2C bus: This is for V2 pi's. For V1 Model B you need i2c-0
static const char *devName = "/dev/i2c-1";
static const char *SocketNumFileName = "SocketNumber.txt";

int main(int argc, char *argv[])
{
    char recvBuff[100];
	unsigned int recvBuffCURRENTelement = 0; //element at which to start saving data to
	unsigned short recvBuffRowLength[NUM_COL_RECV_BUFF_ARRAY];
	unsigned short n = 1;
	struct timespec req={0},rem={0};
	req.tv_nsec = 500000000; //500ms
	
	unsigned short i = 0;
	int fileCount = 1;
	int fileLineCount = 1;
	char fileCounter[8];
	char filepath[] =  "/home/alarm/randomJunk/cCodeTests/DistanceTest/";
	char fileName[] = "DistanceTestFULLER";
	char fileExt[] = ".txt";
	char fullFilePath[80];
	//char fullFilePath[] = "/home/alarm/randomJunk/cCodeTests/DistanceTest/DistanceTestFULL.txt";;
	FILE *filePointer;
	unsigned char* writeArray;
	unsigned char** wrPtr;
	char command[30] = "";
	short offset = 0;
	unsigned short CounterRecvBuffArray = 0;
	
	unsigned short DataLineCounter;
	unsigned int DataGPS[4]; // Longitude, Latitude, Altitude
	unsigned int DataSensors[9]; // External Thermistor, Battery Voltage, Magnotometer X, Y, Z, Humidity, Pressure, Internal Temperature.
	char DataEndLine[3];
	time_t epochTimeSecondsFile = time(0);
	time_t epochTimeSecondsTracking = time(0);
	time_t bufEpoch;
	unsigned short createNewFile = 1; // 1 yes, 0 no
	unsigned short numFullTransmissions = 0;
	unsigned short strikeCounter = 0;
	
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
	
	//look at SocketNum file to check what number to start with
	SocketNumFile = fopen(SocketNumFileName, "r");
	fread(SocketNumFileData, 2, 1, SocketNumFile);
	fclose(SocketNumFile);
	startingSocketNum = SocketNumFileData[0] << 8 | SocketNumFileData[1];
	
	while(1){ 
		
		// If connection was made properly
		if(tryNewSocketConnection() == 0){
		
		
			//while ( (n = recv(ServerFileNum, recvBuff, 32 , 0)) > 0) same as read if last argument is 0
			while ( (n = read(ServerFileNum, recvBuff, lineLength*3)) > 0) 
			{

				if(n!=0){
					strikeCounter = 0;
					//do{		
					//	recvBuffCURRENTelement += n;
					//}while ((recvBuffCURRENTelement < RECV_BUFF_ARRAY_LENGTH) && (n = read(ServerFileNum, &recvBuff[recvBuffCURRENTelement], RECV_BUFF_ARRAY_LENGTH - recvBuffCURRENTelement)) > 0); /* The order of the conditional statement matters. If the first condition fails it will not check the 
					//second condition. This is good because if the first condition fails and the second condition is tryed, the data will be saved
					//outside of the array. This has already caused problems requireing me to change the while loop to the current configuration.
					//*/
					
					// At the start of every new "page", it creates and opens a new file
					if (createNewFile == 1){
						sprintf(fileCounter, "%04d", fileCount);
					
						strcpy(fullFilePath, filepath);
						strcat(fullFilePath, fileName);
						//strcpy(fullFilePath, mostFilePath);
						strcat(fullFilePath, fileCounter);
						strcat(fullFilePath, fileExt);
						filePointer = fopen(fullFilePath, "a");
						createNewFile = 0;
					}
				
					// Writes data to the document unconverted
					if (filePointer != NULL)
					{
						fwrite(recvBuff, n, 1, filePointer);
					}
					
					// Sends data to the mount
					bufEpoch = time(0);
					if(bufEpoch > (epochTimeSecondsTracking + DATA_TO_MOUNT_RATE)){
						epochTimeSecondsTracking = bufEpoch;
						offset = findOffset(recvBuff, LINE_LENGTH *3, LINE_LENGTH); // reads from the beginning of array since each array is about 1/4 second worth of data, the data at the front or end should be pretty similiar. looks through the first 4 line_length worth of data to find a match.
						
						//if(offset >= 0){
						if(offset >= 0){
							//writeArray=recvBuff[offset];
							writeArray = recvBuff + offset;
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
							
							// Send data over I2C
							//sprintf(command, "2 %lu %lu %d ", DataGPS[0], DataGPS[1], DataGPS[2]);
							//write(i2cFile, command, strlen(command));
							command[0] = '2';
							strncat(command+1, recvBuff+offset+2, 9);
							write(i2cFile, command, strlen(command));
						}
					}
					
					
					if (createNewFile == 0){
						
						// WRITES DATA to the document unconverted
						//fflush(filePointer);
						recvBuffCURRENTelement = 0;
						//fclose(filePointer);
						//filePointer = NULL; //This is so that the file pointr can be checked if it has been closed
						
						// close current file, set flag to open a new one
						bufEpoch = time(0);
						if(bufEpoch > (epochTimeSecondsFile + NEW_FILE_RATE)){
							epochTimeSecondsFile = bufEpoch;
							
							// File tracking and counting
							fileLineCount = 0;
							fileCount++;
							fclose(filePointer);
							filePointer = NULL; //This is so that the file pointr can be checked if it has been closed
							createNewFile = 1;
						}
					}				
					
					fileLineCount++;
				}
				
				if(n==0){
					strikeCounter++;
					n=1; //sets it back to not-zero so that it reading too fast won't trigger a failure (reading too fast as in it clears the buffer before it has a chance to get more data)
					if(strikeCounter >= 3){ //3 strikes, you're out. (the reading is probally failing of something)
						n=0; //sets it to zero so it will fail the larger while loop
					}
				}
			}
			
			if(filePointer!=NULL){
				fclose(filePointer);
				filePointer = NULL; //This is so that the file pointr can be checked if it has been closed
				createNewFile = 1;
			}

		}
		
		//delay
		nanosleep(&req,&rem);
	}

    //if(n < 0)
    //{
    //    printf("\n Read error \n");
    //} 

    return 0;
}


// If the recieved data is offset by n number of chars, the function will find n assuming the line ends with "END" and is as long as defined
/*///////////////////////////////////////EXAMPLE//////////////////////////////////////
correct data line: 123456789END

potential offsetingArray to be sent into the function: 3456789END123456789END12
the function will return 10.

potential offsetingArray to be sent into the function: 3456789END123456789EN
the function will return -1 because a complete string of 123456789END can't be achieved.
*/
short findOffset(char* offsetingArray, short lengthOfArray, short lengthOfLine){
	short result = -1;
	short i;
	
	i = lengthOfLine - 3; // -1 puts it at the end of the potential Dataline or 'D', -3 puts is at the potential 'E'
	
	// This will keep looping through the array till it finds an 'E' followed by an 'N' and a 'D' or is hits the end and will return a -1
	while(i < lengthOfArray-2){ // its lengthOfArray-2 because 'E', 'N', and 'D' must match. If it reaches lengthOfArray-2, there couldn't be 'E', 'N', and 'D'.
		if(offsetingArray[i] == 'E'){
			if (offsetingArray[i+1] == 'N' && offsetingArray[i+2] == 'D'){
				result = i - (lengthOfLine - 3); //the element in the offsetingArray that starts the propper data line
				break;
			}
		}
		i++;
	}
	return result;
}


unsigned short getNumberOfFullElements(char arrayToCheck[][PACKET_LENGTH ], unsigned short startingCol, unsigned short maxColNum, unsigned short packetLength, unsigned short rowLengthArray[]){
	unsigned short result = 0;
	unsigned short iCounter = startingCol;
	
	while((rowLengthArray[iCounter] == packetLength) && (iCounter <= maxColNum)){ 
		result++;
		iCounter++;
	}
	
	return result;
}


//SERVER STUFF. setting up socket
int tryNewSocketConnection(){

	//if connection was already made but then was broken and tryNewSocketConnection() was called again, this if statment will increment the socketnumber and reset the connecting flag (madeConnection) before continuing
	if (madeConnection == 1){
		close(ServerFileNum);
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
	
	if(connect(ServerFileNum, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\n Error : Connect Failed \n");
		return -1;
	}
	
	//Only makes it this far if none of the above errors have occured.
	//Connection was made therefor the SocketNumber file but be updated
	SocketNumFileData[1] = (char)(((unsigned short)SocketNumFileData[1]) + 1); //increments the socket number by 1 
	SocketNumFile = fopen(SocketNumFileName, "w");
	fwrite(SocketNumFileData, 2, 1, SocketNumFile);
	fclose(SocketNumFile);
	
	madeConnection = 1;
	
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

void insertBytesFromInt(void* value,unsigned char** byteStart, short numberBytesToCopy){

  unsigned char* valueBytes=value;
  short loopCount=0;
  for(loopCount=0;loopCount<numberBytesToCopy;loopCount++){
    (*byteStart)[loopCount]=valueBytes[loopCount];
  }
  *byteStart+=(short)numberBytesToCopy;
}
