
CFLAGS	:= -O2 -g -fPIC -W -Wall
LDFLAGS	:=

OBJECTS := matrix.o isp_math_util.o

install: libisp_math.a libisp_math.so
	rm -rf  $(OBJECTS)
	$(STP) --strip-debug --strip-unneeded *.a
	$(STP) --strip-debug --strip-unneeded *.so
	mv *.a ../out
	mv *.so ../out

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: libisp_math.a libisp_math.so

libisp_math.a: $(OBJECTS)
	$(AR) r $@ $^

libisp_math.so: $(OBJECTS)
	$(CC) -o $@ -shared $^

clean:
	-$(RM) *.o
	-$(RM) libisp_math.so
	-$(RM) libisp_math.a
