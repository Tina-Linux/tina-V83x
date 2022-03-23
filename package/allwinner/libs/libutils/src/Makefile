TARGET := libutils.so
LIBS := -lcutils

commonSources:= \
	BasicHashtable.cpp \
	BlobCache.cpp \
	JenkinsHash.cpp \
	LinearAllocator.cpp \
	LinearTransform.cpp \
	NativeHandle.cpp \
	Printer.cpp \
	PropertyMap.cpp \
	RefBase.cpp \
	SharedBuffer.cpp \
	Static.cpp \
	StopWatch.cpp \
	String8.cpp \
	String16.cpp \
	SystemClock.cpp \
	Threads.cpp \
	Timers.cpp \
	Tokenizer.cpp \
	Unicode.cpp \
	VectorImpl.cpp \
	misc.cpp \
	FileMap.cpp
	#CallStack.cpp \
	ProcessCallStack.cpp \

local_CFLAGS := -I./include -DHAVE_ENDIAN_H -std=gnu++11 \
	-D_FILE_OFFSET_BITS=64 -DHAVE_PTHREADS -DHAVE_POSIX_FILEMAP -DANDROID_SMP \
	-fkeep-inline-functions

OBJS = $(patsubst %.cpp, %.cpp.o, $(commonSources))

$(TARGET):$(OBJS)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(LIBS) $(local_CFLAGS) -o $@ $^ -fPIC -shared

$(OBJS):%.cpp.o:%.cpp
	$(CXX) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) -c $< -o $@ -fPIC -shared

all: $(TARGET) install

clean:
	-rm $(OBJS) $(TARGET)
install:
	mkdir -p $(INSTALL_PREFIX)/usr/lib
	cp $(TARGET) $(INSTALL_PREFIX)/usr/lib
	mkdir -p $(INSTALL_PREFIX)/usr/include
	cp -r include/*  $(INSTALL_PREFIX)/usr/include
