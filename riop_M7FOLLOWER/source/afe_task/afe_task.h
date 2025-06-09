/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef AFE_TASK_AFE_TASK_H_
#define AFE_TASK_AFE_TASK_H_

#include "api_afe.h"

typedef struct _afe_ctx
{
    NAFE_hdl_t *NafeHdl;
    timer_handle_t *timerHandle;
    uint8_t periodical_measurment_ongoing;
    uint32_t schedule_error;
    AFE_shunt_t afe_shunt;
    uint32_t afe_sampling_period_us;
    uint8_t  afe_sampling_period_counter;
}AFE_ctx_t;

void AFE_task(void *data);

#endif /* AFE_TASK_AFE_TASK_H_ */
