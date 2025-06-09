/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef API_ICC_H_
#define API_ICC_H_

#include "fsl_common.h"
#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
#include "rpmsg_ns.h"
#include "api_riop_common.h"

typedef struct _icc_handle
{
	struct rpmsg_lite_instance *volatile rpmsg_instance;
    struct rpmsg_lite_endpoint *volatile ept_instance;
	volatile rpmsg_queue_handle rpmsg_queue;
	volatile uint32_t remote_addr;
	volatile rpmsg_ns_handle ns_handle;
} icc_handle_t;



riop_status_t ICC_ReadData(icc_handle_t * handle, void *data, uint32_t max_length, uint32_t *data_length, uint32_t timeout);
riop_status_t ICC_WriteData(icc_handle_t * handle, void *data, uint32_t max_length, uint32_t timeout);

#endif /* API_ICC_H_ */
