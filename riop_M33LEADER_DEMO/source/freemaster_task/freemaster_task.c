/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "freemaster_task.h"
#include "freemaster.h"


/*******************************************************************************
 * Variables
 ******************************************************************************/
static FMSTR_U8 FreeMASTER_RecBuffer1[4096];


/*******************************************************************************
 * Code
 ******************************************************************************/
void fmstr_task(void *arg)
{
	FMSTR_REC_BUFF FreeMASTER_Recorder_1 =
	{
	  .name = "Description of recorder 1",
	  .addr = (FMSTR_ADDR)FreeMASTER_RecBuffer1,
	  .size = sizeof(FreeMASTER_RecBuffer1),
	  .basePeriod_ns = 100000UL
	};

	FMSTR_RecorderCreate(1, &FreeMASTER_Recorder_1);

    while (1)
    {
        /* The FreeMASTER poll handles the communication interface and protocol
           processing. This call will block the task execution when no communication
           takes place (also see FMSTR_NET_BLOCKING_TIMEOUT option) */
        FMSTR_Poll();
    }
}
