/**********************
***********************
Student 1: Huy Tran
Red ID: 818608122

Student 2: Hosun Yoo
Red ID: 819543212

CS-570
a04
pageTable.c
***********************
***********************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#include "pageTable.h"

extern int trace;

/*
 * PagetableInit():
 * initialize a new pagetable
 */
PAGETABLE* PagetableInit(int levelCount, int* bits)
{
	PAGETABLE* pt = calloc(1, sizeof(PAGETABLE));

	pt->LevelCount = levelCount;
	pt->BitmaskAry = calloc(levelCount, sizeof(unsigned int));
	pt->ShiftAry = calloc(levelCount, sizeof(int));
	pt->EntryCount = calloc(levelCount, sizeof(unsigned int));

	int totalBits = 0;
	int i;
	for (i=0; i<levelCount; i++) {
		totalBits += bits[i];
	}

	int shift = 32; // 32 bits address
	for (i=0; i < levelCount; i++) {
		// calculating # of entries, shifting bits, bit mask for each level
		unsigned int count = pow(2, bits[i]);
		shift -= bits[i];
		unsigned int bitMask = (count-1) << shift;

		pt->BitmaskAry[i] = bitMask;
		pt->ShiftAry[i] = shift;
		pt->EntryCount[i] = count;
	}
	// create level 0 and set the pagetable to point to this level
	pt->RootNodePtr = LevelInit(pt, 0);

	return pt;
}

/*
 * LevelInit():
 * initialize a new level
 */
LEVEL *LevelInit(PAGETABLE *pt, int depth)
{

	LEVEL* lv = calloc(1, sizeof (LEVEL));
	int i;

	lv->CurrentDepth = depth;
	lv->PageTable = pt;

	unsigned int entryCount = pt->EntryCount[depth];

	if ((depth+1) == pt->LevelCount) { 
		// if leaf node then allocate maps
		lv->Maps = calloc(entryCount, sizeof(MAP*));
		
	} else {
		// if not leaf node then allocate new level
		lv->NextLevel = calloc(entryCount, sizeof(LEVEL*));
	}
	return lv;
}

/*
 * If you are using this program on a big-endian machine (something
 * other than an Intel PC or equivalent) the unsigned longs will need
 * to be converted from little-endian to big-endian.
 *
 * taken from "byu_tracereader.c"
 */
uint32_t swap_endian(uint32_t num)
{
	return(((num << 24) & 0xff000000) | ((num << 8) & 0x00ff0000) |
			((num >> 8) & 0x0000ff00) | ((num >> 24) & 0x000000ff) );
}

/* 
 * determine if system is big- or little- endian
 * 
 * taken from "byu_tracereader.c"
 */
ENDIAN endian()
{
	/* Allocate a 32 bit character array and pointer which will be used
	 * to manipulate it.
	 */
	uint32_t *a;
	unsigned char p[4];

	a = (uint32_t *) p;  /* Let a point to the character array */
	*a = 0x12345678; /* Store a known bit pattern to the array */
	/* Check the first byte.  If it contains the high order bits,
	 * it is big-endian, otherwise little-endian.
	 */
	if(*p == 0x12)
		return BIG;
	else
		return LITTLE;
}

/*
 * PopulatePageTable():
 * build the pagetable data struct with address file
 */
void PopulatePageTable(FILE *add_fp, PAGETABLE* pt, int addressCount)

{
	// Start reading addresses
	p2AddrTr trace_item; // Structure with trace information
	int done = 0;
	int i;
	unsigned int frame = 0;

	// when a set number of addresses are processed (argument -n)
	if (addressCount != 0) { 
		for (i = 0; i < addressCount; i++) {
			// Grab the next address
			int bytesread = NextAddress(add_fp, &trace_item);
			MAP* map = PageLookUp(pt, trace_item.addr);
			// if PageLookUp() returns null, insert page into struct
			if (map == 0) {
				PageInsert(pt, trace_item.addr, frame);
                frame++; // increment frame counter if new map
			}
		}
	} else { // when all addresses are processed
		while (! done) {
			// Grab the next address
			int bytesread = NextAddress(add_fp, &trace_item);
			// Check if we actually got something
			done = (bytesread == 0);
			if (! done) {	
				MAP* map = PageLookUp(pt, trace_item.addr);
				if (map == 0) {
					PageInsert(pt, trace_item.addr, frame);
	               	frame++;
				} 
			}
		}
	}
}

/* 
 * NextAddress():
 * Fetch the next address from the trace.
 *
 * trace_file must be a file handle to an trace file opened
 * with fopen. User provides a pointer to an address structure.
 *
 * Populates the Addr structure and returns non-zero if successful.
 * 
 * taken from "byu_tracereader.c"
 */
int NextAddress(FILE *trace_file, p2AddrTr *addr_ptr) {

	int readN;    /* number of records stored */
	static ENDIAN byte_order = UNKNOWN;   /* don't know machine format */

	if (byte_order == UNKNOWN) {
		/* First invocation.  Determine if this is a litte- or
		 * big- endian machine so that we can convert bit patterns
		 * if needed that are stored in little-endian format
		 */
		byte_order = endian();
	}

	/* Read the next address record. */
	readN = fread(addr_ptr, sizeof(p2AddrTr), 1, trace_file);

	if (readN) {

		if (byte_order == BIG) {
			/* records stored in little endian format, convert */
			addr_ptr->addr = swap_endian(addr_ptr->addr);
			addr_ptr->time = swap_endian(addr_ptr->time);
		}
	}

	return readN;
}

/*
 * PageLookUp():
 * look up the page entry of the pagetable
 */
MAP* PageLookUp(PAGETABLE* pt, unsigned int addr)
{
	return PageLookUpLevel(pt->RootNodePtr, addr);
}

MAP* PageLookUpLevel(LEVEL* lv, unsigned int addr)
{
	PAGETABLE* pt = lv->PageTable;
	int depth = lv->CurrentDepth;

	unsigned int page = LogicalToPage(addr, pt->BitmaskAry[depth], pt->ShiftAry[depth]);

	// if leaf node, return the map entry
	if (depth == (pt->LevelCount-1)) {
		return lv->Maps[page]; // can be null (0)
	} else { // if not leaf node, recursive call function moving to next level
		if (lv->NextLevel[page] != 0) {
			return (PageLookUpLevel(lv->NextLevel[page], addr)); // recursive call
		} else {
			return 0;
		}
	}
}

/*
 * PageInsert():
 * add new entries to the page table when we have discovered that
 * a page has not yet been allocated (PageLookup returns NULL)
 */
void PageInsert(PAGETABLE* pt, unsigned int LogicalAddress, unsigned int Frame)
{
	PageInsertLevel(pt->RootNodePtr, LogicalAddress, Frame);
}

void PageInsertLevel(LEVEL* lv, unsigned int addr, unsigned int Frame)
{
	PAGETABLE* pt = lv->PageTable;
	int depth = lv->CurrentDepth;

	unsigned int page = LogicalToPage(addr, pt->BitmaskAry[depth], pt->ShiftAry[depth]);

	// if leaf node, allocate Map struct and its variables
	if (depth == (pt->LevelCount-1)) {
		MAP* map = calloc(1, sizeof(MAP));
		map->page = page;
		map->valid = true;
		map->frame = Frame;
		map->print = false;
		lv->Maps[page] = map;
	} else { // if not leaf node, recursive call function moving to next level
		if (lv->NextLevel[page] != 0) {
			PageInsertLevel(lv->NextLevel[page], addr, Frame);
		} else {
			lv->NextLevel[page] = LevelInit(pt, depth+1);
			PageInsertLevel(lv->NextLevel[page], addr, Frame);
		}
	}
}

/*
 * LogicalToPage():
 * for the given logical address return the page number
 */
unsigned int LogicalToPage(unsigned int LogicalAddress, unsigned int Mask, unsigned int Shift)
{
	return ((LogicalAddress & Mask) >> Shift);
}

/*
 * dumpPages():
 * process arguments -p and -t
 * dump the pagetable & logical to physical address translation
 */
void dumpPages(FILE* add_fp, PAGETABLE* pt, int addressCount, char* filename, int levelCount, int* bits){

	// Start reading addresses
	p2AddrTr trace_item; // Structure with trace information
	int done = 0, i;
	unsigned int offset, physicalAddress;
	FILE *outputFile = NULL;

	// calculate total number of bits to find offset
	int totalBits = 0;
	for (i=0; i<levelCount; i++) {
		totalBits += bits[i];
	}

 	if (filename != NULL) {
		outputFile = fopen(filename, "w");
	}

	// when a set number of addresses are processed (argument -n)
	if (addressCount != 0) {
		unsigned int address[addressCount];
		// save addresses into an array
		for (i = 0; i < addressCount; i++) {
			// Grab the next address
			int bytesread = NextAddress(add_fp, &trace_item);
			address[i] = trace_item.addr;
			// if argument -t is recognized, perform logical to physical address translation
			if (trace){
				MAP* map = PageLookUp(pt, address[i]);
				if (map != 0) {
					// calculate offset then physical address
					offset = (address[i] << totalBits) >> totalBits;
					physicalAddress = map->page * map->frame + offset;
					printf("%08x -> %08x\n", address[i], physicalAddress);
				}				
			}
		}

		// sort the array so pages can be printed out in ascending order
		int j, k, temp;
		for (j = 0; j < addressCount; j++) {
			for (k = j + 1; k < addressCount; k++) {
				if (address[j] > address[k]) {
					temp = address[j];
					address[j] = address[k];
					address[k] = temp;
				}
			}
		}

		// if argument -p is recognized, dump the pages and their frame
		if (filename != NULL) {
			for (i = 0; i < addressCount; i++) {
				MAP* map = PageLookUp(pt, address[i]);
				if (map != 0) {
					if (map->print == false) {
						fprintf(outputFile, "%08x -> %08x\n", map->page, map->frame);
						map->print = true;
					}
				}			
			}
		}
	} else { // process the rest
		while (! done) {
			// Grab the next address
			int bytesread = NextAddress(add_fp, &trace_item);
			done = (bytesread == 0);
			if (! done) {	
				
				MAP* map = PageLookUp(pt, trace_item.addr);
				if (map != 0) {
					if (trace) {
						offset = (trace_item.addr << totalBits) >> totalBits;
						physicalAddress = map->page * map->frame + offset;
						printf("%08x -> %08x\n", trace_item.addr, physicalAddress);
					}
					if (filename != NULL) {
						if (map->print == false) {
							fprintf(outputFile, "%08x -> %08x\n", map->page, map->frame);
							map->print = true;
						}
					}
				} 
			}
		}
	}
}