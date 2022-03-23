
# the compiler: gcc for C program, define as g++ for C++
#CC      ?= g++

#ARM32
#CC=arm-linux-androideabi-g++
#ARM64
#CC=aarch64-linux-android-g++
#x86
#CC=i686-linux-android-g++
#x86_64
#CC=x86_64-linux-android-g++
#mips
#CC=mipsel-linux-android-g++
#mips64
#CC=mips64el-linux-android-g++


INCLUDES += ./
RM = rm -f

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
LOCAL_CFLAGS = -Bstatic

# the build target executable:
TARGET = rtwpriv

all: $(TARGET)
$(TARGET): rtwpriv.o rtw_api.o
	-@mkdir -p $(CONFIG_PREFIX)
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -I$(INCLUDES) $(LDFLAGS) rtwpriv.o rtw_api.o -lm -lgcc -lgcc_eh -o $@
	@cp $@ $(CONFIG_PREFIX)/rtwpriv

rtwpriv.o: rtw_api.h rtwpriv.h
	$(CC) -I$(INCLUDES) $(CFLAGS) $(LOCAL_CFLAGS) -c rtwpriv.cpp

rtw_api.o: rtw_api.h rtwpriv.h
		$(CC) -I$(INCLUDES) $(CFLAGS) $(LOCAL_CFLAGS) -c rtw_api.cpp

#hwtx.o: rtw_pmact.h
#	$(CC) -I$(INCLUDE) $(CFLAGS) $(LOCAL_CFLAGS) -c hwtx.c

clean:
		$(RM) $(TARGET) *.o rtw_api.o
