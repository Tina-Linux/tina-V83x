#ifndef DTV_TSC_CONFIG_H
#define DTV_TSC_CONFIG_H

//* configuration for interface between TS controller and Demodulator.
#define TS_INTERFACE_TYPE                       (0) //* 0: using ts spi;            1: using ts ssi.
#define TS_INTERFACE_PACKET_SYNC_METHOD         (1) //* 0: by PSync signal;         1: by 0x47 sync byte.
#define TS_INTERFACE_SSI_DATA_ORDER             (0) //* 0: MSB first;               1: LSB first.
#define TS_INTERFACE_CLOCK_SIGNAL_POLARITY      (0) //* 0: rising edge capture;     1: faling edge capture.
#define TS_INTERFACE_ERROR_SIGNAL_POLARITY      (0) //* 0: high level active;       1: low level active.
#define TS_INTERFACE_DATA_VALID_SIGNAL_POLARITY (0) //* 0: high level active;       1: low level active.
#define TS_INTERFACE_PSYNC_SIGNAL_POLARITY      (0) //* 0: high level active;       1: low level active.
#define TS_INTERFACE_PACKET_SIZE                (0) //* 0: 188;             1: 192;             2: 204.
#define TS_INTERFACE_TSF0_INPUT                 (1) //* 0: TSG              1: PORT0            2: PORT1
#define TS_INTERFACE_TSF1_INPUT                 (1) //* 0: TSG              1: PORT0            2: PORT1

#endif
