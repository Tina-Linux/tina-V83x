
SRCDIRS     =./ \

INCLUDES := $(foreach dir,$(SRCDIRS),-I$(dir))

SRCCS    = $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))

LOCAL_FLAGS := -Wall -O0 -g -D__OS_LINUX

INCLUDES += -I../../include/base/ -I../../ -I../../include/demux/

LIBS += -lpthread -lMemAdapter -ltsdemux -ltsc -L../../lib/

TARGET = recordTest

all: $(TARGET)

$(TARGET): $(SRCCS)
	$(CC) $(CFLAGS) $(LOCAL_FLAGS) $(INCLUDES) $(LIBS) -o $@ $^

	cp  recordTest  ../../lib/

clean:
	-rm -f $(TARGET)
	-rm -f ../../lib/recordTest
