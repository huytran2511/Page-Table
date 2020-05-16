/**********************
***********************
Student 1: Huy Tran
Red ID: 818608122

Student 2: Hosun Yoo
Red ID: 819543212

CS-570
a04
myPageTable.c
***********************
***********************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#include "pageTable.h"

int trace = 0; // flag for argument -t

int main(int argc, char **argv)
{

	if (argc >= 3) { //make sure correct number of args provided

		int levelCount, addressCount;
		char *addressFileName, *outputFileName = NULL;

		// process the arguments from command line
		int* bits = PopulateArgs(argc, argv, &levelCount, &addressCount, &addressFileName, &outputFileName);
		if (bits == 0) {
			exit(0);
		}
		
		//initialize a pagetable
		PAGETABLE* pt = PagetableInit(levelCount, bits);

		FILE *addressFile = fopen(addressFileName, "r");
		// check if File is able to open
		if (addressFile == NULL) {
			printf("file \"%s\" doesn't exist!\n", addressFileName);
			exit(0);
		}
        
		// build pagetable data struct
		PopulatePageTable(addressFile, pt, addressCount);

		// move the file pointer back to the beginning
		fseek(addressFile, 0, SEEK_SET);

		// dump the pagetable & logical to physical address translation
		// (process arguments -p and -t)
		dumpPages(addressFile, pt, addressCount, outputFileName, levelCount, bits);

		fclose(addressFile);

	} else {

		printf("not enough arguments.\n");
		exit(0);

	}

	return 0;
}

/*
 * return: bits (number of levels and entryCount for each level)
 */
int* PopulateArgs(int argc, char **argv, int* lv_count, int* add_count, char** addressFileName, char** outputFileName)
{
	int Option, addressCount = 0;

	while ((Option = getopt(argc, argv, "n:p:t")) != -1) {
		switch (Option) { // TODO: options
		case 'n': /* Number of addresses to process */
			addressCount = atoi(optarg);
			break;
		case 'p': /* produce map of pages */
			*outputFileName = optarg;
			break;
		case 't': /* Show address translation */
			// No argument this time, just set a flag
			trace = 1;
			break;
		default:
			// print something about the usage and die...
			exit(BADFLAG); // BADFLAG is an error # defined in a header
		}
	}

	int levelCount = (argc-optind)-1; // first one is "addressFileName"

	// check if level count is valid 
	if (levelCount < 1 || levelCount > 31) {
		printf("levelCount should be 1~31: you have %d\n", levelCount);
		return 0;
	}

	*addressFileName = argv[optind];
	int* bits = calloc(levelCount, sizeof(int)); 
	int levelIndex = 0;

	// save number of bits for each level into bits[]
	// first optind is for addressFileName, for loop starts after that
	int argIndex;
	for (argIndex = optind + 1; argIndex < argc; argIndex++) {
		bits[levelIndex++] = atoi(argv[argIndex]);
	}
	*add_count = addressCount;	
	*lv_count = levelCount;

	return bits;
}
