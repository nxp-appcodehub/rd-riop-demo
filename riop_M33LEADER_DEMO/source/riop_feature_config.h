/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef RIOP_FEATURE_CONFIG_H_
#define RIOP_FEATURE_CONFIG_H_

#define MAX_MCU_DIG_GPIO_INPUT_PINS			(7)
#define MAX_MCU_DIG_GPIO_OUTPUT_PINS		(8)

#define AFE_MAX_EXTERNAL_CHANNELS			(3)
#define AFE_MAX_INTERNAL_CHANNELS		    (5)
#define AFE_MAX_CURRENT_CHANNELS    		(1)
#define AFE_MAX_TEMP_CHANNELS    			(1)


#define AFE_LV_MAX_SAMPLES					2000
#define AFE_TEMP_MAX_SAMPLES				2000
#define AFE_MIN_SAMPLING_PERIOD_US_TSN		1000
#define AFE_MAX_SAMPLING_PERIOD_US_TSN		100000
#define AFE_CALL_SAMPLES					100


#define AFE_SAMPLING_PERIOD_US_TSN  1000



#endif /* RIOP_FEATURE_CONFIG_H_ */
