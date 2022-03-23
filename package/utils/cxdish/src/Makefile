
# Which compiler
CC = $(CROSS_COMPILE)g++

#Options for development
CFLAGS= -Wall -ansi
LDFLAGS= -static
SOURCES=cxdish.cpp host_depend.cpp CxFlash.c
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=cxdish

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *o cxdish

