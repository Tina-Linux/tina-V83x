TARGET = duilite-app

SRCS = $(wildcard *.c) 			\
       ./thirdparty/cjson/cJSON.c 	\
       ./app/main_app.c			\
       ./app/cjson_data_parse.c

OBJS = $(patsubst %.c, %.c.o, $(SRCS))

INCLUDES +=-I./include -I./include/cjson/ \
	   -I./include/app/

LIBS += -lauth_r328 -lduilite-r328-0.11.0-39-20190506153403-release
LIBS += -lpthread -ldl -lrt #-WL,-rpath=lib
all:$(TARGET)

$(OBJS):%.c.o:%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@

$(TARGET): $(OBJS)
	$(CC) $^ $(LIBS) $(LDFLAGS) $(LOCAL_LDFLAGS) -o $@

clean:
	rm -rf $(TARGET) $(OBJS)

