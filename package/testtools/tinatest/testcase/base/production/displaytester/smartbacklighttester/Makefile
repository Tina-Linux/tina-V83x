TARGET := smartbacklighttester

MAIN_SRC := ./smartbacklighttester.c
SRC := ../common/displayInterface.c

local_CFLAGS := -D_GNU_SOURCE -Werror

SRC		:= $(strip $(SRC))
MAIN_SRC	:= $(strip $(MAIN_SRC))
OBJS		:= $(addsuffix .o, $(basename $(SRC)))
MAIN_OBJS	:= $(addsuffix .o, $(basename $(MAIN_SRC)))

$(TARGET): %: %.o $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) $^ -o $@

all: $(TARGET) install

clean:
	-rm $(MAIN_OBJS) $(OBJS) $(TARGET)
install:
	cp $(TARGET) $(PREFIX)
