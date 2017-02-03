#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Prototyping
void insertBytesFromInt(void* ,unsigned char** ,short){

int main(int argc, char *argv[])
{
    FILE * fl;
    long fl_size;
    size_t res;
	int line_counter = 0;
	int SuccessfulCount = 0;
	int TotalSentenceCount = 0;
	int LocalFailTally = 0;
	int sentenceLength = 32;
	char stringToMatch[32];
	char buffSubstring[27];
	char buffLine[sentenceLength + 1];
	
	///////////////////////////////////////////////REMOVE AFTER TEST///////////////////////////////////////////////////////////////////
	char* writeTo=stringToMatch;
	
	//Line counter-------------------------------------------
	int intBuflineCount = 150;
	insertBytesFromInt(&intBuflineCount, &writeTo, 3);

	//Latitude 
	long longBuflatitude = (unsigned long)(29.172045 * 1000000);
	insertBytesFromInt(&longBuflatitude, &writeTo, 5);

	//Longitude
	long longBuflongitude = (unsigned long)(81.078736 * 1000000);
	insertBytesFromInt(&longBuflongitude, &writeTo, 5);

	//Altitude * 100--------------------------------------------
	int intBufaltitude = 1000 * 100;
	insertBytesFromInt(&intBufaltitude, &writeTo, 3);

	//Thermistor count------------------------------------------
	int intBuftemperature = 450;
	insertBytesFromInt(&intBuftemperature, &writeTo, 2);

	//Battery Voltage---------------------------------------------
	int intBufpressure = 120;
	insertBytesFromInt(&intBufpressure, &writeTo, 1);

	//Magnotometer X---------------------------------------------
	int intBufpressure2 = 80;
	insertBytesFromInt(&intBufpressure2, &writeTo, 1);

	//Magnotometer Y---------------------------------------------
	int intBufpressure3 = 60;
	insertBytesFromInt(&intBufpressure3, &writeTo, 1);

	//Magnotometer Z---------------------------------------------
	int intBufpressure4 = 40;
	insertBytesFromInt(&intBufpressure4, &writeTo, 1);

	//Humidity---------------------------------------------
	int intBufpressure5 = 96;
	insertBytesFromInt(&intBufpressure5, &writeTo, 1);

	//Pressure---------------------------------------------
	int intBufpressure6 = 102300;
	insertBytesFromInt(&intBufpressure6, &writeTo, 4);

	//Internal Temperature---------------------------------------------
	int intBufpressure7 = 15;
	insertBytesFromInt(&intBufpressure7, &writeTo, 2);

	//End of line chars-------------------------------------------

	recvBuf[29] = 'E';
	recvBuf[30] = 'N';
	recvBuf[31] = 'D';
	/////////////////////////////////////////////////////////REMOVE AFTER TEST////////////////////////////////////////////////////////////
	

    fl = fopen (argv[1], "r");
    if (fl==NULL) {
        fprintf (stderr, "File error\n"); 
        exit (1);
    }

    while((fgets(buffLine, sentenceLength+1, fl) != NULL))
    {
		// resets tally to zero
		LocalFailTally = 0;
		
		// copys a substring and compares it to what is should be
		strncpy(buffSubstring, buffLine+3, 26);
		
		printf("REMOVE ME. String is %s", buffSubstring);
		
		if (strncmp(buffSubstring, stringToMatch, 26) != 0) {LocalFailTally++;}
		
		// end of sentence should match
		if (buffLine[29] != 'E') {LocalFailTally++;}
		if (buffLine[30] != 'N') {LocalFailTally++;}
		if (buffLine[31] != 'D') {LocalFailTally++;}
		
		// if tally was incremented, there was an error
		if(LocalFailTally == 0)
		{
			SuccessfulCount++;
		}
		
		TotalSentenceCount++;
    }
	
	printf("Total sentence count: %d\n", TotalSentenceCount);
	printf("Number of Successful: %d\n", SuccessfulCount);
	printf("Number of failed: %d\n", TotalSentenceCount - SuccessfulCount);

    fclose (fl);
    return 0;
}

// Converts whole integer values into chars
void insertBytesFromInt(void* value,unsigned char** byteStart, short numberBytesToCopy){

  unsigned char* valueBytes=value;
  short loopCount=0;
  for(loopCount=0;loopCount<numberBytesToCopy;loopCount++){
    (*byteStart)[loopCount]=valueBytes[loopCount];
  }
  *byteStart+=(short)numberBytesToCopy;
}