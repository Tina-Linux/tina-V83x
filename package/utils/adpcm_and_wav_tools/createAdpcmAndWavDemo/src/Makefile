Target = createAdpcmAndWavDemo

CompileFlags = $(CFLAGS)

$(Target): main.c adpcm.c wav.c
	$(CC) -o $@ $^ $(CompileFlags) $(LDFLAGS)
