CC=mingw32-gcc
CXX=mingw32-g++
CFLAGS=-O2 -g -std=c99 -Wall
#SRC = $(wildcard *.c)
#OBJS=$(patsubst %.c,%.o, $(SRC))

.PHONY: clean all

all: bubbleSort insertSort shellSort heapSort mergeSort quicksort kselect

bubbleSort: bubbleSort.o utils.o
	$(CC) -o bubbleSort bubbleSort.o utils.o $(CFLAGS)

insertSort: insertSort.o utils.o
	$(CC) -o insertSort insertSort.o utils.o $(CFLAGS)

shellSort: shellSort.o utils.o
	$(CC) -o shellSort shellSort.o utils.o $(CFLAGS)
	
heapSort: heapSort.o utils.o
	$(CC) -o heapSort heapSort.o utils.o $(CFLAGS)
	
mergeSort: mergeSort.o utils.o
	$(CC) -o mergeSort mergeSort.o utils.o $(CFLAGS)

quicksort: quicksort.o utils.o
	$(CC) -o quicksort quicksort.o utils.o $(CFLAGS)
	
kselect: kselect.o utils.o
	$(CC) -o kselect kselect.o utils.o $(CFLAGS)

clean: 
	-rm *.exe *.o