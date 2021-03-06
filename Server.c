//http://www.thegeekstuff.com/2011/12/c-socket-programming/
//http://developer.toradex.com/knowledge-base/watchdog-(linux)
//http://my.fit.edu/~vkepuska/ece3551/ADI_Speedway_Golden/Blackfin%20Speedway%20Manuals/LwIP/socket-api/setsockopt_exp.html
//http://stackoverflow.com/questions/2876024/linux-is-there-a-read-or-recv-from-socket-with-timeout
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
#include <signal.h>
#include <wiringPi.h>

////////////RF SPEED IN bps////////////////
#define RF_SPEED 85000

//Prototyping
unsigned long getIntFromByte(unsigned char** ,short);
void insertBytesFromInt(void* ,unsigned char** , short);
void tryNewSocketConnection(void);
void SetNewData(short);
void TripleData(void);
void updateLineCounter(void);
bool CheckSumMatches(char* , short);
int api_watchdog_hwfeed(void);
void api_watchdog_setTime(int);
int api_watchdog_open(const char *);
int api_watchdog_init(const char *);
void SetNewGPS(short);
void initArduinoReset(void);
void ResetArduino(void);

// The slave Arduino info
#define ADDRESS 0x04
#define ARDUINO_RESET_PIN 26
#define STARTING_SOCKET_NUMBER 5000

// The I2C bus: This is for V2 pi's. For V1 Model B you need i2c-0
static const char *devName = "/dev/i2c-1";
static const char *SocketNumFileName = "SocketNumber.txt";

//Globals
char recvBuf[500];
unsigned int startingSocketNum;
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
	char i2cDataPrechecked[33];
	short GPSLocCounter = 0;
	struct timespec RFstart={0,0}, RFstop={0,0}, GPSstart={0,0}, GPSstop={0,0};
	int i;
	packetTimeNanoSec =  (((double)dataLineLength*10*8)/ RF_SPEED )*1.0e9; // time between each packet transmission in nanoseconds
	GPStimeNanoSec = (0.01) * 1.0e9; // (time in seconds)
	double mathVarible;
	int bytesSentCounter = 0;
	int startElementForSending = 0;
	int totalBytesToSend = dataLineLength*10;
	short smallStrikes = 0;
	short strikes = 0; // failures
	short satisfied = 0; // 1 satisfied, 0 not-satisfied
	short i2cDropCount = 0;
	short arduinoResets = 0;
	
	
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
	
	//Initializes WiringPi library, allows GPIO pins to be used
	wiringPiSetupGpio(); // Initializes wiringPi using the Broadcom GPIO pin numbers
	initArduinoReset(); 
	
	//set sleep duration
	req.tv_nsec = 50000000; //50ms. effects stuff near line 160, its a forced sleep till reset
	
	//look at SocketNum file to check what number to start with
	//SocketNumFile = fopen(SocketNumFileName, "r");
	//fread(SocketNumFileData, 2, 1, SocketNumFile);
	//fclose(SocketNumFile);
	startingSocketNum = STARTING_SOCKET_NUMBER;
	
	///////////////////////////////////////////////REMOVE AFTER TEST///////////////////////////////////////////////////////////////////
	//SetNewData(GPSLocCounter);
	/////////////////////////////////////////////////////////REMOVE AFTER TEST////////////////////////////////////////////////////////////

    while(1)
    {
        // start clocks
		clock_gettime(CLOCK_MONOTONIC, &GPSstart);
		
		// try to make a connection
		tryNewSocketConnection();
		
		if(madeConnection == 1){
			fprintf(stderr, "Connected\n");
			connectionError = 0;
			strikes = 0;
			
			while (connectionError >= 0){
				
				api_watchdog_setTime(6); // 6 second timer
				
				clock_gettime(CLOCK_MONOTONIC, &GPSstop); //taking new time measurement
				if(((GPSstop.tv_nsec + GPSstop.tv_sec*1.0e9) - (GPSstart.tv_nsec + GPSstart.tv_sec*1.0e9)) > GPStimeNanoSec){
					
					// reset clock
					clock_gettime(CLOCK_MONOTONIC, &GPSstart);
					
					// keeps reading till a good message gets through
					arduinoResets = 0;
					i2cDropCount = 0;
					i2cReadStatus = read(i2cfile, i2cDataPrechecked, dataLineLength+1); //The +1 is to also read the checksum
					while(!CheckSumMatches(i2cDataPrechecked, dataLineLength)){
						i2cDropCount++;
						fprintf(stderr, "i2cData dropped");
						
						if(i2cDropCount >= 3){ // if data is dropped 3 times in a row, arduino will be reset
							watchdogReturn = api_watchdog_hwfeed();
							i2cDropCount = 0; // restes to zero
							if(arduinoResets >= 3){ // Arduino has had to reset 3 times, Pi will be reset
								api_watchdog_setTime(1); // 1 second timer
								for(i=0;i<300;i++){ // will sleep for 5ms * 300 = 1.5 seconds. should only take 1 second
									nanosleep(&req,&rem);
								}
							}
							
							ResetArduino();
							arduinoResets++;
						}
						
						i2cReadStatus = read(i2cfile, i2cDataPrechecked, dataLineLength+1); //The +1 is to also read the checksum
					}
					
					// copies the data 				
					for(i=0;i<dataLineLength;i++){
						recvBuf[i] = i2cDataPrechecked[i];
					}
					
					// SetNewGPS(GPSLocCounter);
					
					// if(counter%1500 == 0){
						// GPSLocCounter++;
						// if (GPSLocCounter > 23){
							// GPSLocCounter = 0;
						// }						
					// }
					
					TripleData();
					
				}
				
				// update counter
				updateLineCounter();
				
				
				startElementForSending = 0;
				// tries to send data, if available space in buffer is less than the data length, the loop will keep trying to send till the buffer clears up enough to send it.
				// send() was used instead of write() because send() have the flag argument as the last argument. MSG_NOSIGNAL as the flag is required because it tells send() to not exit/return errors if the connection is dropped.
				do{
					satisfied = 0;
					do{
						bytesSentCounter = send(ServerFileNum, &recvBuf[startElementForSending], totalBytesToSend, 0);
						if (bytesSentCounter < 0){
							if(errno == EAGAIN || errno == EWOULDBLOCK){
								watchdogReturn = api_watchdog_hwfeed();	
								smallStrikes++;
								if(smallStrikes >= 3){
									satisfied = 1;
									strikes = 4; // will cause the program to try and reconnect
								}
							} else if(errno == EPIPE){
								satisfied = 1;
								strikes = 4; // will cause the program to try and reconnect
							}
						} else{
							satisfied = 1;
							startElementForSending += bytesSentCounter;
						}
					}while(satisfied == 0); // keeps running till the commands goes through (if blocked due to data over flow) or 
				}while((bytesSentCounter >= 0) && (startElementForSending < totalBytesToSend)); // keeps writing till buffer is full
				counter++;
				
				if(bytesSentCounter < 0 || startElementForSending == 0){
					strikes++;
				} else{
					strikes == 0;
				}
				
				if (strikes < 3){
					watchdogReturn = api_watchdog_hwfeed();	
				} else {
					connectionError = -1;
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
					
					fprintf(stderr, "%d ", (int)getIntFromByte(wrPtr,2));
					
					fprintf(stderr, "%d ", (int)getIntFromByte(wrPtr,2));
			  
					fprintf(stderr, "%d ", (unsigned int)getIntFromByte(wrPtr,3));
					
					fprintf(stderr, "%d ", (unsigned int)getIntFromByte(wrPtr,2));

					fprintf(stderr, "%c", (char)getIntFromByte(wrPtr,1));

					fprintf(stderr, "%c", (char)getIntFromByte(wrPtr,1));

					fprintf(stderr, "%c\n", (char)getIntFromByte(wrPtr,1));
				}
				
			}
			if(connectionError != 0){
				watchdogReturn = api_watchdog_hwfeed();
				api_watchdog_setTime(15); // 15 second timer
			}
		}
		
		//delay
		nanosleep(&req,&rem);
		
    }
	fprintf(stderr, "Finished sending");
    close(ServerFileNum);
}

// Initializes pin settings for Arduino reset
void initArduinoReset(void){
	pinMode( ARDUINO_RESET_PIN , OUTPUT);
	digitalWrite( ARDUINO_RESET_PIN , HIGH);
}

void ResetArduino(void){
	struct timespec req={0},rem={0};
	int i;
	
	//set sleep duration
	req.tv_nsec = 10000000; //10ms
	
	// Sends LOW to Arduino reset pin, cause it to reset
	digitalWrite( ARDUINO_RESET_PIN , LOW );
	
	//delay to ensure it got the message
	for(i=0;i<50;i++){ // will sleep for 10ms * 50 = 0.5 seconds.
		nanosleep(&req,&rem);
	}
	
	// Sends HIGH to Arduino reset pin, allowing it to turn on as normal
	digitalWrite( ARDUINO_RESET_PIN , HIGH );
}

// "pet" watchdog
int api_watchdog_hwfeed(void){
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
void api_watchdog_setTime(int timeout){
	if(ioctl(api_watchdog_fd , WDIOC_SETTIMEOUT ,&timeout) != 0) {
		perror("SET");
		close (api_watchdog_fd);
		exit(1);
	}
}

// opens and starts watchdog
int api_watchdog_open(const char * watchdog_device){
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
int api_watchdog_init(const char *pcDevice){
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
	
	// the +1 is to be sure its never 0. 0 sometimes appears in error, this would prevent an errored message from being accepted.
	if((unsigned char)arrayToCheck[dataLength] == (unsigned char)(summedAmount%64) + 1){
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
		startingSocketNum = STARTING_SOCKET_NUMBER + ((startingSocketNum+1) - STARTING_SOCKET_NUMBER)%3;
		madeConnection = 0;
	}
	
	int listenfd = 0;
	struct sockaddr_in serv_addr;
	struct sigaction handler;
	struct timeval tv;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(startingSocketNum); 

	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	listen(listenfd, 10);

	// set a timeout for you  commands
	tv.tv_sec = 1;  /* 1 Secs Timeout */
	tv.tv_usec = 0;  // Not init'ing this can cause strange errors
	setsockopt(listenfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(struct timeval));

	ServerFileNum = accept(listenfd, (struct sockaddr*)NULL, NULL);

	if(ServerFileNum >= 0){
		// Setup Action Handler
		handler.sa_handler = SIG_IGN; // Ignore signal
		sigemptyset(&handler.sa_mask);
		handler.sa_flags=0;
		if (sigaction(SIGPIPE,&handler,0) < 0){ // Setup signal
			perror(0);
		}

		//Only makes it this far if none of the above errors have occured.
		//Connection was made therefor the SocketNumber file but be updated
		//SocketNumFileData[1] = (char)(((unsigned short)SocketNumFileData[1]) + 1); //increments the socket number by 1 
		//SocketNumFile = fopen(SocketNumFileName, "w");
		//fwrite(SocketNumFileData, 2, 1, SocketNumFile);
		//fclose(SocketNumFile);

		madeConnection = 1;
	}
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