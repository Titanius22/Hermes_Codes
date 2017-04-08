//http://www.thegeekstuff.com/2011/12/c-socket-programming/
//http://developer.toradex.com/knowledge-base/watchdog-(linux)
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
#include <sys/stat.h>
#include <linux/watchdog.h>

////////////RF SPEED IN bps////////////////
#define RF_SPEED 85000

//Prototyping
unsigned long getIntFromByte(unsigned char** ,short);
void insertBytesFromInt(void* ,unsigned char** , short);
void tryNewSocketConnection();
void SetNewData(short);
void TripleData();
void updateLineCounter();
bool CheckSumMatches(char* , short);

// The slave Arduino address
#define ADDRESS 0x04

// The I2C bus: This is for V2 pi's. For V1 Model B you need i2c-0
static const char *devName = "/dev/i2c-1";
static const char *SocketNumFileName = "SocketNumber.txt";

//Globals
char recvBuf[500];
unsigned short startingSocketNum;
short madeConnection = 0; //becomes true when connection is made. If connection is lost afterwards (meaning when madeConnection is true), the port number is incremented and madeConnection is set to false till another connection is found.
FILE *SocketNumFile;
char SocketNumFileData[5];
int ServerFileNum = 0;
unsigned int counter = 0;
int i2cReadStatus;
short dataLineLength = 29;
short testNum = 0;
double packetTimeNanoSec;
double GPStimeNanoSec;
int numberLinesToSend = 10;

// watchdog stuff
static int api_watchdog_fd = -1;




int main(int argc, char *argv[])
{
    struct timespec req={0},rem={0};
	short connectionError;
	short connected = 0;
	char i2cDataPrechecked[33];
	short GPSLocCounter = 0;
	struct timespec RFstart={0,0}, RFstop={0,0}, GPSstart={0,0}, GPSstop={0,0};
	int i;
	packetTimeNanoSec =  (((double)dataLineLength*10*8)/ RF_SPEED )*1.0e9; // time between each packet transmission in nanoseconds
	GPStimeNanoSec = (1.0) * 1.0e9; // (time in seconds)
	double mathVarible;
	int bytesSentCounter = 0;
	int startElementForSending = 0;
	int totalBytesToSend = dataLineLength*10;
	short strikes = 0; // failures
	
	
	// Watchdog stuff
	int watchdogReturn = 0;

	watchdogReturn = api_watchdog_init("/dev/watchdog");
	if(watchdogReturn < 0){
        fprintf(stderr, "Could not init watchdog: %s\n", strerror(errno));
    }
	api_watchdog_setTime(15); // 15 second timer
	
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
	//SetNewData(GPSLocCounter);
	/////////////////////////////////////////////////////////REMOVE AFTER TEST////////////////////////////////////////////////////////////

    while(1)
    {
        // start clocks
		//clock_gettime(CLOCK_MONOTONIC, &RFstart);
		clock_gettime(CLOCK_MONOTONIC, &GPSstart);
		
		// try to make a connection
		tryNewSocketConnection();
		connectionError = 0;
		
		while (connectionError >= 0){
			
			api_watchdog_setTime(5); // 10 second timer
			
			connected = 1;
			
			clock_gettime(CLOCK_MONOTONIC, &GPSstop); //taking new time measurement
			if(((GPSstop.tv_nsec + GPSstop.tv_sec*1.0e9) - (GPSstart.tv_nsec + GPSstart.tv_sec*1.0e9)) > GPStimeNanoSec){
				
				// reset clock
				clock_gettime(CLOCK_MONOTONIC, &GPSstart);
				
				// keeps reading till a good message gets through
				i2cReadStatus = read(i2cfile, i2cDataPrechecked, dataLineLength+1); //The +1 is to also read the checksum
				while(!CheckSumMatches(i2cDataPrechecked, dataLineLength)){
					printf("i2cData dropped");
					i2cReadStatus = read(i2cfile, i2cDataPrechecked, dataLineLength+1); //The +1 is to also read the checksum
				}
				
				// copies the data 				
				for(i=0;i<dataLineLength;i++){
					recvBuf[i] = i2cDataPrechecked[i];
				}
				
				if(CheckSumMatches(i2cDataPrechecked, dataLineLength)){
					for(i=0;i<dataLineLength;i++){
						recvBuf[i] = i2cDataPrechecked[i];
					}
				}
				else{
					printf("i2cData dropped");
				}
				

				if(counter%1400 == 0){
					GPSLocCounter++;
					if (GPSLocCounter > 23){
						GPSLocCounter = 0;
					}
					
					SetNewGPS(GPSLocCounter);
				}
				
				TripleData(10);
				
			}
			
			// update counter
			updateLineCounter();
			
			
			startElementForSending = 0;
			// tries to send data, if available space in buffer is less than the data length, the loop will keep trying to send till the buffer clears up enough to send it.
			// send() was used instead of write() because send() have the flag argument as the last argument. MSG_NOSIGNAL as the flag is required because it tells send() to not exit/return errors if the connection is dropped.
			do{
				bytesSentCounter = send(ServerFileNum, &recvBuf[startElementForSending], totalBytesToSend, MSG_NOSIGNAL);
				startElementForSending += bytesSentCounter;
			}while((startElementForSending < totalBytesToSend) && (bytesSentCounter >= 0));
			counter++;
			
			if(bytesSentCounter < 0){
				strikes++;
			} else {
				strikes == 0;
			}
			
			if (strikes < 3){
				watchdogReturn = api_watchdog_hwfeed();	
			}
			
			if(counter%500 == 0){
				unsigned char* writeArray=recvBuf;
				unsigned char** wrPtr=&writeArray;
				
				fprintf(stderr, "%d ", (unsigned int)getIntFromByte(wrPtr,2));
			
				fprintf(stderr, "%d ", (unsigned int)getIntFromByte(wrPtr,3));
		  
				fprintf(stderr, "%d ", (unsigned int)getIntFromByte(wrPtr,3));

				fprintf(stderr, "%d ", (unsigned int)getIntFromByte(wrPtr,3));

				fprintf(stderr, "%d ", (unsigned int)getIntFromByte(wrPtr,2));
				
				fprintf(stderr, "%d ", (unsigned int)getIntFromByte(wrPtr,2));
				
				fprintf(stderr, "%d ", (unsigned int)getIntFromByte(wrPtr,1));
				
				fprintf(stderr, "%d ", (unsigned int)getIntFromByte(wrPtr,1));
				
				fprintf(stderr, "%d ", (unsigned int)getIntFromByte(wrPtr,1));
				
				fprintf(stderr, "%d ", (unsigned int)getIntFromByte(wrPtr,1));
				
				fprintf(stderr, "%d ", (unsigned int)getIntFromByte(wrPtr,1));
				
				fprintf(stderr, "%d ", (unsigned int)getIntFromByte(wrPtr,1));
		  
				fprintf(stderr, "%d ", (unsigned int)getIntFromByte(wrPtr,3));
				
				fprintf(stderr, "%d ", (unsigned int)getIntFromByte(wrPtr,2));

				fprintf(stderr, "%c", (char)getIntFromByte(wrPtr,1));

				fprintf(stderr, "%c", (char)getIntFromByte(wrPtr,1));

				fprintf(stderr, "%c\n", (char)getIntFromByte(wrPtr,1));
			}
			
		
			//printf("%s\n", buf);
			//nanosleep(&req,&rem);
			//lineCounter++;
		}
		if(connected == 1){
			connected = 0;
			watchdogReturn = api_watchdog_hwfeed();
			api_watchdog_setTime(15); // 10 second timer
		}
		
		
		//delay
		nanosleep(&req,&rem);
		
		
    }
	fprintf(stderr, "Finished sending");
    close(ServerFileNum);
}

// "pet" watchdog
int api_watchdog_hwfeed(void)
{
    int ret = -1;
    if (api_watchdog_fd < 0){
        fprintf(stderr, "Watchdog must be opened first!\n");
        return ret;
    }
    ret = ioctl(api_watchdog_fd, WDIOC_KEEPALIVE, NULL);
    if (ret < 0){
        fprintf(stderr, "Could not pat watchdog: %s\n", strerror(errno));
    }
    return ret;
}

// set new timeout for watchdog
void api_watchdog_setTime(int timeout)
{
	if(ioctl(api_watchdog_fd , WDIOC_SETTIMEOUT ,&timeout) != 0) {
		perror("SET");
		close (api_watchdog_fd);
		exit(1);
	}
}

// opens and starts watchdog
int api_watchdog_open(const char * watchdog_device)
{
    int ret = -1;
    if (api_watchdog_fd >= 0){
        fprintf(stderr, "Watchdog already opened\n");
        return ret;
    }
    api_watchdog_fd = open(watchdog_device, O_RDWR);
    if (api_watchdog_fd < 0){
        fprintf(stderr, "Could not open %s: %s\n", watchdog_device, strerror(errno));
        return api_watchdog_fd;
    }
    return api_watchdog_fd;
}

// initiate watchdog
int api_watchdog_init(const char *pcDevice)
{
    printf("Open WatchDog\n");
    int ret = 0;
    ret = api_watchdog_open("/dev/watchdog");
    if(ret < 0){
        return ret;
    }
    ret = api_watchdog_hwfeed();
    return ret;
}

void SetNewGPS(short pick){
	
	unsigned char* writeTo=recvBuf;
	
	//Line counter-------------------------------------------
	insertBytesFromInt(&counter, &writeTo, 2);

	//Latitude
	unsigned int intBuflatitude[12];
	intBuflatitude[0] = (unsigned int)(29.50329 * 100000);
	intBuflatitude[1] = (unsigned int)(29.43119 * 100000);
	intBuflatitude[2] = (unsigned int)(29.35710 * 100000);
	intBuflatitude[3] = (unsigned int)(29.22104 * 100000);
	intBuflatitude[4] = (unsigned int)(28.66936 * 100000);
	intBuflatitude[5] = (unsigned int)(28.56476 * 100000);
	intBuflatitude[6] = (unsigned int)(28.16450 * 100000);
	intBuflatitude[7] = (unsigned int)(28.50346 * 100000);
	intBuflatitude[8] = (unsigned int)(28.77712 * 100000);
	intBuflatitude[9] = (unsigned int)(29.20388 * 100000);
	intBuflatitude[10] = (unsigned int)(29.37722 * 100000);
	intBuflatitude[11] = (unsigned int)(29.54485 * 100000);
	if(pick < 12){
		insertBytesFromInt(&(intBuflatitude[pick]), &writeTo, 3);
	}
	else{
		insertBytesFromInt(&(intBuflatitude[23-pick]), &writeTo, 3);
	}
	
	//Longitude
	unsigned int intBuflongitude[12];
	intBuflongitude[0] = (unsigned int)(80.97349 * 100000);
	intBuflongitude[1] = (unsigned int)(80.79702 * 100000);
	intBuflongitude[2] = (unsigned int)(80.43164 * 100000);
	intBuflongitude[3] = (unsigned int)(79.91662 * 100000);
	intBuflongitude[4] = (unsigned int)(79.89670 * 100000);
	intBuflongitude[5] = (unsigned int)(80.57611 * 100000);
	intBuflongitude[6] = (unsigned int)(81.02110 * 100000);
	intBuflongitude[7] = (unsigned int)(81.53164 * 100000);
	intBuflongitude[8] = (unsigned int)(81.91366 * 100000);
	intBuflongitude[9] = (unsigned int)(82.27724 * 100000);
	intBuflongitude[10] = (unsigned int)(81.81272 * 100000);
	intBuflongitude[11] = (unsigned int)(81.32954 * 100000);
	if(pick < 12){
		insertBytesFromInt(&(intBuflongitude[pick]), &writeTo, 3);
	}
	else{
		insertBytesFromInt(&(intBuflongitude[23-pick]), &writeTo, 3);
	}
	

	//Altitude (meters) * 100 --------------------------------------------
	unsigned int intBufaltitude[24];
	intBufaltitude[0] = (unsigned int)(0 * 100);
	intBufaltitude[1] = (unsigned int)(1521.73 * 100);
	intBufaltitude[2] = (unsigned int)(3043.47 * 100);
	intBufaltitude[3] = (unsigned int)(4565.21 * 100);
	intBufaltitude[4] = (unsigned int)(6086.95 * 100);
	intBufaltitude[5] = (unsigned int)(7608.69 * 100);
	intBufaltitude[6] = (unsigned int)(9130.43 * 100);
	intBufaltitude[7] = (unsigned int)(10652.17 * 100);
	intBufaltitude[8] = (unsigned int)(12173.91 * 100);
	intBufaltitude[9] = (unsigned int)(13695.65 * 100);
	intBufaltitude[10] = (unsigned int)(15217.39 * 100);
	intBufaltitude[11] = (unsigned int)(16739.13 * 100);
	intBufaltitude[12] = (unsigned int)(18260.87 * 100);
	intBufaltitude[13] = (unsigned int)(19782.60 * 100);
	intBufaltitude[14] = (unsigned int)(21304.34 * 100);
	intBufaltitude[15] = (unsigned int)(22826.08 * 100);
	intBufaltitude[16] = (unsigned int)(24347.82 * 100);
	intBufaltitude[17] = (unsigned int)(25869.56 * 100);
	intBufaltitude[18] = (unsigned int)(27391.30 * 100);
	intBufaltitude[19] = (unsigned int)(28913.04 * 100);
	intBufaltitude[20] = (unsigned int)(30434.78 * 100);
	intBufaltitude[21] = (unsigned int)(31956.52 * 100);
	intBufaltitude[22] = (unsigned int)(33478.26 * 100);
	intBufaltitude[23] = (unsigned int)(35000 * 100);
	insertBytesFromInt(&(intBufaltitude[pick]), &writeTo, 3);

}

void SetNewData(short pick){
	
	unsigned char* writeTo=recvBuf;
	
	//Line counter-------------------------------------------
	insertBytesFromInt(&counter, &writeTo, 2);

	//Latitude
	unsigned int intBuflatitude[12];
	intBuflatitude[0] = (unsigned int)(29.50329 * 100000);
	intBuflatitude[1] = (unsigned int)(29.43119 * 100000);
	intBuflatitude[2] = (unsigned int)(29.35710 * 100000);
	intBuflatitude[3] = (unsigned int)(29.22104 * 100000);
	intBuflatitude[4] = (unsigned int)(28.66936 * 100000);
	intBuflatitude[5] = (unsigned int)(28.56476 * 100000);
	intBuflatitude[6] = (unsigned int)(28.16450 * 100000);
	intBuflatitude[7] = (unsigned int)(28.50346 * 100000);
	intBuflatitude[8] = (unsigned int)(28.77712 * 100000);
	intBuflatitude[9] = (unsigned int)(29.20388 * 100000);
	intBuflatitude[10] = (unsigned int)(29.37722 * 100000);
	intBuflatitude[11] = (unsigned int)(29.54485 * 100000);
	if(pick < 12){
		insertBytesFromInt(&(intBuflatitude[pick]), &writeTo, 3);
	}
	else{
		insertBytesFromInt(&(intBuflatitude[23-pick]), &writeTo, 3);
	}
	
	//Longitude
	unsigned int intBuflongitude[12];
	intBuflongitude[0] = (unsigned int)(80.97349 * 100000);
	intBuflongitude[1] = (unsigned int)(80.79702 * 100000);
	intBuflongitude[2] = (unsigned int)(80.43164 * 100000);
	intBuflongitude[3] = (unsigned int)(79.91662 * 100000);
	intBuflongitude[4] = (unsigned int)(79.89670 * 100000);
	intBuflongitude[5] = (unsigned int)(80.57611 * 100000);
	intBuflongitude[6] = (unsigned int)(81.02110 * 100000);
	intBuflongitude[7] = (unsigned int)(81.53164 * 100000);
	intBuflongitude[8] = (unsigned int)(81.91366 * 100000);
	intBuflongitude[9] = (unsigned int)(82.27724 * 100000);
	intBuflongitude[10] = (unsigned int)(81.81272 * 100000);
	intBuflongitude[11] = (unsigned int)(81.32954 * 100000);
	if(pick < 12){
		insertBytesFromInt(&(intBuflongitude[pick]), &writeTo, 3);
	}
	else{
		insertBytesFromInt(&(intBuflongitude[23-pick]), &writeTo, 3);
	}
	

	//Altitude (meters) * 100 --------------------------------------------
	unsigned int intBufaltitude[24];
	intBufaltitude[0] = (unsigned int)(0 * 100);
	intBufaltitude[1] = (unsigned int)(1521.73 * 100);
	intBufaltitude[2] = (unsigned int)(3043.47 * 100);
	intBufaltitude[3] = (unsigned int)(4565.21 * 100);
	intBufaltitude[4] = (unsigned int)(6086.95 * 100);
	intBufaltitude[5] = (unsigned int)(7608.69 * 100);
	intBufaltitude[6] = (unsigned int)(9130.43 * 100);
	intBufaltitude[7] = (unsigned int)(10652.17 * 100);
	intBufaltitude[8] = (unsigned int)(12173.91 * 100);
	intBufaltitude[9] = (unsigned int)(13695.65 * 100);
	intBufaltitude[10] = (unsigned int)(15217.39 * 100);
	intBufaltitude[11] = (unsigned int)(16739.13 * 100);
	intBufaltitude[12] = (unsigned int)(18260.87 * 100);
	intBufaltitude[13] = (unsigned int)(19782.60 * 100);
	intBufaltitude[14] = (unsigned int)(21304.34 * 100);
	intBufaltitude[15] = (unsigned int)(22826.08 * 100);
	intBufaltitude[16] = (unsigned int)(24347.82 * 100);
	intBufaltitude[17] = (unsigned int)(25869.56 * 100);
	intBufaltitude[18] = (unsigned int)(27391.30 * 100);
	intBufaltitude[19] = (unsigned int)(28913.04 * 100);
	intBufaltitude[20] = (unsigned int)(30434.78 * 100);
	intBufaltitude[21] = (unsigned int)(31956.52 * 100);
	intBufaltitude[22] = (unsigned int)(33478.26 * 100);
	intBufaltitude[23] = (unsigned int)(35000 * 100);
	insertBytesFromInt(&(intBufaltitude[pick]), &writeTo, 3);
	
	//Time(secs since UTC half day)--------------------------------------------
	unsigned int intBufTime = 450;
	insertBytesFromInt(&intBufTime, &writeTo, 2);

	//Thermistor count------------------------------------------
	unsigned short intBuftemperature = 450;
	insertBytesFromInt(&intBuftemperature, &writeTo, 2);

	//Battery Voltage---------------------------------------------
	unsigned short intBufBatVolt = 120;
	insertBytesFromInt(&intBufBatVolt, &writeTo, 1);
	
	//Battery Current---------------------------------------------
	unsigned short intBufBatCur = 120;
	insertBytesFromInt(&intBufBatCur, &writeTo, 1);

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
	
	// copy first line to all other lines
	TripleData();
}


void TripleData(){
	int i;
	int j;
	// repeat data for the second 2 lines of the 96 byte (3 x 32) transmission
	for(i=0;i<dataLineLength;i++){
		for(j=0;j<(numberLinesToSend-1);j++){
			recvBuf[i+(dataLineLength*j)] = recvBuf[i];
		}
	}
}


void updateLineCounter(){
	
	unsigned char* writeTo=recvBuf;
	int i;
	
	//Line counter-------------------------------------------
	insertBytesFromInt(&counter, &writeTo, 2);
	
	// repeat data for the second 2 lines of the 87 byte (3 x 29) transmission
	for(i=1;i<(numberLinesToSend);i++){
		recvBuf[(dataLineLength*i)] = recvBuf[0];
		recvBuf[(dataLineLength*i)+1] = recvBuf[1];
	}
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
	
	fprintf(stderr, "trying to connect\n");
	
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
  for(loopCount=0;loopCount<sizeof(unsigned long);loopCount++){

    //Copying bytes from one array to the other
    if(loopCount<bytes){
      intPtr[loopCount]=(*arrayStart)[loopCount];
    }else{
            intPtr[loopCount]=0;
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