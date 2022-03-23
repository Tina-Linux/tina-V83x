PLUGINS:= $(wildcard outlog_*.c)
PLUGINS:= $(PLUGINS:%.c=%.o)

.PHONY: all

all: $(PLUGINS)

%.so: %.c
	@echo -e "\033[32mBuilding Plugin: $@\033[0m"
	@flags=`head -n 1 $^ | grep "LDFLAGS *+= *.*" | sed 's#.*+= *\(.*\)\*/#\1#'`; \
		$(CC) $(CFLAGS) $(LDFLAGS) $${flags} $^ -o $(PREFIX)/lib/tt-module/$@
