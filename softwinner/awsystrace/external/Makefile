
CC = ./toolchain/gccsf/bin/arm-linux-gnueabi-c++

all : atrace libawtrace.so test_example 

atrace : 
	$(CC) -o atrace atrace.cpp

libawtrace.so :
	$(CC) -fPIC -shared -o libawtrace.so awtrace.cpp

test_example:
	$(CC) -L. -lawtrace -o awtrace_example example_02.cpp

.PHONY:clean
clean:
	rm *.o -f
	rm atrace -f
	rm libawtrace.so -f
	rm awtrace_example -f

