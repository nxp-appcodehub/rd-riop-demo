/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef __FMSTR_NETWORK_H
#define __FMSTR_NETWORK_H

#include "board.h"
#include "network_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Definitions
 ******************************************************************************/
/* PHY */

#include "fsl_phyvsc8541.h"
#ifndef EXAMPLE_PHY_RES
#define EXAMPLE_PHY_RES       phy_vsc8541_resource_t
#endif

#ifndef EXAMPLE_PHY_PREFIX
#define EXAMPLE_PHY_PREFIX(m) PHY_VSC8541_##m
#endif


/******************************************************************************
 * Functions definitions
 ******************************************************************************/

/* Network example application functions */
void Network_Init(uint32_t csrClock_Hz);

/* Network init internal function */
void Network_PhyInit(EXAMPLE_PHY_RES *phy_res, phy_operations_t *phy_ops, uint32_t csrClock_Hz);

#ifdef __cplusplus
}
#endif

#endif /* __FMSTR_NETWORK_H */
