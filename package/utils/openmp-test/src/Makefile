target			= openmp-test

INCLUDES        += -I.
LIBS            += -lgomp
LDFLAGS		+= -fopenmp

install:$(target)
	@mkdir -p $(CONFIG_PREFIX)/bin
	@cp openmp1 $(CONFIG_PREFIX)/bin
	@cp openmp2 $(CONFIG_PREFIX)/bin
	@cp fib $(CONFIG_PREFIX)/bin

$(target):openmp.c openmp2.c fib.c
	$(CC) $(INCLUDES) $(LDFLAGS) $(LIBS) openmp.c -o openmp1
	$(CC) $(INCLUDES) $(LDFLAGS) $(LIBS) openmp2.c -o openmp2
	$(CC) $(INCLUDES) $(LDFLAGS) $(LIBS) fib.c -o fib

####################################################################
all: install
clean:
	rm -rf *.c openmp1 openmp2 fib
