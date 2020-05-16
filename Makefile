CC=gcc
CCFLAGS=-std=c11 -g

pagetable : pageTable.o myPageTable.o
	$(CC) $(CCFLAGS) -o pagetable $^ -lm
	
pageTable.o : pageTable.h byutr.h pageTable.c

myPageTable.o : pageTable.h myPageTable.c

clean :
	rm -fr pagetable $(OBJS) *.o
