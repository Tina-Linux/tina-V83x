ADB_AUTH_LIB             = libawadbauth.so
ADB_AUTH_SERVICE         = adbd_auth_service
INCLUDES        += -I./include

LIB_SRCS := \
	aw_adb_auth.c

SERVICE_SRCS := \
	aw_adb_auth_service.c

SERVICE_LIB := -L./ -lawadbauth

LIB_OBJS    = $(LIB_SRCS:.c=.o)
SERVICE_OBJS    = $(SERVICE_SRCS:.c=.o)

LOCAL_CFLAGS := -fPIC

%.o: %.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) $(INCLUDES) -c -o $@ $<

$(ADB_AUTH_LIB): $(LIB_OBJS)
	$(CC) -shared $(LOCAL_CFLAGS) $(LDFLAGS) -lev $^ -o $@

$(ADB_AUTH_SERVICE): $(SERVICE_OBJS)
	$(CC) $(LDFLAGS) $(SERVICE_LIB) $^ -o $@

lib:$(ADB_AUTH_LIB)

service:$(ADB_AUTH_SERVICE)

clean:
	rm -rf $(ADB_AUTH_LIB) $(ADB_AUTH_SERVICE) *.o
