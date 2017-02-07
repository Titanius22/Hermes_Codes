#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Prototyping
void insertBytesFromInt(void* ,unsigned char** ,short);

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

	stringToMatch[29] = 'E';
	stringToMatch[30] = 'N';
	stringToMatch[31] = 'D';
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
		//strncpy(buffSubstring, buffLine+3, 26);
		
		//if (strncmp(buffSubstring, stringToMatch, 26) != 0) {LocalFailTally++;}
		
		// Match char by char
		if (buffLine[3] != stringToMatch[3]) {LocalFailTally++;}
		if (buffLine[4] != stringToMatch[4]) {LocalFailTally++;}
		if (buffLine[5] != stringToMatch[5]) {LocalFailTally++;}
		if (buffLine[6] != stringToMatch[6]) {LocalFailTally++;}
		if (buffLine[7] != stringToMatch[7]) {LocalFailTally++;}
		if (buffLine[8] != stringToMatch[8]) {LocalFailTally++;}
		if (buffLine[9] != stringToMatch[9]) {LocalFailTally++;}
		if (buffLine[10] != stringToMatch[10]) {LocalFailTally++;}
		if (buffLine[11] != stringToMatch[11]) {LocalFailTally++;}
		if (buffLine[12] != stringToMatch[12]) {LocalFailTally++;}
		if (buffLine[13] != stringToMatch[13]) {LocalFailTally++;}
		if (buffLine[14] != stringToMatch[14]) {LocalFailTally++;}
		if (buffLine[15] != stringToMatch[15]) {LocalFailTally++;}
		if (buffLine[16] != stringToMatch[16]) {LocalFailTally++;}
		if (buffLine[17] != stringToMatch[17]) {LocalFailTally++;}
		if (buffLine[18] != stringToMatch[18]) {LocalFailTally++;}
		if (buffLine[19] != stringToMatch[19]) {LocalFailTally++;}
		if (buffLine[20] != stringToMatch[20]) {LocalFailTally++;}
		if (buffLine[21] != stringToMatch[21]) {LocalFailTally++;}
		if (buffLine[22] != stringToMatch[22]) {LocalFailTally++;}
		if (buffLine[23] != stringToMatch[23]) {LocalFailTally++;}
		if (buffLine[24] != stringToMatch[24]) {LocalFailTally++;}
		if (buffLine[25] != stringToMatch[25]) {LocalFailTally++;}
		if (buffLine[26] != stringToMatch[26]) {LocalFailTally++;}
		if (buffLine[27] != stringToMatch[27]) {LocalFailTally++;}
		if (buffLine[28] != stringToMatch[28]) {LocalFailTally++;}
		
		// end of sentence should match
		if (buffLine[29] != stringToMatch[29]) {LocalFailTally++;}
		if (buffLine[30] != stringToMatch[30]) {LocalFailTally++;}
		if (buffLine[31] != stringToMatch[31]) {LocalFailTally++;}
		
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