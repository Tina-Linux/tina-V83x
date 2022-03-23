Hi Shawn, thank you for sharing us the Alexa AEC testing result.
I tried to figure out why our wakeup rate is worse than Connexant 2-MIC solution.
Firstly, using more MIC does not lead to a better AEC performance.
Using more MIC normally helps improve noise suppression and dereveberation performances.
If I am right, Connexant does AEC on its chip at which AEC reference and MIC signal are cleaner and better synced.
AEC/NS algorithm software running on a ARM host, like our case, receives signal that is sometimes limited by audio driver.
For example, if we retrive data with S16_LE format on R16 EVM, we receive 16-bit PCM but only high 12-bit of it is effective.
Therefore, I generated another test program, phraseSpotR16_32bpcm_quickAEC.bin (4.a), other than phraseSpotR16_20171006.bin.
phraseSpotR16_32bpcm_quickAEC.bin retrieves data with S24_LE format on R16 EVM and takes high 16-bit as the input to our libaray.
(I am implementing another test program with our modified library which can take 32-bit PCM as input)
However, I cannot rule out the possibility that our technology is not good enough.
Our AEC performance may degrade while we optimize for R16 platform.
Therefore, I provides two libaraies, 2.a & 2.b, trading off between AEC performance and computational expense.
phraseSpotR16_32bpcm_slowAEC.bin (4.b) is built using slower AEC algorithm but with better performance.

If possible, could u please try 4.a and 4.b on Alexa AEC test to see any improvement?

===============================================================================
File description
===============================================================================

1. Header files
1.a. tutu_tool.h <-- parameter file parsing tool
1.b. tutu_typedef.h <-- typedef needed for integrating our library
1.c. tutuClear.h <-- our AEC/NS library API

2. Static library
2.a. libtutuClear_152_20171019.a <-- with optimized AEC
2.b. libtutuClear_152_20171020.a <-- with slower AEC with slightly better AEC performance

3. integration sample code
3.a. phraseSpotR16_16bpcm.c <-- it is the application code for building phraseSpotR16_20171006.bin
3.b. phraseSpotR16_32bpcm.c <-- retrive PCM data in S24_LE format
- Please search __TUTUCLEAR_20171019__ for the code regions related to our library.
- I guess it will bring you some troubles that we only accept 10 ms frame size for processing.
- ln 288-323@phraseSpotR16_16bpcm.c show how we convert the format of PCM buffer from audio I/O driver to what we want.

4. sample code binary
4.a. phraseSpotR16_32bpcm_quickAEC.bin <- S24_LE with libtutuClear_152_20171019.a
4.b. phraseSpotR16_32bpcm_slowAEC.bin <- S24_LE with libtutuClear_152_20171020.a

5. parameter file
5.a. tutuClearA1_ns4wakeup.prm for phraseSpotR16_16/32bpcm.c
5.b. It enables 3-MIC AEC/NS at 16000 Hz with frame size of 10 ms
