/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef API_SIGGEN_H_
#define API_SIGGEN_H_

#include "fsl_pwm.h"
#include "api_riop_common.h"
#include "fsl_lpit.h"

#define SAMPLE_CNT					150
#define SAWTOOTH_DC_INCREMENT		(65535/SAMPLE_CNT)

typedef void (*SIGGEN_timerCallback_t)(void *data);

typedef enum _SIGGEN_signal_type
{
	kSignalSinus = 0,
	kSignalSawtooth = 1,
}SIGGEN_signal_type_t;

typedef struct _SIGGEN_timer_handle
{
	lpit_config_t lpitConfig;
	lpit_chnl_params_t lpitChannelConfig;
	SIGGEN_timerCallback_t timerCallback;
} SIGGEN_timer_handle_t;



typedef struct _SIGGEN_config
{
	uint32_t outputSignalFrequency;
	SIGGEN_signal_type_t outputSignalType;
	float correctionFactor;
}SIGGEN_config_t;



riop_status_t SIGGEN_Init(SIGGEN_config_t *SIGGEN_config);
riop_status_t SIGGEN_ReInit(SIGGEN_config_t *SIGGEN_config);
riop_status_t SIGGEN_GetConfig(SIGGEN_config_t *SIGGEN_config);
riop_status_t SIGGEN_Start(SIGGEN_config_t *SIGGEN_config, SIGGEN_timerCallback_t callback);
void SIGGEN_Stop(void);



#endif /* API_SIGGEN_H_ */
