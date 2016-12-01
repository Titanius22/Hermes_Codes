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

//Prototyping
unsigned long getIntFromByte(unsigned char** ,short);

// The slave Arduino address
#define ADDRESS 0x04




// The I2C bus: This is for V2 pi's. For V1 Model B you need i2c-0
static const char *devName = "/dev/i2c-1";

int main(int argc, char *argv[])
{
    //char letter=0;
	//int letterCount;
	int lineCount;
	int fileCount;
	char Data[200];
	char recvBuf[25];
	struct timespec req={0},rem={0};
	//srand(time(NULL));
	char fileCounter[8];
	
	int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 
	
	//for(letterCount=0;letterCount<199;letterCount++){
	//			letter=rand()%(127-32)+32;
	//			Data[letterCount] = letter;
	//	}
	//Data[199] = 10;

    //SERVER STUFF. setting up socket
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10);
	
	//I2C STUFF. setting up i2c for communication
	printf("I2C: Connecting\n");
	int file;

	if ((file = open(devName, O_RDWR)) < 0) {
		fprintf(stderr, "I2C: Failed to access %d\n", devName);
		exit(1);
	}

	printf("I2C: acquiring buss to 0x%x\n", ADDRESS);

	if (ioctl(file, I2C_SLAVE, ADDRESS) < 0) {
		fprintf(stderr, "I2C: Failed to acquire bus access/talk to slave 0x%x\n", ADDRESS);
		exit(1);
	}
	
	req.tv_nsec = 50000000; //50ms
	//int lineCounter = 1;

    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
		
		//Data[0] = "1"; ////////////////////////////////////////////////////////////////used for counting lines
		/*write(connfd, Data, 200);
		
		for(lineCount=2;lineCount<=50;lineCount++){
				sprintf(fileCounter, "%d", lineCount+1);
				//Data[0] = fileCounter;//////////////////////////////////////////////////used for counting lines
				write(connfd, Data, 200);
		}
		
		for(fileCount=1;fileCount<60;fileCount++){
			for(lineCount=0;lineCount<50;lineCount++){
				sprintf(fileCounter, "%d", lineCount);
				//Data[0] = fileCounter;//////////////////////////////////////////////////used for counting lines
				write(connfd, Data, 200);
			}
		}*/
		
		while (1){
			read(file, recvBuf, 22);
			write(connfd, recvBuf, 22);
		
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
			nanosleep(&req,&rem);
			//lineCounter++;
		}
		close(file);
		
		
		
		fprintf(stderr, "Finished sending");
        close(connfd);
        sleep(1);
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