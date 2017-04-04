/*
fread example: read an entire file
http://www.cplusplus.com/reference/cstdio/fread/
 */
#include <stdio.h>
#include <stdlib.h>

//Prototype
void writeLine(long);
unsigned long getIntFromByte(unsigned char** , short );
void insertBytesFromInt(void* ,unsigned char** , short );

//Globals
FILE * csvFile;
long lSize;
int lineLength = 29;
char * buffer;

int main () {
    FILE * pFile;
    unsigned long readCounter = 0;
    size_t result;
    long bufferIndex = 0;
    long mathVar;
    long lastGoodEnd = -1;

    pFile = fopen ( "fakeData.bin" , "r" );
    if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

    csvFile = fopen ( "csvData.csv" , "w+" );
    if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

    // obtain file size:
    fseek (pFile , 0 , SEEK_END);
    lSize = ftell (pFile);
    printf("lSize: %d\n", lSize);
    rewind (pFile);

    // allocate memory to contain the whole file:
    buffer = (char*) malloc (sizeof(char)*lSize);
    if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

    // copy the file into the buffer:
    while((result > 0) && (readCounter < lSize)){
        result = fread(&buffer[readCounter],1,lSize,pFile);
        //printf("readCounter: %d\n", readCounter);
        if(result > 0){
            readCounter += result;
            //printf("result: %d\n", result);
            //fseek (pFile ,  , SEEK_CUR);
        }
        else if(result == 0){
            //while(feof(pFile)){
                //printf("readCounter: %d\n", readCounter);
                buffer[readCounter] = fgetc(pFile);
                //printf("buffer char: %c\n", buffer[readCounter]);
                fseek (pFile , 1 , SEEK_CUR);
                readCounter++;
                result = 1;
            //}
            //printf("Loop\n");
        }
        else{break;}
    }
    printf("result: %d\n", result);
    if (readCounter != lSize) {fputs ("Reading error",stderr); exit (3);}

    /* the whole file is now loaded in the memory buffer. */
    //printf("%c %c %c", buffer[readCounter-3], buffer[readCounter-2], buffer[readCounter-1]);

    // put data into csv format
    for(bufferIndex=0; bufferIndex < readCounter; bufferIndex++){
        //printf("%c\n", buffer[bufferIndex]);
        if((buffer[bufferIndex] == 'E') && (buffer[bufferIndex+1] == 'N') && (buffer[bufferIndex+2] == 'D')){
            if((bufferIndex+2) < lineLength){
                mathVar = bufferIndex - (lineLength - 3);
                writeLine(mathVar);
                lastGoodEnd = bufferIndex+2;
            }else if((buffer[bufferIndex-lineLength] == 'E') && (buffer[bufferIndex+1-lineLength] == 'N') && (buffer[bufferIndex+2-lineLength] == 'D')){
                mathVar = bufferIndex - (lineLength - 3);
                writeLine(mathVar);
                lastGoodEnd = bufferIndex+2;
            }else{
                /*

                do something with the data from buffer[lastGoodEnd+1] to buffer[bufferIndex+2]
                lastGoodEnd = bufferIndex+2;

                */
            }
        }
    }

    // terminate
    fclose (pFile);
    free (buffer);
    return 0;
}


// writes line to file
void writeLine(long mathVar){
    unsigned char* writeArray;
	unsigned char** wrPtr;
	unsigned int DataLineCounter;
	unsigned int DataGPS[4];
	unsigned int DataSensors[9];
	char DataEndLine[3];

    writeArray = buffer;
    wrPtr=&writeArray;

    DataLineCounter = (unsigned int)getIntFromByte(wrPtr,2);
    //printf("%d ", DataLineCounter); // Line Counter

    DataGPS[0] = (unsigned int)getIntFromByte(wrPtr,3);
    //printf("%d ", DataGPS[0]); // Longitude

    DataGPS[1] = (unsigned int)getIntFromByte(wrPtr,3);
    //printf("%d ", DataGPS[1]); // Latitude

    DataGPS[2] = (unsigned int)getIntFromByte(wrPtr,3);
    //printf("%d ", DataGPS[2]); // Altitude

    DataGPS[3] = (unsigned int)getIntFromByte(wrPtr,2);
    //printf("%d ", DataGPS[3]); // Seconds since half UTC day

    DataSensors[0] = (unsigned int)getIntFromByte(wrPtr,2);
    //printf("%d ", DataSensors[0]); // External Thermistor

    DataSensors[1] = (unsigned int)getIntFromByte(wrPtr,1);
    //printf("%d ", DataSensors[1]); // Battery Voltage

    DataSensors[2] = (unsigned int)getIntFromByte(wrPtr,1);
    //printf("%d ", DataSensors[2]); // Battery Current

    DataSensors[3] = (unsigned int)getIntFromByte(wrPtr,1);
    //printf("%d ", DataSensors[3]); // Magnotometer X

    DataSensors[4] = (unsigned int)getIntFromByte(wrPtr,1);
    //printf("%d ", DataSensors[4]); // Magnotometer Y

    DataSensors[5] = (unsigned int)getIntFromByte(wrPtr,1);
    //printf("%d ", DataSensors[5]); // Magnotometer Z

    DataSensors[6] = (unsigned int)getIntFromByte(wrPtr,1);
    //printf("%d ", DataSensors[6]); // Humidity

    DataSensors[7] = (unsigned int)getIntFromByte(wrPtr,3);
    //printf("%d ", DataSensors[7]); // Pressure

    DataSensors[8] = (unsigned int)getIntFromByte(wrPtr,2);
    //printf("%d ", DataSensors[8]); // Internal Temperature

    DataEndLine[0] = (char)getIntFromByte(wrPtr,1);
    //printf("%c", DataEndLine[0]); // 'E'

    DataEndLine[1] = (char)getIntFromByte(wrPtr,1);
    //printf("%c", DataEndLine[1]); // 'N'

    DataEndLine[2] = (char)getIntFromByte(wrPtr,1);
    //printf("%c\n", DataEndLine[2]); // 'D'

    fprintf(csvFile, "%d,%d,%d,%d,%d,%d,%d%d,%d,%d,%d,%d,%d,%d,%c,%c,%c\n", DataLineCounter, DataGPS[0], DataGPS[1], DataGPS[2], DataGPS[3], DataSensors[0], DataSensors[1], DataSensors[2], DataSensors[3], DataSensors[4], DataSensors[5], DataSensors[6], DataSensors[7], DataSensors[8], DataEndLine[0], DataEndLine[1], DataEndLine[2]);
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





// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

////Prototyping
// void insertBytesFromInt(void* ,unsigned char** ,short);

// int main(int argc, char *argv[])
// {
    // FILE * fl;
    // long fl_size;
    // size_t res;
	// int line_counter = 0;
	// int SuccessfulCount = 0;
	// int TotalSentenceCount = 0;
	// int LocalFailTally = 0;
	// int sentenceLength = 32;
	// char stringToMatch[32];
	// char buffSubstring[27];
	// char buffLine[sentenceLength + 1];

	/////////////////////////////////////////////REMOVE AFTER TEST///////////////////////////////////////////////////////////////////
	// char* writeTo=stringToMatch;

	////Line counter-------------------------------------------
	// int intBuflineCount = 150;
	// insertBytesFromInt(&intBuflineCount, &writeTo, 3);

	////Latitude
	// long longBuflatitude = (unsigned long)(30.336944 * 1000000);
	// insertBytesFromInt(&longBuflatitude, &writeTo, 5);

	////Longitude
	// long longBuflongitude = (unsigned long)(81.661389 * 1000000);
	// insertBytesFromInt(&longBuflongitude, &writeTo, 5);

	////Altitude * 100--------------------------------------------
	// int intBufaltitude = 1000 * 100;
	// insertBytesFromInt(&intBufaltitude, &writeTo, 3);

	////Thermistor count------------------------------------------
	// int intBuftemperature = 450;
	// insertBytesFromInt(&intBuftemperature, &writeTo, 2);

	////Battery Voltage---------------------------------------------
	// int intBufpressure = 120;
	// insertBytesFromInt(&intBufpressure, &writeTo, 1);

	////Magnotometer X---------------------------------------------
	// int intBufpressure2 = 80;
	// insertBytesFromInt(&intBufpressure2, &writeTo, 1);

	////Magnotometer Y---------------------------------------------
	// int intBufpressure3 = 60;
	// insertBytesFromInt(&intBufpressure3, &writeTo, 1);

	////Magnotometer Z---------------------------------------------
	// int intBufpressure4 = 40;
	// insertBytesFromInt(&intBufpressure4, &writeTo, 1);

	////Humidity---------------------------------------------
	// int intBufpressure5 = 96;
	// insertBytesFromInt(&intBufpressure5, &writeTo, 1);

	////Pressure---------------------------------------------
	// int intBufpressure6 = 102300;
	// insertBytesFromInt(&intBufpressure6, &writeTo, 4);

	////Internal Temperature---------------------------------------------
	// int intBufpressure7 = 15;
	// insertBytesFromInt(&intBufpressure7, &writeTo, 2);

	////End of line chars-------------------------------------------

	// stringToMatch[29] = 'E';
	// stringToMatch[30] = 'N';
	// stringToMatch[31] = 'D';
	///////////////////////////////////////////////////////REMOVE AFTER TEST////////////////////////////////////////////////////////////


    // fl = fopen (argv[1], "r");
    // if (fl==NULL) {
        // fprintf (stderr, "File error\n");
        // exit (1);
    // }

    // while((fgets(buffLine, sentenceLength+1, fl) != NULL))
    // {
		////resets tally to zero
		// LocalFailTally = 0;

		////copys a substring and compares it to what is should be
		////strncpy(buffSubstring, buffLine+3, 26);

		////if (strncmp(buffSubstring, stringToMatch, 26) != 0) {LocalFailTally++;}

		////Match char by char
		// if (buffLine[3] != stringToMatch[3]) {LocalFailTally++;}
		// if (buffLine[4] != stringToMatch[4]) {LocalFailTally++;}
		// if (buffLine[5] != stringToMatch[5]) {LocalFailTally++;}
		// if (buffLine[6] != stringToMatch[6]) {LocalFailTally++;}
		// if (buffLine[7] != stringToMatch[7]) {LocalFailTally++;}
		// if (buffLine[8] != stringToMatch[8]) {LocalFailTally++;}
		// if (buffLine[9] != stringToMatch[9]) {LocalFailTally++;}
		// if (buffLine[10] != stringToMatch[10]) {LocalFailTally++;}
		// if (buffLine[11] != stringToMatch[11]) {LocalFailTally++;}
		// if (buffLine[12] != stringToMatch[12]) {LocalFailTally++;}
		// if (buffLine[13] != stringToMatch[13]) {LocalFailTally++;}
		// if (buffLine[14] != stringToMatch[14]) {LocalFailTally++;}
		// if (buffLine[15] != stringToMatch[15]) {LocalFailTally++;}
		// if (buffLine[16] != stringToMatch[16]) {LocalFailTally++;}
		// if (buffLine[17] != stringToMatch[17]) {LocalFailTally++;}
		// if (buffLine[18] != stringToMatch[18]) {LocalFailTally++;}
		// if (buffLine[19] != stringToMatch[19]) {LocalFailTally++;}
		// if (buffLine[20] != stringToMatch[20]) {LocalFailTally++;}
		// if (buffLine[21] != stringToMatch[21]) {LocalFailTally++;}
		// if (buffLine[22] != stringToMatch[22]) {LocalFailTally++;}
		// if (buffLine[23] != stringToMatch[23]) {LocalFailTally++;}
		// if (buffLine[24] != stringToMatch[24]) {LocalFailTally++;}
		// if (buffLine[25] != stringToMatch[25]) {LocalFailTally++;}
		// if (buffLine[26] != stringToMatch[26]) {LocalFailTally++;}
		// if (buffLine[27] != stringToMatch[27]) {LocalFailTally++;}
		// if (buffLine[28] != stringToMatch[28]) {LocalFailTally++;}

		////end of sentence should match
		// if (buffLine[29] != stringToMatch[29]) {LocalFailTally++;}
		// if (buffLine[30] != stringToMatch[30]) {LocalFailTally++;}
		// if (buffLine[31] != stringToMatch[31]) {LocalFailTally++;}

		////if tally was incremented, there was an error
		// if(LocalFailTally == 0)
		// {
			// SuccessfulCount++;
		// }

		// TotalSentenceCount++;
    // }

	// printf("Total sentence count: %d\n", TotalSentenceCount);
	// printf("Number of Successful: %d\n", SuccessfulCount);
	// printf("Number of failed: %d\n", TotalSentenceCount - SuccessfulCount);

    // fclose (fl);
    // return 0;
// }

////Converts whole integer values into chars
// void insertBytesFromInt(void* value,unsigned char** byteStart, short numberBytesToCopy){

	// unsigned char* valueBytes=value;
	// short loopCount=0;
	// for(loopCount=0;loopCount<numberBytesToCopy;loopCount++){
		// (*byteStart)[loopCount]=valueBytes[loopCount];
	// }
	// *byteStart+=(short)numberBytesToCopy;
// }
