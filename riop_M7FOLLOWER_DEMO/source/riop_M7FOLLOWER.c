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
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "mcmgr.h"
#include "fsl_debug_console.h"

#include "fsl_rgpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "api_afe.h"
#include "gpio_task.h"
#include "api_riop.h"
#include "riop_feature_config.h"
#include "api_riop_common.h"
#include "afe_task.h"
#include "api_siggen.h"
#include "SIGGEN_task.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define RPMSG_INIT_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE + 256)
#define RPMSG_INIT_TASK_PRIORITY   15

#define GPIO_STATUS_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE + 256)
#define GPIO_STATUS_PRIORITY   5

#define AFE_TASK_PRIORITY		7
#define AFE_TASK_STACK_SIZE		(configMINIMAL_STACK_SIZE + 2048)

#define SIGGEN_TASK_PRIORITY		6
#define SIGGEN_TASK_STACK_SIZE		(configMINIMAL_STACK_SIZE + 256)


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

SemaphoreHandle_t riop_status_mutex = NULL;
QueueHandle_t afe_command_queue = NULL;
QueueHandle_t gpio_command_queue = NULL;
QueueHandle_t siggen_command_queue = NULL;
QueueHandle_t afe_gpio_command_queue = NULL;

QueueHandle_t afe_hvsig_sample_queue = NULL;
QueueHandle_t afe_lvsig_sample_queue = NULL;
QueueHandle_t afe_current_sample_queue = NULL;
QueueHandle_t afe_temp_sample_queue = NULL;
QueueHandle_t afe_gpio_input_status_queue = NULL;




extern NAFE_hdl_t gNafe_Hdl;
extern AFE_ctx_t gAfe_ctx;
extern SIGGEN_config_t gSIGGEN_config;
extern SIGGEN_ctx_t SIGGEN_ctx;

TaskHandle_t GPIO_TaskHandle = NULL;
TaskHandle_t AFE_TaskHandle = NULL;


/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Application-specific implementation of the SystemInitHook() weak function.
 */
void SystemInitHook(void)
{
    /* Initialize MCMGR - low level multicore management library. Call this
       function as close to the reset entry as possible to allow CoreUp event
       triggering. The SystemInitHook() weak function overloading is used in this
       application. */
    (void)MCMGR_EarlyInit();
}


/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t startupData;
    mcmgr_status_t status;

    /* Init board hardware.*/
    BOARD_ConfigMPU();
 //   BOARD_InitDebugConsole();

    SystemCoreClock = CLOCK_GetRootClockFreq(kCLOCK_Root_M7);

    /* Initialize MCMGR, install generic event handlers */
    (void)MCMGR_Init();

    /* Get the startup data */
    do
    {
        status = MCMGR_GetStartupData(kMCMGR_Core1, &startupData);
    } while (status != kStatus_MCMGR_Success);


    riop_status_mutex = xSemaphoreCreateMutex();

    afe_command_queue = xQueueCreate(10, sizeof(afe_command_t));
    afe_gpio_command_queue = xQueueCreate(10, sizeof(uint16_t));
    siggen_command_queue = xQueueCreate(10, sizeof(siggen_control_command_t));
    gpio_command_queue = xQueueCreate(10, sizeof(uint8_t));

    afe_hvsig_sample_queue = xQueueCreate(10, AFE_MAX_EXTERNAL_CHANNELS*sizeof(float));
    afe_lvsig_sample_queue = xQueueCreate(10, 2*AFE_MAX_INTERNAL_CHANNELS*sizeof(float));
    afe_gpio_input_status_queue = xQueueCreate(10, sizeof(uint16_t));
    afe_current_sample_queue = xQueueCreate(10, AFE_MAX_CURRENT_CHANNELS*sizeof(float));
    afe_temp_sample_queue = xQueueCreate(10, sizeof(float));



    if (xTaskCreate(ICC_InitTask, "rpmsg init task", RPMSG_INIT_TASK_STACK_SIZE, (void *)startupData, RPMSG_INIT_TASK_PRIORITY, NULL) != pdPASS)
	{
		PRINTF("ICC_InitTask() failed\n");
		while(1);
	}

    if (xTaskCreate(GPIO_ControlTask, "GPIO status task", GPIO_STATUS_TASK_STACK_SIZE, &gAfe_ctx, GPIO_STATUS_PRIORITY, &GPIO_TaskHandle) != pdPASS)
	{
		PRINTF("GPIO_ControlTask() failed\n");
		while(1);
	}

    if (xTaskCreate(AFE_task, "afe task", AFE_TASK_STACK_SIZE, &gAfe_ctx, AFE_TASK_PRIORITY, &AFE_TaskHandle) != pdPASS)
	{
		PRINTF("AFE_task() creation failed\n");
		while(1);
	}

    if (xTaskCreate(SIGGEN_ControlTask, "siggen task", SIGGEN_TASK_STACK_SIZE, &SIGGEN_ctx, SIGGEN_TASK_PRIORITY, &AFE_TaskHandle) != pdPASS)
	{
		PRINTF("SIGGEN_ControlTask() creation failed\n");
		while(1);
	}


    if (AFE_Init(&gNafe_Hdl) != kStatus_RIOP_Ok)
    {
    	PRINTF("AFE_Init() creation failed\n");
    	while(1);
    }

    if(SIGGEN_Init(&gSIGGEN_config) != kStatus_RIOP_Ok)
    {
    	PRINTF("SIGGEN_Init() creation failed\n");
    	while(1);
    }

    RIOP_StatusShmInit();


    vTaskStartScheduler();

    for (;;)
    {

    }
}
