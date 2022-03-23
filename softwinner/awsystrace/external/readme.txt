
1. atrace.cpp  
how to compile:
arm-linux-gnueabihf-gcc -o atrace atrace.cpp -static
arm-linux-gnueabihf-gcc -o atrace atrace.cpp
push atrace to target

2. example_01.c  
how to compile: 
	arm-linux-gnueabihf-gcc -o awsystrace_test example_01.c -static 
	arm-linux-gnueabihf-gcc -o awsystrace_test example_01.c
Note: the compiler(arm-linux-gnueabihf-c++) must be the same with target-board

push awsystrace_test to target
The scene-code which try to trace as below:
	atrace_begin(1, "AWSYSTRACE-TEST");
	printf("AWSYSTRACE-TEST\n");
	sleep(1);
	atrace_end(1);

About atrace_begin and atrace_end, please refer to android source-code:
system/core/include/cutils/trace.h

3. awtrace.h awtrace.cpp example_02.c
how to compile: 
	arm-linux-gnueabihf-c++ -o atest awtrace.cpp example_02.c -static
	arm-linux-gnueabihf-c++ -o atest awtrace.cpp example_02.c
Note: the compiler(arm-linux-gnueabihf-c++) must be the same with target-board

To test three Macro: ATRACE_ACLL atrace_begin atrace_end
About ATRACE_ACLL(), please refer to android source-code:
system/core/include/utils/Trace.h
Test-step:
(1). adb push atest /tmp
(2). in target machine, execute: chomd 777 /tmp/atest; /tmp/atest
(3). in host PC, execute: python -B parser_engine.py -t 8 -b 4096 sched freq
About the result: please refer to ftrace_orig.log  and result.html in current directory.

4. dynamic compile
please use this command: 
./toolchain/gcchf/bin/arm-linux-gnueabihf-c++ -o example02_dynamic example_02.c awtrace.cpp

5. How to used with .so
compile dynamic library: ./toolchain/gcchf/bin/arm-linux-gnueabihf-c++ -fPIC -shared -o libawtrace.so awtrace.cpp
compile executable file: ./toolchain/gcchf/bin/arm-linux-gnueabihf-c++ -L. -lawtrace -o example02_dynamic example_02.c
Get : dynamic-lib libawtrace.so and dynamic-test-executable-file example02_dynamic
currently will get Error: example02_dynamic: error while loading shared libraries: /lib/libawtrace.so: internal error

how to fix:
(1). use "readelf -h xxx" to check "soft-float ABI" or "hard-float ABI"
(2). please guarantee to use the same "xxxx-float ABI"
As above, in target board, default use "soft-float ABI". But here using "hard-float ABI"
to compile, so get such error.

