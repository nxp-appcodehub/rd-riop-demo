/*
* Copyright 2024-2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
#include "rpmsg_ns.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "FreeRTOS.h"
#include "task.h"

#include "fsl_common.h"
#include "mcmgr.h"
#include "rpmsg_task.h"
#include "api_icc.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define RPMSG_LITE_LINK_ID (RL_PLATFORM_IMXRT1180_M33_M7_LINK_ID)
#define LOCAL_EPT_ADDR    (40U)


/*******************************************************************************
 * Prototypes
 ******************************************************************************/



/*******************************************************************************
 * Variables
 ******************************************************************************/
char rpmsg_lite_base[SH_MEM_TOTAL_SIZE] __attribute__((section(".noinit.$rpmsg_sh_mem")));
icc_handle_t g_icc_handle;



/*******************************************************************************
 * Code
 ******************************************************************************/

static void app_nameservice_isr_cb(uint32_t new_ept, const char *new_ept_name, uint32_t flags, void *user_data)
{
    uint32_t *data = (uint32_t *)user_data;

    *data = new_ept;
}


void rpmsg_task_init(void *data)
{
	g_icc_handle.rpmsg_instance = rpmsg_lite_master_init(rpmsg_lite_base, SH_MEM_TOTAL_SIZE, RPMSG_LITE_LINK_ID, RL_NO_FLAGS);

	g_icc_handle.rpmsg_queue  = rpmsg_queue_create(g_icc_handle.rpmsg_instance);
	g_icc_handle.ept_instance    = rpmsg_lite_create_ept(g_icc_handle.rpmsg_instance, LOCAL_EPT_ADDR, rpmsg_queue_rx_cb, g_icc_handle.rpmsg_queue);
	g_icc_handle.ns_handle = rpmsg_ns_bind(g_icc_handle.rpmsg_instance, app_nameservice_isr_cb, (void *)&g_icc_handle.remote_addr);

	/* Wait until the secondary core application issues the nameservice isr and the remote endpoint address is known. */
	while (0U == g_icc_handle.remote_addr)
	{
	};

	vTaskDelete(NULL);
}
