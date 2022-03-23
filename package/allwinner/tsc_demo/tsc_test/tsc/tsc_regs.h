#ifndef DTV_TSC_REGS_H
#define DTV_TSC_REGS_H
#include "tsc_type.h"

// 1689
#define TSC_BASE    0x01c04000
#define TSC_OFFSET  0x0
#define TSG_OFFSET  0x40
#define TSF0_OFFSET 0x80
#define TSD_OFFSET  0x180

//#define TSF1_OFFSET 0x100

//read and set register
#define SetValue(X, value)      do { \
    volatile void *p = (void *)(&(X)); \
    (*(uint32_t *)p) = (value); \
    } while(0)

#define GetValue(X, value)         do { \
	volatile void *p = (void *)(&(X)); \
	(value) = (*(uint32_t *)p); \
	} while(0)

//**************************** tsc register define ****************************/
//* tsc contrl register.
typedef struct TSC00_CTRL_REG {
	volatile  unsigned enable :1;
	volatile  unsigned reset :1;
	volatile  unsigned reserved :30;
} tsc00_ctrl_reg_t;

//* tsc status register.
typedef struct TSC04_STATUS_REG {
	volatile  unsigned reserved :32;
} tsc04_status_reg_t;

//* tsc port control register.
typedef struct TSC10_PORT_CTRL_REG {
	volatile  unsigned port0_input_ctrl :1; //* 0: spi, 1: ssi.
	//uint32_t port1_input_ctrl :1; //* 0: spi, 1: ssi.
	volatile  unsigned reserved :31; //* 0: spi, 1: ssi.
} tsc10_port_ctrl_reg_t;

//* tsc port parameter register.
typedef struct TSC14_PORT_PARAM_REG {
	volatile  unsigned port0_psync_polarity :1; //* 0: high level active, 1: low level active;
	volatile  unsigned port0_dvalid_polarity :1; //* 0: high level active, 1: low level active;
	volatile  unsigned port0_error_polarity :1; //* 0: high level active, 1: low level active;
	volatile  unsigned port0_clock_polarity :1; //* 0: rise edge capture, 1: fall edge capture;
	volatile  unsigned port0_ssi_data_order :1; //* 0: MSB first for one byte data, 1: LSB first for one byte data;
	volatile  unsigned reserved :3;
	volatile  unsigned reserved1 :8;
	volatile  unsigned reserved2 :16;
} tsc14_port_param_reg_t;

//* tsc tsf input multeplex control register.
typedef struct TSC20_IN_MUX_CTRL_REG {
	volatile  unsigned tsf0_in_mux_ctrl :4; //* 0: from tsg, 1: from port 0, others: reserved;
	volatile  unsigned reserved :28;
} tsc20_in_mux_ctrl_reg_t;

//* tsc tsf output multiplex control register.
typedef struct TSC28_OUT_MUX_CTRL_REG {
	volatile  unsigned reserved :32;
} tsc28_out_mux_ctrl_reg_t;

//**************************** tsg register define ****************************/
//* tsg control register
typedef struct TSG00_CTRL_REG {
	volatile  unsigned start :1; //* write '1' to start tsg;
	volatile  unsigned stop :1; //* write '1' to stop tsg;
	volatile  unsigned pause :1; //* write '1' to pause or resume tsg;
	volatile  unsigned reserved0 :5; //*
	volatile  unsigned check_sync_byte :1; //* 0: disable, 1: enable, it will give an interrupt and go into pause state if unsync.
	volatile  unsigned loop_mode_enable :1; //* 0: disable, 1: tsg buffer in loop back mode.
	volatile  unsigned reserved1 :14; //*
	volatile  unsigned tsg_status :2; //* 0: idle, 1: running, 2: pause;
	volatile  unsigned reserved2 :6; //*
} tsg00_ctrl_reg_t;

//* tsg packet parameter register
typedef struct TSG04_PKT_PARAM_REG {
	volatile  unsigned pkt_size :2; //* packet size for tsg.0:188 bytes  others:reserved
	volatile  unsigned reserved0 :5; //*
	volatile  unsigned sync_byte_pos :1; //* 0: the first byte is 0x47, 1: the fifth byte is 0x47.
	volatile  unsigned reserved1 :8; //*
	volatile  unsigned sync_byte_value :8; //* 0x47.
	volatile  unsigned reserved2 :8; //*
} tsg04_pkt_param_reg_t;

//* tsg status register
typedef struct TSG08_STATUS_REG {
	volatile  unsigned err_sync_byte :1; //* find error sync byte case, write '1' to clear.
	volatile  unsigned half_finish :1; //* half finished, write '1' to clear.
	volatile  unsigned full_finish :1; //* full finished, write '1' to clear.
	volatile  unsigned end :1; //* tsg end, write '1' to clear.
	volatile  unsigned reserved0 :12; //*
	volatile  unsigned err_sync_byte_intr_en :1; //* tsg error sync byte interrupt enable bit, 0: disable, 1: enable.
	volatile  unsigned half_finish_intr_en :1; //* tsg half finish interrupt enable bit, 0: disable, 1: enable.
	volatile  unsigned full_finish_intr_en :1; //* tsg full finish interrupt enable bit, 0: disable, 1: enable.
	volatile  unsigned end_intr_en :1; //* tsg end interrupt enable bit, 0: diable, 1: enable.
	volatile  unsigned reserved1 :12; //*
} tsg08_status_reg_t;

//* tsg clock control register
typedef struct TSG0c_CLK_CTRL_REG {
	//* output frequency fo = (fi*(n+1))/(16*(d+1)), fi is the special input clock to tsc.
	volatile  unsigned clock_divid_factor_d :16; //* clock divid factor d.
	volatile  unsigned clock_divid_factor_n :16; //* clock divid factor n.
} tsg0c_clk_ctrl_reg_t;

//* tsg buffer base address register
typedef struct TSG10_BUF_ADDR_REG {
	volatile  unsigned addr :32; //* buffer base address, 16 bytes aligned.
} tsg10_buf_addr_reg_t;

//* tsg buffer size register
typedef struct TSG14_BUF_SIZE_REG {
	volatile  unsigned size :24; //* buffer size, 16 bytes aligned.
	volatile  unsigned reserved :8; //*
} tsg14_buf_size_reg_t;

//* tsg buffer pointer register
typedef struct TSG18_BUF_PTR_REG {
	volatile  unsigned cur_pos :24; //* tsg current sending position.
	volatile  unsigned reserved :8; //*
} tsg18_buf_ptr_reg_t;

//**************************** tsf register define ****************************/
//* tsf control register
typedef struct TSF00_CTRL_REG {
	volatile  unsigned reset :1; //* tsf global soft reset.
	volatile  unsigned mode :1; //* no use currently.
	volatile  unsigned enable :1; //* tsf enable control.
	volatile  unsigned reserve :29;
} tsf00_ctrl_reg_t;

//* tsf packet parameter register
typedef struct TSF04_PKT_PARAM_REG {
	volatile  unsigned pkt_size :2; //* packet size, 0:188, 1:192, 2:204;
	volatile  unsigned reserved0 :5; //*
	volatile  unsigned sync_byte_pos :1; //* 0: the first byte, 1: the fifth byte.
	volatile  unsigned sync_method :2; //* 0: byte psync signal, 1: by sync byte, 2: by both psync and sync byte.
	volatile  unsigned reserved1 :6; //*
	volatile  unsigned sync_byte_value :8; //* 0x47
	volatile  unsigned sync_pkt_threshold :4; //*
	volatile  unsigned lost_sync_pkt_threshold :4; //*
} tsf04_pkt_param_reg_t;

//* tsf status register
typedef struct TSF08_STATUS_REG {
	volatile  unsigned dma_status :1; //* it is a global status of all 32 channels
	volatile  unsigned overlap_status :1; //* it is a global status of all 32 channels
	volatile  unsigned pcr_found :1; //* a PCR packet is found, write '1' to clear
	volatile  unsigned fifo_overrun :1; //* tsf internal fifo overrun
	volatile  unsigned reserved0 :12; //*
	volatile  unsigned dma_intr_en :1; //* 0: disable, 1: enable
	volatile  unsigned overlap_intr_en :1; //* 0: disable, 1: enable
	volatile  unsigned pcr_intr_en :1; //* 0: disable, 1: enable
	volatile  unsigned fifo_overrun_intr_en :1; //* 0: disable, 1: enable
	volatile  unsigned reserved1 :12; //*
} tsf08_status_reg_t;

//* tsf dma interrupt enable register
typedef struct TSF10_DMA_INTR_EN_REG {
	volatile  unsigned en_ctrl_bits; //* dma intrrupt enable control bits, bits 0~31 for channel 0~31.
} tsf10_dma_intr_en_reg_t;

//* tsf overlap interrupt enable register
typedef struct TSF14_OVERLAP_INTR_EN_REG {
	volatile  unsigned en_ctrl_bits; //* overlap interrupt enable control bits, bits 0~31 for channel 0~31.
} tsf14_overlap_intr_en_reg_t;

//* tsf dma interrupt status register
typedef struct TSF18_DMA_STATUS_REG {
	volatile  unsigned dma_status; //* dma interrupt status, bits 0~31 for channel 0~31, write 1 to clear.
} tsf18_dma_status_reg_t;

//* tsf overlap interrupt status register
typedef struct TSF1c_OVERLAP_STATUS_REG {
	volatile  unsigned overlap_status; //* overlap interrupt status, bits 0~31 for channel 0~31, write 1 to clear.
} tsf1c_overlap_stauts_reg_t;

//* tsf pcr control register
typedef struct TSF20_PCR_CTRL_REG {
	volatile  unsigned pcr_lsb_bit :1; //* pcr contest LSB 1 bit.
	volatile  unsigned reserved0 :7; //*
	volatile  unsigned pcr_chan_idx :5; //* channel index for detecting PCR packet.
	volatile  unsigned reserved1 :3; //*
	volatile  unsigned pcr_detect_en :1; //* pcr detecting enable
	volatile  unsigned reserved2 :15; //*
} tsf20_pcr_ctrl_reg_t;

//* tsf pcr data register
typedef struct TSF24_PCR_DATA_REG {
	volatile  unsigned pcr_value; //* pcr bit 32~1
} tsf24_pcr_data_reg_t;

//* tsf channel enable register
typedef struct TSF30_CHAN_EN_REG {
	volatile  unsigned filter_en_ctrl_bits; //* 0: disable, 1: enable, bit 0~31 for channel 0~31.
} tsf30_chan_en_reg_t;

//* tsf channel pes enable register
typedef struct TSF34_CHAN_PES_EN_REG {
	volatile  unsigned pes_en_ctrl_bits; //* 0: disable, 1: enable, bit 0~31 for channel 0~31.
} tsf34_chan_pes_en_reg_t;

//* tsf channel descramble enable register
typedef struct TSF38_CHAN_DESCRAMBLE_EN_REG {
	volatile  unsigned descramble_en_ctrl_bits;
} tsf38_chan_descramble_en_reg_t;

//* tsf channel index register
typedef struct TSF3c_CHAN_IDX_REG {
	volatile  unsigned chan_idx :5; //* when you are writing pid, you have to write this reg to tell which
	volatile  unsigned reserved0 :27; //* channel you are setting.
} tsf3c_chan_idx_reg_t;

//* tsf channel control register
typedef struct TSF40_CHAN_CTRL_REG {
	volatile  unsigned reserved0;
} tsf40_chan_ctrl_reg_t;

//* tsf channel status register
typedef struct TSF44_CHAN_STATUS_REG {
	volatile  unsigned reserved0;
} tsf44_chan_status_reg_t;

//* tsf channel CW index register
typedef struct TSF48_CHAN_CW_IDX_REG {
	volatile  unsigned related_ctrl_word_idx :3; //*
	volatile  unsigned reserved :29; //*
} tsf48_chan_cw_idx_reg_t;

//* tsf channel pid register
typedef struct TSF4c_CHAN_PID_REG {
	volatile  unsigned pid :16; //* channel pid.
	volatile  unsigned mask :16; //* pid mask.
} tsf4c_chan_pid_reg_t;

//* tsf channel buffer base address register
typedef struct TSF50_CHAN_BUF_ADDR_REG {
	volatile  unsigned addr :32; //* data buffer base address for channel
} tsf50_chan_buf_addr_reg_t;

//* tsf channel buffer size register
typedef struct TSF54_CHAN_BUF_SIZE_REG {
	volatile  unsigned size :21; //* the exact buffer size is N+16 bytes.
	volatile  unsigned reserved0 :3; //* the size should be 16 bytes aligned.
	volatile  unsigned dma_intr_threshold :2; //* 0: 1/2, 1: 1/4, 2: 1/8, 3: 1/16.
	volatile  unsigned reserved1 :6; //*
} tsf54_chan_buf_size_reg_t;

//* tsf channel buffer write pointer register
typedef struct TSF58_CHAN_WRITE_POS_REG {
	volatile  unsigned wt_pos :21; //* data buffer write pos.
	volatile  unsigned reserved0 :11; //*
} tsf58_chan_wt_pos_reg_t;

//* tsf channel buffer read pointer register
typedef struct TSF5c_CHAN_READ_POS_REG {
	volatile  unsigned rd_pos :21; //* data buffer read pos.
	volatile  unsigned reserved0 :11; //*
} tsf5c_chan_rd_pos_reg_t;


//**************************** tsd register define ****************************/
//* tsdcontrol register
typedef struct TSD00_CTRL_REG {
	volatile  unsigned descramble_method :2; //* descramble arithmetic 0: DVD CSA V1.1  others: reserved;
	volatile  unsigned reserved :30;
} tsd00_ctrl_reg_t;

//* tsd status register
typedef struct TSD04_STATUS_REG {
	volatile  unsigned reserved :32; //*
} tsd04_status_reg_t;

//* tsd control Word Index register
typedef struct TSD1c_CW_INDEX_REG {
	volatile  unsigned cw_internal_idx :2; //*
	volatile  unsigned reserved0 :2; //*
	volatile  unsigned cw_idx :3; //*
	volatile  unsigned reserved1 :25; //*
} tsd1c_cw_index_reg_t;

//* tsd Control Word register
typedef struct TSD20_CW_REG {
	volatile  unsigned cw_content :32; //*
} tsd20_cw_reg_t;


//************************ ts controller register list ************************/

typedef struct TSC_TSC_REG_LIST {
	volatile tsc00_ctrl_reg_t _00_ctrl;
	volatile tsc04_status_reg_t _04_status;
	volatile uint32_t _reserved0[2];
	volatile tsc10_port_ctrl_reg_t _10_port_ctrl;
	volatile tsc14_port_param_reg_t _14_port_param;
	volatile uint32_t _reserved1[2];
	volatile tsc20_in_mux_ctrl_reg_t _20_in_mux_ctrl;
	volatile uint32_t _reserved2[1];
	volatile tsc28_out_mux_ctrl_reg_t _28_out_mux_ctrl;
	volatile uint32_t _reserved3[5];
} tsc_tsc_reg_list_t;

typedef struct TSC_TSG_REG_LIST {
	volatile tsg00_ctrl_reg_t _00_ctrl;
	volatile tsg04_pkt_param_reg_t _04_pkt_param;
	volatile tsg08_status_reg_t _08_status;
	volatile tsg0c_clk_ctrl_reg_t _0c_clk_ctrl;
	volatile tsg10_buf_addr_reg_t _10_buf_addr;
	volatile tsg14_buf_size_reg_t _14_buf_size;
	volatile tsg18_buf_ptr_reg_t _18_buf_pos;
	volatile uint32_t _reserved0[9];
} tsc_tsg_reg_list_t;

typedef struct TSC_TSF_REG_LIST {
	volatile tsf00_ctrl_reg_t _00_ctrl;
	volatile tsf04_pkt_param_reg_t _04_pkt_param;
	volatile tsf08_status_reg_t _08_status;
	volatile uint32_t _reserved0[1];
	volatile tsf10_dma_intr_en_reg_t _10_dma_intr_en;
	volatile tsf14_overlap_intr_en_reg_t _14_overlap_intr_en;
	volatile tsf18_dma_status_reg_t _18_dma_status;
	volatile tsf1c_overlap_stauts_reg_t _1c_overlap_status;
	volatile tsf20_pcr_ctrl_reg_t _20_pcr_ctrl;
	volatile tsf24_pcr_data_reg_t _24_pcr_data;
	volatile uint32_t _reserved1[2];
	volatile tsf30_chan_en_reg_t _30_chan_en;
	volatile tsf34_chan_pes_en_reg_t _34_chan_pes_en;
	volatile tsf38_chan_descramble_en_reg_t _38_chan_descramble_en;
	volatile tsf3c_chan_idx_reg_t _3c_chan_idx;
	volatile tsf40_chan_ctrl_reg_t _40_chan_ctrl;
	volatile tsf44_chan_status_reg_t _44_chan_status;
	volatile tsf48_chan_cw_idx_reg_t _48_chan_cw_idx;
	volatile tsf4c_chan_pid_reg_t _4c_chan_pid;
	volatile tsf50_chan_buf_addr_reg_t _50_chan_buf_addr;
	volatile tsf54_chan_buf_size_reg_t _54_chan_buf_size;
	volatile tsf58_chan_wt_pos_reg_t _58_chan_wt_pos;
	volatile tsf5c_chan_rd_pos_reg_t _5c_chan_rd_pos;
	volatile uint32_t _reserved2[8];
} tsc_tsf_reg_list_t;

typedef struct TSC_TSD_REG_LIST {
	volatile tsd00_ctrl_reg_t _00_ctrl;
	volatile tsd04_status_reg_t _04_status;
	volatile uint32_t _reserved[5];
	volatile tsd1c_cw_index_reg_t _1c_cw_idx;
	volatile tsd20_cw_reg_t _20_cw;
} tsc_tsd_reg_list_t;


typedef struct TSC_REG_LIST {
	//* tsc registers.
	tsc_tsc_reg_list_t tsc;

	//* tsg registers.
	tsc_tsg_reg_list_t tsg;

	//* tsf0 registers.
	tsc_tsf_reg_list_t tsf;

	tsc_tsf_reg_list_t reserved;//no use, just for align

	//* tsd registers
	tsc_tsd_reg_list_t tsd;
} tsc_reg_list_t;

#endif
