/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/
#include "api_icc.h"
#include "rpmsg_queue.h"
#include "api_riop_common.h"




/*******************************************************************************
 * Code
 ******************************************************************************/
riop_status_t ICC_ReadData(icc_handle_t * handle, void *data, uint32_t max_length, uint32_t *data_length, uint32_t timeout)
{
	if(handle == NULL || data == NULL || data_length == NULL)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

	uint32_t status = RL_FALSE;

	status = rpmsg_queue_recv(handle->rpmsg_instance, handle->rpmsg_queue, (uint32_t *)&handle->remote_addr, data, max_length, data_length, timeout);

	if(status != RL_SUCCESS)
	{
		return kStatus_RIOP_Err;
	}

	return kStatus_RIOP_Ok;

}


riop_status_t ICC_WriteData(icc_handle_t * handle, void *data, uint32_t max_length, uint32_t timeout)
{
	if(handle == NULL || data == NULL)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

	uint32_t status = RL_FALSE;

	status = rpmsg_lite_send(handle->rpmsg_instance, handle->ept_instance, handle->remote_addr, data, max_length, timeout);

	if(status != RL_TRUE)
	{
		return kStatus_RIOP_Err;
	}

	return kStatus_RIOP_Ok;
}
