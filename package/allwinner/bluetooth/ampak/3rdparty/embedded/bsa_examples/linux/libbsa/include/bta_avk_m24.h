/*****************************************************************************
**
**  Name:           bta_avk_m24.h
**
**  Description:    This is the interface to utility functions for dealing
**                  with MPEG-2, 4 AAC  data frames and codec capabilities.
**
**  Copyright (c) 2006, Widcomm Inc., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_AVK_M24_H
#define BTA_AVK_M24_H

/*****************************************************************************
**  constants
*****************************************************************************/

/*******************************************************************************
**
** Function         bta_avk_m24_cfg_for_cap
**
** Description      Determine the preferred M24 codec configuration for the
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
extern UINT8 bta_avk_m24_cfg_for_cap(UINT8 *p_peer, tA2D_M24_CIE *p_cap, tA2D_M24_CIE *p_pref);

/*******************************************************************************
**
** Function         bta_avk_m24_cfg_in_cap
**
** Description      This function checks whether an M24 codec configuration
**                  is allowable for the given codec capabilities.
**
** Returns          0 if ok, nonzero if error.
**
*******************************************************************************/
extern UINT8 bta_avk_m24_cfg_in_cap(UINT8 *p_cfg, tA2D_M24_CIE *p_cap);


#endif /* BTA_AVK_M24_H */
