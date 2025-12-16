/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EDMA_PERMISSION_H_
#define _EDMA_PERMISSION_H_

#include "fsl_common_arm.h"

typedef enum
{
    kPermissionEdma3 = 1 << 0,
    kPermissionEdma4 = 1 << 1,
} PermissionEdmaInstance_t;

#if defined(__cplusplus)
extern "C"
{
#endif /*_cplusplus*/

    void EDMA_SetPermission(uint32_t mask);

#if defined(__cplusplus)
}
#endif /*_cplusplus*/
/*@}*/

#endif