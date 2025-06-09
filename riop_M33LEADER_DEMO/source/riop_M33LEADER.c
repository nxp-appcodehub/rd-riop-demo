/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "mcmgr.h"
#include "rpmsg_task.h"

#include "FreeRTOS.h"
#include "task.h"

#include "cyclic_task.h"
#include "freemaster.h"
#include "network.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Address of memory, from which the secondary core will boot */
#define CORE1_BOOT_ADDRESS    (void *)0x303C0000
#define CORE1_KICKOFF_ADDRESS 0x0
#define APP_RPMSG_READY_EVENT_DATA (1U)

#define RPMSG_INIT_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE + 256)
#define RPMSG_TASK_PRIORITY   3

#define MAIN_INIT_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE + 256)
#define MAIN_TASK_PRIORITY   3


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

extern char rpmsg_lite_base[SH_MEM_TOTAL_SIZE];

void main_task_init(void *data);


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
    Prepare_CM7(CORE1_KICKOFF_ADDRESS);
}

static volatile uint16_t RPMsgRemoteReadyEventData = 0U;

static void RPMsgRemoteReadyEventHandler(uint16_t eventData, void *context)
{
    uint16_t *data = (uint16_t *)context;

    *data = eventData;
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Initialize MCMGR, install generic event handlers */
    (void)MCMGR_Init();

    /* Init board hardware.*/
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Print the initial banner from Primary core */
    (void)PRINTF("\r\nHello World from the Primary Core!\r\n\n");


    /* Boot Secondary core application */
    (void)PRINTF("Starting Secondary core.\r\n");
    (void)MCMGR_RegisterEvent(kMCMGR_RemoteApplicationEvent, RPMsgRemoteReadyEventHandler,
                                      (void *)&RPMsgRemoteReadyEventData);


    (void)MCMGR_StartCore(kMCMGR_Core1, (void *)(char *)CORE1_BOOT_ADDRESS, (uint32_t)rpmsg_lite_base, kMCMGR_Start_Synchronous);

    while (APP_RPMSG_READY_EVENT_DATA != RPMsgRemoteReadyEventData)
    {
	};

    (void)PRINTF("The secondary core application has been started.\r\n");


    if (xTaskCreate(rpmsg_task_init, "rpmsg init task", RPMSG_INIT_TASK_STACK_SIZE, NULL, RPMSG_TASK_PRIORITY+3, NULL) != pdPASS)
    {
    	PRINTF("rpmsg_init() failed\n");
    }

    if (xTaskCreate(main_task_init, "main init task", MAIN_INIT_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, NULL) != pdPASS)
    {
       	PRINTF("rpmsg_init() failed\n");
    }

    vTaskStartScheduler();


    for (;;)
    {
    }
}

void main_task_init(void *data)
{
	Network_Init(CLOCK_GetRootClockFreq(kCLOCK_Root_Netc));

	/* FreeMASTER driver initialization */
	FMSTR_Init();

	cyclic_task_init();

	vTaskDelete(NULL);
}
