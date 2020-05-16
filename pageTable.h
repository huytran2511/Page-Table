#ifndef _PAGETABLE_H_
#define _PAGETABLE_H_

#include <stdbool.h>
#include "byutr.h"

#define BADFLAG -1

/*
 * PAGETABLE:
 * contains information about the tree
 * Top level descriptor describing attributes of the N level page table and containing a pointer
 * to the level 0 page structure.
 */
typedef struct {
	int LevelCount; // number of levels
	unsigned int *BitmaskAry; // bit mask array for level i
	int *ShiftAry; // # of bits to shift level i page bits
	unsigned int *EntryCount; // # of possible pages for level i
	struct LEVEL_ *RootNodePtr; // pointer to level 0
} PAGETABLE;

/*
 * MAP:
 * A structure containing information about the mapping of a page to a frame,
 * used in leaf nodes of the tree.
 */
typedef struct {
	int page; // page number
	bool valid;
	unsigned int frame; // frame index that the page number is mapped to
	bool print; // use for dumping pages
} MAP;

/*
 * LEVEL: 
 * An entry for an arbitrary level, this is the structure (orclass) which
 * represents one of the sublevels
 * a structure describing a specific level of the page table
 */
typedef struct LEVEL_ {
	int CurrentDepth; // depth of the current level
	struct LEVEL_ **NextLevel; // pointer to next level
	MAP **Maps; // array which maps from a logical page to a physical frame; leaf node
	PAGETABLE *PageTable; // to access information of pagetable
} LEVEL;

int* PopulateArgs(int argc, char **argv, int* lv_count, int* add_count, char** addressFileName, char** outputFileName);

LEVEL *LevelInit(PAGETABLE *pt, int depth);

PAGETABLE* PagetableInit(int levelCount, int* bits);

void PopulatePageTable(FILE *add_fp, PAGETABLE* pt, int addressCount);

MAP* PageLookUp(PAGETABLE* pt, unsigned int addr);

void PageInsert(PAGETABLE* pt, unsigned int LogicalAddress, unsigned int Frame);

void PageInsertLevel(LEVEL* lv, unsigned int addr, unsigned int Frame);

unsigned int LogicalToPage(unsigned int LogicalAddress, unsigned int Mask, unsigned int Shift);

MAP* PageLookUpLevel(LEVEL* lv, unsigned int addr);

int NextAddress(FILE *trace_file, p2AddrTr *addr_ptr);

void dumpPages(FILE *add_fp, PAGETABLE* pt, int addressCount, char *filename, int levelCount, int* bits);

#endif
