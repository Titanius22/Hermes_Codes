#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	char stringToMatch[] = "Aabcdefghijklmnopqrstuvwxyz"
	char buffSubstring[27];
	char buffLine[sentenceLength + 1];
	

    fl = fopen (argv[1], "r");
    if (fl==NULL) {
        fprintf (stderr, "File error\n"); 
        exit (1);
    }

    while((fgets(buffLine, sentenceLength, fl) != NULL))
    {
		// resets tally to zero
		LocalFailTally = 0;
		
		// copys a substring and compares it to what is should be
		strncpy(buffSubstring, buffLine, 27);
		if (strncmp(buffSubstring+2, stringToMatch, 27) != 0) {LocalFailTally++;}
		
		// end of sentence should match
		if (buffLine[29] == 'E') {LocalFailTally++;}
		if (buffLine[30] == 'N') {LocalFailTally++;}
		if (buffLine[31] == 'D') {LocalFailTally++;}
		
		// if tally was incremented, there was an error
		if(LocalFailTally == 0)
		{
			SuccessfulCount++;
		}
		
		TotalSentenceCount++;
    }
	
	printf("Total sentence count: %d", TotalSentenceCount);
	printf("Number of Successful: %d", SuccessfulCount);
	printf("Number of failed: %d", TotalSentenceCount - SuccessfulCount);

    fclose (fl);
    return 0;
}