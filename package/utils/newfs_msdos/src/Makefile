TARGET := newfs_msdos

SRCS := newfs_msdos.c
OBJS := $(SRCS:.c=.o)

LOCAL_CFLAGS := -DANDROID $(CFLAGS)

%.o: %.c
	$(CC) $(LOCAL_CFLAGS) $(INCLUDES) -c -o $@ $^

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

all: $(TARGET)
