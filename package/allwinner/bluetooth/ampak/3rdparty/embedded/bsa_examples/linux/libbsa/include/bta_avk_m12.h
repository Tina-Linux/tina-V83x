/*****************************************************************************
**
**  Name:           bta_avk_m12.h
**
**  Description:    This is the interface to utility functions for dealing
**                  with MPEG-1, 2 Audio data frames and codec capabilities.
**
**  Copyright (c) 2004, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_AVK_M12_H
#define BTA_AVK_M12_H

/*****************************************************************************
**  constants
*****************************************************************************/

/* MPEG-1, 2 Audio packet header size */
#define BTA_AVK_M12_HDR_SIZE         A2D_M12_MPL_HDR_LEN


/*******************************************************************************
**
** Function         bta_avk_m12_cfg_for_cap
**
** Description      Determine the preferred MPEG-1, 2 Audio codec configuration for the
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
extern UINT8 bta_avk_m12_cfg_for_cap(UINT8 *p_peer, tA2D_M12_CIE *p_cap, tA2D_M12_CIE *p_pref);

/*******************************************************************************
**
** Function         bta_avk_m12_cfg_in_cap
**
** Description      This function checks whether an MPEG-1, 2 Audio codec configuration
**                  is allowable for the given codec capabilities.
**
** Returns          0 if ok, nonzero if error.
**
*******************************************************************************/
extern UINT8 bta_avk_m12_cfg_in_cap(UINT8 *p_cfg, tA2D_M12_CIE *p_cap);

/*******************************************************************************
**
** Function         bta_avk_m12_bld_hdr
**
** Description      This function builds the packet header for MPF1.
**
** Returns          void
**
*******************************************************************************/
extern void bta_avk_m12_bld_hdr(BT_HDR *p_buf, UINT16 frag_offset);

#endif /* BTA_AVK_M12_H */
