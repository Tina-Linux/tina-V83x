/*****************************************************************************
**
**  Name:           ptim.h
**
**  Description:    Protocol timer services.
**
**  Copyright (c) 2003-2006, Broadcom Corp., All Rights Reserved.
**  Widcomm Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef PTIM_H
#define PTIM_H

#include "gki.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/

typedef struct
{
    TIMER_LIST_Q        timer_queue;        /* GKI timer queue */
    INT32               period;             /* Timer period in milliseconds */
    UINT32              last_gki_ticks;     /* GKI ticks since last time update called */
    UINT8               timer_id;           /* GKI timer id */
} tPTIM_CB;

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************
**  Function Declarations
*****************************************************************************/

/*******************************************************************************
**
** Function         ptim_init
**
** Description      Initialize a protocol timer service control block.
**
** Returns          void
**
*******************************************************************************/
extern void ptim_init(tPTIM_CB *p_cb, UINT16 period, UINT8 timer_id);

/*******************************************************************************
**
** Function         ptim_timer_update
**
** Description      Update the protocol timer list and handle expired timers.
**
** Returns          void
**
*******************************************************************************/
extern void ptim_timer_update(tPTIM_CB *p_cb);

/*******************************************************************************
**
** Function         ptim_start_timer
**
** Description      Start a protocol timer for the specified amount
**                  of time in milliseconds.
**
** Returns          void
**
*******************************************************************************/
extern void ptim_start_timer(tPTIM_CB *p_cb, TIMER_LIST_ENT *p_tle, UINT16 type, INT32 timeout);

/*******************************************************************************
**
** Function         ptim_stop_timer
**
** Description      Stop a protocol timer.
**
** Returns          void
**
*******************************************************************************/
extern void ptim_stop_timer(tPTIM_CB *p_cb, TIMER_LIST_ENT *p_tle);

/* BSA_SPECIFIC */
/*******************************************************************************
**
** Function         ptim_get_remaining_ms
**
** Description      Get remaining ticks to expires
**
** Returns          UINT32: ms to expire
**
*******************************************************************************/
extern  UINT32 ptim_get_remaining_ms(tPTIM_CB *p_cb, TIMER_LIST_ENT *p_tle);
#ifdef __cplusplus
}
#endif

#endif /* PTIM_H */
