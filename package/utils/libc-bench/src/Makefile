TARGET := libc-bench

commonSources := \
	benchmark_main.cpp \
	math_benchmark.cpp \
	pthread_benchmark.cpp \
	semaphore_benchmark.cpp \
	stdio_benchmark.cpp \
	string_benchmark.cpp \
	time_benchmark.cpp \
	unistd_benchmark.cpp

local_CFLAGS := \
    -O2 \
    -Wall -Wextra -Wunused \
    -Werror \
    -fno-builtin \
    -std=gnu++11 \

OBJS = $(patsubst %.cpp, %.cpp.o, $(commonSources))

$(TARGET):$(OBJS)
	$(CXX) $(CFLAGS) $(LDFLAGS) -lm $(local_CFLAGS) -o $@ $^

$(OBJS):%.cpp.o:%.cpp
	$(CXX) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) -c $^ -o $@

all: $(TARGET) install

clean:
	-rm $(OBJS) $(TARGET)
install:
	mkdir -p $(INSTALL_PREFIX)/usr/bin
	cp $(TARGET) $(INSTALL_PREFIX)/usr/bin
