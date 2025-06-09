/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef SIGGEN_TASK_SIGGEN_TASK_H_
#define SIGGEN_TASK_SIGGEN_TASK_H_

#include "fsl_common.h"
#include "api_siggen.h"

typedef struct _siggen_ctx
{
	uint8_t generate;
	SIGGEN_config_t *siggenConfig;

}SIGGEN_ctx_t;


void SIGGEN_ControlTask(void *data);

#endif /* SIGGEN_TASK_SIGGEN_TASK_H_ */
