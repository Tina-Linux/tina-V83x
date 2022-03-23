INCLUDES += -I. -I./include -I./../src/include \
			-I./../src/testcase -I./../src/framebuffer \
			-I./../src/virtual

LIBS := --static -L./../src/testcase -ltestcase \
	-L./../src/framebuffer -lfb \
	-L./../src/virtual -lbmp \
	-lm

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) $(LIBS) -c -o $@ $<

###############test  bin###############
TARGET_TEST=display_test

TEST_SRCS := screen-test.c \
			canvas.c

TEST_OBJS = $(patsubst %.c, %.o, $(TEST_SRCS))
$(TARGET_TEST):$(TEST_OBJS) $(TARGET)
	$(CC) -o $@ $(TEST_OBJS) $(CFLAGS) $(LDFLAGS) $(INCLUDES) $(LIBS)

all: $(TARGET_TEST)

clean:
	rm -rf $(TARGET_TEST)

