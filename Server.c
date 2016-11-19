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

int main(int argc, char *argv[])
{
    char letter=0;
	int letterCount;
	int lineCount;
	int fileCount;
	char Data[200];
	srand(time(NULL));
	char fileCounter[8];
	
	int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    char sendBuff[1025];
    time_t ticks; 

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10); 

    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 

        ticks = time(NULL);
        //snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
        //write(connfd, sendBuff, strlen(sendBuff));
		
		Data[0] = "1";
		for(letterCount=1;letterCount<199;letterCount++){
				letter=rand()%(127-32)+32;
				Data[letterCount] = letter;
		}
		Data[letterCount+1] = "\n";
		write(connfd, Data, 200);
		
		for(lineCount=1;lineCount<50;lineCount++){
				sprintf(fileCounter, "%d", lineCount+1);
				Data[0] = ltostr();
				write(connfd, Data, 200);
		}
		
		for(fileCount=1;fileCount<60;fileCount++){
			for(lineCount=0;lineCount<50;lineCount++){
				sprintf(fileCounter, "%d", lineCount);
				Data[0] = fileCounter;
				write(connfd, Data, 200);
			}
		}

        close(connfd);
        sleep(1);
     }
}