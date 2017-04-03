#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Prototyping
unsigned long getIntFromByte(unsigned char** ,short);
void insertBytesFromInt(void* ,unsigned char** , short);
void SetNewData(short);
void updateLineCounter();
void TripleData();

//Globals
char buffer[300];
int dataLineLength = 29;
int numberLinesToSend = 1;
int dataSpeed = 90000; // bps
int transmitTime = 100; //21600; //6 hours * 60 mins * 60 seconds
int packetsToSend;

unsigned short counter = 0;

int main(int argc, char *argv[])
{
	int i;
	int other;
	packetsToSend = ((dataSpeed*transmitTime)/(8*dataLineLength)) +1; // +1 because the vaues always round down
    FILE *f = fopen("fakeData.bin", "w+");
	printf("%d\n", packetsToSend);
	if (f == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}



	for(i=0;i<packetsToSend;i++){
		other = i/1000;
		SetNewData(other);
		updateLineCounter();
		fwrite(buffer, 29, 1, f);
	}


	fclose(f);
    return 0;
}

// Updates the lineCounts
void updateLineCounter(){

	unsigned char* writeTo=buffer;
	int i;
	unsigned char* ptrVar;

	// repeat data for the second 2 lines of the 87 byte (3 x 29) transmission
	for(i=0;i<numberLinesToSend;i++){
		//Line counter-------------------------------------------
		ptrVar = &writeTo[0]+(dataLineLength*i);
		insertBytesFromInt(&counter, &ptrVar, 2);
		counter++;
	}
}

//  put new data in buffer
void SetNewData(short pick){

	unsigned char* writeTo=buffer;
	int modVal = pick%24;

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
	if(modVal < 12){
		insertBytesFromInt(&(intBuflatitude[modVal]), &writeTo, 3);
	}
	else{
		insertBytesFromInt(&(intBuflatitude[23-modVal]), &writeTo, 3);
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
	if(modVal < 12){
		insertBytesFromInt(&(intBuflongitude[modVal]), &writeTo, 3);
	}
	else{
		insertBytesFromInt(&(intBuflongitude[23-modVal]), &writeTo, 3);
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
	insertBytesFromInt(&(intBufaltitude[modVal]), &writeTo, 3);

	//Time(secs since UTC half day)--------------------------------------------
	unsigned int intBufTime = pick;
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

	buffer[dataLineLength-3] = 'E';
	buffer[dataLineLength-2] = 'N';
	buffer[dataLineLength-1] = 'D';

	TripleData();
}

// Duplicate the data to multiple lines
void TripleData(){
	int i;
	int j;
	// repeat data for the second 2 lines of the 96 byte (3 x 32) transmission
	for(i=0;i<dataLineLength;i++){
		for(j=0;j<(numberLinesToSend-1);j++){
			buffer[i+(dataLineLength*j)] = buffer[i];
		}
	}
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
