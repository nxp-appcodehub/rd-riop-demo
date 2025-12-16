/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef API_RIOP_COMMON_H_
#define API_RIOP_COMMON_H_

#include "fsl_common.h"



typedef enum _riop_status
{
    kStatus_RIOP_Ok          		= kStatus_Success,                         /*!< Operation was successful; no errors occurred. */
    kStatus_RIOP_Err         		= kStatus_Fail,                            /*!< General error; not further specified. */
	kStatus_RIOP_ErrArgInvalid 	= kStatus_InvalidArgument,                 /*!< Argument invalid e.g. wrong type, NULL pointer, etc. */
	kStatus_RIOP_ErrRange      	= kStatus_OutOfRange,					   /*!< Error in case input argument is out of range e.g. array out of bounds, value not defined in enumeration. */

} riop_status_t;



#endif /* API_RIOP_COMMON_H_ */
