/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "icc_task.h"
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
#include "api_riop.h"
#include "api_riop_common.h"
#include "api_icc.h"



/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define RPMSG_LITE_LINK_ID            (RL_PLATFORM_IMXRT1180_M33_M7_LINK_ID)
#define APP_RPMSG_READY_EVENT_DATA (1U)
#define LOCAL_EPT_ADDR (30U)
#define RPMSG_LITE_NS_ANNOUNCE_STRING "rpmsg-openamp-demo-channel"

#define RPMSG_DATA_HANDLE_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE + 1024)
#define RPMSG_DATA_HANDLE_TASK_PRIORITY   13

/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/
icc_handle_t g_icc_handle;

extern QueueHandle_t afe_command_queue;
extern QueueHandle_t gpio_command_queue;
extern QueueHandle_t siggen_command_queue;


/*******************************************************************************
 * Code
 ******************************************************************************/
static void app_nameservice_isr_cb(uint32_t new_ept, const char *new_ept_name, uint32_t flags, void *user_data)
{
}


void ICC_DataHandleTask(void *data)
{
	uint32_t len = 0;

	riop_status_t status = kStatus_RIOP_Err;

	riop_command_t riop_board_command;
	riop_board_status_t riop_board_status;

	while(1)
	{
		status = ICC_ReadData(&g_icc_handle, (char *)&riop_board_command, sizeof(riop_board_command), &len, portMAX_DELAY);
		if(status == kStatus_RIOP_Ok)
		{
			if(riop_board_command.siggen_control_command.is_dirty)
			{
				xQueueSend(siggen_command_queue, &riop_board_command.siggen_control_command, 0);
			}

			if(riop_board_command.mcu_digital_output_command.is_dirty)
			{
				xQueueSend(gpio_command_queue, &riop_board_command.mcu_digital_output_command.mcu_digital_output, 0);
			}

			if(		(riop_board_command.afe_command.afe_hvsig_command.is_dirty)
				|| 	(riop_board_command.afe_command.afe_lvsig_command.is_dirty)
				|| 	(riop_board_command.afe_command.afe_gpio_command.is_dirty)
				||  (riop_board_command.afe_command.afe_current_command.is_dirty)
				||  (riop_board_command.afe_command.afe_temperature_command.is_dirty)
				||  (riop_board_command.afe_command.afe_calibration_command.is_dirty))
			{
				xQueueSend(afe_command_queue, &riop_board_command.afe_command , 0);
			}
		}

		/*Read riop status from gRiop_status_shm*/
		RIOP_GetBoardStatusForICC(&riop_board_status);

		/*Send riop status to cm33 core*/
		ICC_WriteData(&g_icc_handle, (char *)&riop_board_status, sizeof(riop_board_status), 0);

	}
}


void ICC_InitTask(void *data)
{
	DbgConsole_Printf("rpmsg_task_init CM7 entered\n");

	uint32_t *startupData = data;

	g_icc_handle.rpmsg_instance = rpmsg_lite_remote_init((void *)(char *)startupData, RPMSG_LITE_LINK_ID, RL_NO_FLAGS);

	/* Signal the other core we are ready by triggering the event and passing the APP_RPMSG_READY_EVENT_DATA */
	(void)MCMGR_TriggerEvent(kMCMGR_Core0, kMCMGR_RemoteApplicationEvent, APP_RPMSG_READY_EVENT_DATA);

	rpmsg_lite_wait_for_link_up(g_icc_handle.rpmsg_instance, RL_BLOCK);
	(void)PRINTF("Link is up!\r\n");

	g_icc_handle.rpmsg_queue  = rpmsg_queue_create(g_icc_handle.rpmsg_instance);
	g_icc_handle.ept_instance    = rpmsg_lite_create_ept(g_icc_handle.rpmsg_instance, LOCAL_EPT_ADDR, rpmsg_queue_rx_cb, g_icc_handle.rpmsg_queue);
	g_icc_handle.ns_handle = rpmsg_ns_bind(g_icc_handle.rpmsg_instance, app_nameservice_isr_cb, ((void *)0));

	/* Introduce some delay to avoid NS announce message not being captured by the master side.
   	   This could happen when the remote side execution is too fast and the NS announce message is triggered
   	   before the nameservice_isr_cb is registered on the master side. */
	SDK_DelayAtLeastUs(2000000U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

	(void)rpmsg_ns_announce(g_icc_handle.rpmsg_instance, g_icc_handle.ept_instance, RPMSG_LITE_NS_ANNOUNCE_STRING, (uint32_t)RL_NS_CREATE);

	(void)PRINTF("Nameservice announce sent.\r\n");

	if (xTaskCreate(ICC_DataHandleTask, "rpmsg task", RPMSG_DATA_HANDLE_TASK_STACK_SIZE, NULL, RPMSG_DATA_HANDLE_TASK_PRIORITY, NULL) != pdPASS)
	{
		DbgConsole_Printf("rpmsg_task() creation failed\n");
	}

	vTaskDelete(NULL);
}




