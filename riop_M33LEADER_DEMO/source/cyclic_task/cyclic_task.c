/*
* Copyright 2025 NXP
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
#include "FreeRTOS.h"
#include "task.h"
#include "fsl_common.h"
#include "cyclic_task.h"
#include "freemaster_task.h"
#include "freemaster.h"
#include "fsl_gpt.h"

#include "api_riop.h"
#include "api_icc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Stack size of the temporary lwIP initialization thread. */
#define FMSTR_THREAD_STACKSIZE 1024

/* Priority of the temporary lwIP initialization thread. */
#define FMSTR_THREAD_PRIO 3

#define CYCLIC_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE + 2048)
#define CYCLIC_TASK_PRIORITY   12

#define GPT_IRQ_ID             GPT1_IRQn
#define TIMER_ISR              GPT1_IRQHandler


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static riop_status_t TIMER_Init(void);
static riop_status_t TIMER_Start(uint32_t interval_us);
static void TIMER_Stop(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
TaskHandle_t cyclic_TaskHandle = NULL;
uint8_t FRM_dataReady = 0;
riop_command_t riop_command_buffer;
riop_board_status_t riop_status_buffer;

float hvsig_ch0;
float hvsig_ch1;
float hvsig_ch2;
float current_ch0;
extern icc_handle_t g_icc_handle;



/*******************************************************************************
 * Code
 ******************************************************************************/
void cyclic_task_init()
{
	if(TIMER_Init() != kStatus_RIOP_Ok)
	{
		PRINTF("Cyclic task timer init failed\n");
		while(1);
	}

	/* FreeMaster task */
	if(xTaskCreate(fmstr_task, "fmstr_task", FMSTR_THREAD_STACKSIZE, NULL, FMSTR_THREAD_PRIO, NULL) != pdPASS)
	{
		PRINTF("Freemaster task creation failed\n");
		while(1);
	}

	if(xTaskCreate(cyclic_task, "cyclic_task", CYCLIC_TASK_STACK_SIZE, NULL, CYCLIC_TASK_PRIORITY, &cyclic_TaskHandle) != pdPASS)
	{
		PRINTF("Cyclic task creation failed\n");
		while(1);
	}
}


void cyclic_task(void *arg)
{
	uint32_t notify;
	TickType_t timeout = pdMS_TO_TICKS(2000);

	if(TIMER_Start(1000) != kStatus_RIOP_Ok)
	{
		PRINTF("Cyclic task timer start failed\n");
		TIMER_Stop();
		while(1);
	}

	riop_command_t riop_command;
	static riop_board_status_t riop_board_status;
	uint32_t len = 0;

	while(1)
	{
		notify = ulTaskNotifyTake(pdTRUE, timeout);

		if (!notify)
		{
		    continue;
		}

		memset(&riop_command, 0, sizeof(riop_command_t));
		len = 0;

		/*Send data from Freemaster GUI to cm7*/
		if(FRM_dataReady == 1)
		{
			memcpy(&riop_command, &riop_command_buffer, sizeof(riop_command_t));
		}

		ICC_WriteData(&g_icc_handle, (char *)&riop_command, sizeof(riop_command_t), 0);

		FRM_dataReady = 0;

		/*Receive data from CM7 and run Freemaster recorder*/
		ICC_ReadData(&g_icc_handle, (char *)&riop_board_status, sizeof(riop_board_status), &len, 0);

		memcpy(&riop_status_buffer, &riop_board_status, sizeof(riop_board_status_t));

		hvsig_ch0 = riop_board_status.afe_status.hvsig_status.channel_value[0];
		hvsig_ch1 = riop_board_status.afe_status.hvsig_status.channel_value[1];
		hvsig_ch2 = riop_board_status.afe_status.hvsig_status.channel_value[2];
		current_ch0 = riop_board_status.afe_status.current_status.channel_value;

		FMSTR_Recorder(0);
		FMSTR_Recorder(1);
	};
}

static riop_status_t TIMER_Init(void)
{
	gpt_config_t gptConfig;
	GPT_GetDefaultConfig(&gptConfig);
	GPT_Init(GPT1, &gptConfig);
	GPT_SetClockDivider(GPT1, 3);

	return kStatus_RIOP_Ok;
}

static riop_status_t TIMER_Start(uint32_t interval_us)
{
	uint32_t gptFreq = 0;

	if(interval_us < 1000)
	{
		interval_us = 1000;
	}

	gptFreq = CLOCK_GetRootClockFreq(kCLOCK_Root_Gpt1)/3000000;
	gptFreq *= interval_us;

	/* Set both GPT modules to 1 second duration */
	GPT_SetOutputCompareValue(GPT1, kGPT_OutputCompare_Channel1, gptFreq);

	/* Enable GPT Output Compare1 interrupt */
	GPT_EnableInterrupts(GPT1, kGPT_OutputCompare1InterruptEnable);

	/* Enable at the Interrupt */
	EnableIRQ(GPT_IRQ_ID);
	IRQ_SetPriority(GPT_IRQ_ID, 5);

	GPT_StartTimer(GPT1);

	return kStatus_RIOP_Ok;
}

static void TIMER_Stop(void)
{
	GPT_StopTimer(GPT1);
}

void TIMER_ISR(void)
{
	GPT_ClearStatusFlags(GPT1, kGPT_OutputCompare1Flag);

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	vTaskNotifyGiveFromISR(cyclic_TaskHandle, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	SDK_ISR_EXIT_BARRIER;
}
