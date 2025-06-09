/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef CYCLIC_TASK_CYCLIC_TASK_H_
#define CYCLIC_TASK_CYCLIC_TASK_H_

#include "api_riop_common.h"

void cyclic_task_init();
void cyclic_task(void *arg);

#endif /* CYCLIC_TASK_CYCLIC_TASK_H_ */
