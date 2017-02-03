//http://www.thegeekstuff.com/2011/12/c-socket-programming/?utm_source=feedburner
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

//prototyping
unsigned long getIntFromByte(unsigned char** , short);

int main(int argc, char *argv[])
{
    int sockfd = 0, n = 0;
    char recvBuff[200];
    struct sockaddr_in serv_addr; 

    if(argc != 2)
    {
        printf("\n Usage: %s <ip of server> \n",argv[0]);
        return 1;
    }

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000); 

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    } 

	while(1){
	
		if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		{
		   printf("\n Error : Connect Failed \n");
		   return 1;
		} 
		
		int fileCount = 1;
		//int packetCount = 1;
		int fileLineCount = 1;
		char fileCounter[8];
		//char filepath[] =  "/home/alarm/randomJunk/cCodeTests/";
		char fileName[] = "DistanceTest";
		char mostFilePath[] = FREAKIN FIX MY ADDRESS "/home/alarm/randomJunk/cCodeTests/DistanceTest";
		char fileExt[] = ".txt";
		char fullFilePath[60];
		FILE *filePointer;
		char leftOvers[25];
		char* writeArray;
		char** wrPtr;
		
		while ( (n = recv(sockfd, recvBuff, 32 , 0)) > 0)
		{
			// if (n != 200){
				// fprintf(stderr, "What the jack just happen????     n: %d\n", n);
			// }
			
			if (fileLineCount == 1){ // && fileCount == 1){
				sprintf(fileCounter, "%04d", fileCount);
			
				strcpy(fullFilePath, filepath);
				strcat(fullFilePath, fileName);
				strcpy(fullFilePath, mostFilePath);
				strcat(fullFilePath, fileCounter);
				strcat(fullFilePath, fileExt);
				
				filePointer = fopen(fullFilePath, "a");
			}
		
			if (filePointer != NULL)
			{
				fwrite(&recvBuff, n, 1, filePointer);
						// if ((packetCount%10 == 0)){
							 //char* writeArray=recvBuff;
							 //char** wrPtr=&writeArray;
							
							// printf("%d ", (unsigned int)getIntFromByte(wrPtr,3)); // Line Counter
					  
							// printf("%d ", (unsigned int)getIntFromByte(wrPtr,4)); // Longitude
					  
							// printf("%d ", (unsigned int)getIntFromByte(wrPtr,4)); // Latitude

							// printf("%d ", (unsigned int)getIntFromByte(wrPtr,3)); // Altitude

							// printf("%d ", (unsigned int)getIntFromByte(wrPtr,2)); 
					  
							// printf("%d ", (unsigned int)getIntFromByte(wrPtr,3));

							// printf("%c", (char)getIntFromByte(wrPtr,1)); // 'E'
 
							// printf("%c", (char)getIntFromByte(wrPtr,1)); // 'N'

							// printf("%c\n", (char)getIntFromByte(wrPtr,1)); // 'D'
						// }
			}
			recvBuff[n] = 0;
			if(fputs(recvBuff, stdout) == EOF)
			{
			   printf("\n Error : Fputs error\n");
			}
			if(fileLineCount == 20){
				
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
				
				fileLineCount = 0;
				fileCount++;
				fclose(filePointer);
			}
			fileLineCount++;
			//packetCount++;
		}
		fclose(filePointer);
	}

    if(n < 0)
    {
        printf("\n Read error \n");
    } 

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
