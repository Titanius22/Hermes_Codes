
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>


//argc: the number of argments sent to main()
//argv: the actual arguments
int main(void) { 

	char letter=0;
	int letterCount;
	int lineCount;
	char Data[200][50];
	srand(time(null));
	for(lineCount=0;lineCount<200;lineCount++){
		for(letterCount=0;letterCount<49;letterCount++){
			letter=rand()%(127-32)+32;
			Data[lineCount][letterCount] = letter;
		}
		Data[lineCount][letterCount] = "\n";
	}
	
	char filepath[] =  "/home/alarm/SendFiles/cCodeTests/";
	char fileName[] = "cTestPacket";
	char fileExt[] = ".txt";
	char fullFilePath[60];
	FILE *filePointer;
	int numOfiles = 60;
	int fileCount = 0;
	
	for (fileCount=1;fileCount<(numOfiles+1);fileCount++){
		
		strcat(fullFilePath, filepath);
		strcat(fullFilePath, cTestPacket);
		strcat(fullFilePath, itoa(fileCount));
		strcat(fullFilePath, fileExt);
		
		filePointer = fopen(fullFilePath, "a");
	
		if (filePointer != NULL)
		{
			fwrite(&Data, 200, 50, filePointer);
			fclose(filePointer);
		}
		
		fullFilePath = NULL;
	}
	
	return 0;
}