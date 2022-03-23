SOURCE := $(wildcard *.cpp)
PROGRAM:= $(patsubst %.cpp,%,$(SOURCE))

all: $(PROGRAM)

%:%.cpp
	$(CROSS_COMPILE)g++ $(CFLAGS) $(LDFLAGS) -o $@ $< -ldmalloc

clean:
	rm -f $(PROGRAM)
