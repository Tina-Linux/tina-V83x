TARGET := libiniparser.so libiniparser.a
SUBLIB_DIR = ./..

SHARED_TARGET := libiniparser.so
STATIC_TARGET := libiniparser.a

commonSources:= \
	src/dictionary.c \
    src/iniparser.c

local_CFLAGS := -I./src

OBJS = $(patsubst %.c, %.c.o, $(commonSources))

$(SHARED_TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $(local_CFLAGS) -o $@ $^ -fPIC -shared

$(STATIC_TARGET):$(OBJS)
	$(AR) rc $@ $^

$(OBJS):%.c.o:%.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) -c $< -o $@ -fPIC

all: $(TARGET) install

clean:
	-rm $(OBJS) $(TARGET)
install:
	mkdir -p $(INSTALL_PREFIX)/usr/lib
	mkdir -p $(SUBLIB_DIR)
	mkdir -p $(STAGING_DIR)/usr/lib
	install -m0755 $(TARGET) $(INSTALL_PREFIX)/usr/lib
	install -m0755 $(TARGET) $(SUBLIB_DIR)
	install -m0755 $(TARGET) $(STAGING_DIR)/usr/lib

