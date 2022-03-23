/*****************************************************************************
**
**  Name:           bta_avk_sbc.h
**
**  Description:    This is the interface to utility functions for dealing
**                  with SBC data frames and codec capabilities.
**
**  Copyright (c) 2004, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_AVK_SBC_H
#define BTA_AVK_SBC_H

/*****************************************************************************
**  constants
*****************************************************************************/

/* SBC packet header size */
#define BTA_AVK_SBC_HDR_SIZE         A2D_SBC_MPL_HDR_LEN

/*******************************************************************************
**
** Function         bta_avk_sbc_init_up_sample
**
** Description      initialize the up sample
**
**                  src_sps: samples per second (source audio data)
**                  dst_sps: samples per second (converted audio data)
**                  bits: number of bits per pcm sample
**                  n_channels: number of channels (i.e. mono(1), stereo(2)...)
**
** Returns          none
**
*******************************************************************************/
extern void bta_avk_sbc_init_up_sample (UINT32 src_sps, UINT32 dst_sps,
                                       UINT16 bits, UINT16 n_channels);

/*******************************************************************************
**
** Function         bta_avk_sbc_up_sample
**
** Description      Given the source (p_src) audio data and
**                  source speed (src_sps, samples per second),
**                  This function converts it to audio data in the desired format
**
**                  p_src: the data buffer that holds the source audio data
**                  p_dst: the data buffer to hold the converted audio data
**                  src_samples: The number of source samples (number of bytes)
**                  dst_samples: The size of p_dst (number of bytes)
**
** Returns          The number of bytes used in p_dst
**                  The number of bytes used in p_src (in *p_ret)
**
*******************************************************************************/
extern int bta_avk_sbc_up_sample (void *p_src, void *p_dst,
                                 UINT32 src_samples, UINT32 dst_samples,
                                 UINT32 *p_ret);

/*******************************************************************************
**
** Function         bta_avk_sbc_up_sample_16s (16bits-stereo)
**
** Description      Given the source (p_src) audio data and
**                  source speed (src_sps, samples per second),
**                  This function converts it to audio data in the desired format
**
**                  p_src: the data buffer that holds the source audio data
**                  p_dst: the data buffer to hold the converted audio data
**                  src_samples: The number of source samples (in uint of 4 bytes)
**                  dst_samples: The size of p_dst (in uint of 4 bytes)
**
** Returns          The number of bytes used in p_dst
**                  The number of bytes used in p_src (in *p_ret)
**
*******************************************************************************/
extern int bta_avk_sbc_up_sample_16s (void *p_src, void *p_dst,
                                 UINT32 src_samples, UINT32 dst_samples,
                                 UINT32 *p_ret);

/*******************************************************************************
**
** Function         bta_avk_sbc_up_sample_16m (16bits-mono)
**
** Description      Given the source (p_src) audio data and
**                  source speed (src_sps, samples per second),
**                  This function converts it to audio data in the desired format
**
**                  p_src: the data buffer that holds the source audio data
**                  p_dst: the data buffer to hold the converted audio data
**                  src_samples: The number of source samples (in uint of 2 bytes)
**                  dst_samples: The size of p_dst (in uint of 2 bytes)
**
** Returns          The number of bytes used in p_dst
**                  The number of bytes used in p_src (in *p_ret)
**
*******************************************************************************/
extern int bta_avk_sbc_up_sample_16m (void *p_src, void *p_dst,
                                     UINT32 src_samples, UINT32 dst_samples,
                                     UINT32 *p_ret);

/*******************************************************************************
**
** Function         bta_avk_sbc_up_sample_8s (8bits-stereo)
**
** Description      Given the source (p_src) audio data and
**                  source speed (src_sps, samples per second),
**                  This function converts it to audio data in the desired format
**
**                  p_src: the data buffer that holds the source audio data
**                  p_dst: the data buffer to hold the converted audio data
**                  src_samples: The number of source samples (in uint of 2 bytes)
**                  dst_samples: The size of p_dst (in uint of 2 bytes)
**
** Returns          The number of bytes used in p_dst
**                  The number of bytes used in p_src (in *p_ret)
**
*******************************************************************************/
extern int bta_avk_sbc_up_sample_8s (void *p_src, void *p_dst,
                                 UINT32 src_samples, UINT32 dst_samples,
                                 UINT32 *p_ret);

/*******************************************************************************
**
** Function         bta_avk_sbc_up_sample_8m (8bits-mono)
**
** Description      Given the source (p_src) audio data and
**                  source speed (src_sps, samples per second),
**                  This function converts it to audio data in the desired format
**
**                  p_src: the data buffer that holds the source audio data
**                  p_dst: the data buffer to hold the converted audio data
**                  src_samples: The number of source samples (number of bytes)
**                  dst_samples: The size of p_dst (number of bytes)
**
** Returns          The number of bytes used in p_dst
**                  The number of bytes used in p_src (in *p_ret)
**
*******************************************************************************/
extern int bta_avk_sbc_up_sample_8m (void *p_src, void *p_dst,
                                     UINT32 src_samples, UINT32 dst_samples,
                                     UINT32 *p_ret);

/*******************************************************************************
**
** Function         bta_avk_sbc_cfg_for_cap
**
** Description      Determine the preferred SBC codec configuration for the
**                  given codec capabilities.  The function is passed the
**                  preferred codec configuration and the peer codec
**                  capabilities for the stream.  The function attempts to
**                  match the preferred capabilities with the configuration
**                  as best it can.  The resulting codec configuration is
**                  returned in the same memory used for the capabilities.
**
** Returns          0 if ok, nonzero if error.
**                  Codec configuration in p_cap.
**
*******************************************************************************/
extern UINT8 bta_avk_sbc_cfg_for_cap(UINT8 *p_peer, tA2D_SBC_CIE *p_cap, tA2D_SBC_CIE *p_pref);

/*******************************************************************************
**
** Function         bta_avk_sbc_cfg_in_cap
**
** Description      This function checks whether an SBC codec configuration
**                  is allowable for the given codec capabilities.
**
** Returns          0 if ok, nonzero if error.
**
*******************************************************************************/
extern UINT8 bta_avk_sbc_cfg_in_cap(UINT8 *p_cfg, tA2D_SBC_CIE *p_cap);

/*******************************************************************************
**
** Function         bta_avk_sbc_bld_hdr
**
** Description      This function builds the packet header for MPF1.
**
** Returns          void
**
*******************************************************************************/
extern void bta_avk_sbc_bld_hdr(BT_HDR *p_buf, UINT16 fr_per_pkt);

#endif /* BTA_AVK_SBC_H */
