/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef API_AFE_H_
#define API_AFE_H_


#include "api_riop_common.h"
#include "api_riop.h"
#include "nafe13388.h"
#include "fsl_gpt.h"

#define NAFE_DEV_ADDR           1u
#define GPIO_DEFAULT_DIRECTION  0x3FC
#define DEMO_SPI_BAUD_RATE      20000000


typedef void (*gpt_isr_t) (void *handle);
typedef void (*timer_callback_t)(void *data);

typedef struct _timer_handle
{
	timer_callback_t callback;
	void *userData;
}timer_handle_t;

typedef enum
{
	kCalStart = 0,
	kCalMeasurementPositive,
	kCalMeasurementNegative,
}calibration_phase_t;

typedef struct _afe_calibration
{
	calibration_phase_t phase;
	float inserted_sample[2];
	float measured_sample[2];
}afe_calibration_t;


riop_status_t AFE_Init(NAFE_hdl_t *nafeHdl);
riop_status_t AFE_ChannelConfiguration(NAFE_hdl_t *nafeHdl, afe_command_t *afe_command);
riop_status_t AFE_StartPeriodicMeasurement(NAFE_hdl_t *nafeHdl, afe_command_t *afe_command, timer_handle_t *timer_handle, uint32_t period);
void AFE_StopPeriodicMeasurement(void);
riop_status_t AFE_StartSingleMeasurement(NAFE_hdl_t *nafeHdl, afe_command_t *afe_command);
riop_status_t AFE_installPeriodicMeasurementCallback(timer_handle_t *timer_handle, timer_callback_t callback, void *userData);
riop_status_t AFE_CalculateMean(NAFE_hdl_t *nafeHdl, uint8_t * channels_configured, float * meanV, uint8_t voltage);
riop_status_t AFE_CalculateStdev(NAFE_hdl_t *nafeHdl, uint8_t * channels_configured, float * meanV, float *stdevV);
riop_status_t AFE_WaitForMeasurementComplete(TaskHandle_t *afe_task_handle, uint8_t timeout);
riop_status_t AFE_AdcCode2VoltageInternal(NAFE_hdl_t *nafeHdl, uint32_t input_data, float *result, uint8_t channelNum);
riop_status_t AFE_AdcCode2TemperatureInternal(NAFE_hdl_t *nafeHdl, uint32_t input_data, float *result);
riop_status_t AFE_Calibration(NAFE_hdl_t *nafeHdl, afe_command_t *afe_command);
#endif /* API_AFE_H_ */
