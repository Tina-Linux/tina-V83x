PLUGINS:= $(wildcard mcollectd_*.c)
PLUGINS:= $(PLUGINS:%.c=%.o)

.PHONY: all

all: $(PLUGINS)

%.so: %.c
	@echo -e "\033[32mBuilding Plugin: $@\033[0m"
	@$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $(PREFIX)/lib/tt-module/$@
