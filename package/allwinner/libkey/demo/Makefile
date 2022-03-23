INCLUDES += -I. -I./include -I./../src/include

LIBS := \
		-L../src/ \
		-lsec_key

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) $(LIBS) -c -o $@ $<

###############test  bin###############
TARGET_TEST=sec_key_test

TEST_SRCS := test.c \
			main.c

TEST_OBJS = $(patsubst %.c, %.o, $(TEST_SRCS))
$(TARGET_TEST):$(TEST_OBJS) $(TARGET)
	$(CC) -o $@ $(TEST_OBJS) $(CFLAGS) $(LDFLAGS) $(INCLUDES) $(LIBS)

all: $(TARGET_TEST)

clean:
	rm -rf $(TARGET_TEST)

