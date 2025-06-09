/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/


#include "FreeRTOS.h"
#include "task.h"
#include "board.h"
#include "pin_mux.h"
#include "queue.h"
#include "fsl_debug_console.h"
#include "api_riop.h"
#include "riop_feature_config.h"
#include "nafe_hal.h"
#include "nafe13388.h"
#include "api_afe.h"
#include "afe_task.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/
extern QueueHandle_t afe_command_queue;
extern QueueHandle_t afe_hvsig_sample_queue;
extern QueueHandle_t afe_lvsig_sample_queue;
extern QueueHandle_t afe_current_sample_queue;
extern QueueHandle_t afe_gpio_input_status_queue;
extern TaskHandle_t AFE_TaskHandle;
extern QueueHandle_t afe_gpio_command_queue;
extern QueueHandle_t afe_temp_sample_queue;


extern NAFE_hdl_t gNafe_Hdl;
extern afe_calibration_t gVoltageCal;

static timer_handle_t timerHandle;

AFE_ctx_t gAfe_ctx = {
      .NafeHdl = &gNafe_Hdl,
	  .timerHandle = &timerHandle,
	  .periodical_measurment_ongoing = 0,
	  .schedule_error = 0,
	  .afe_shunt = kNone,
	  .afe_sampling_period_us = 0,
	  .afe_sampling_period_counter = 0,
};

/*******************************************************************************
 * Code
 ******************************************************************************/
void AFE_callback(void *data)
{
	NAFE_hdl_t *nafeHdl = data;

	if(nafeHdl->sampleMode == kNafeSampleMode_scsrNonBlock || nafeHdl->sampleMode == kNafeSampleMode_mcmrNonBlock)
	{
		float *src = NULL;
		uint16_t afe_gpio_input_status = 0;
		uint16_t afe_gpio_output_value = 0;
		NAFE_terminateSample(nafeHdl);
		float current = 0;

		if ((NAFE_hvInputPos_t)nafeHdl->chConfig[0].hvAin == kNafeHvInputPos_ai3p)
		{
			src = nafeHdl->data;
			current = *src/((float)gAfe_ctx.afe_shunt);
			xQueueSendFromISR(afe_current_sample_queue, &current, pdFALSE);
		}
		else
		{
			src = nafeHdl->data;
			xQueueSendFromISR(afe_hvsig_sample_queue, src, pdFALSE);
		}

		if(gAfe_ctx.afe_sampling_period_us < 10000)
		{
			if(gAfe_ctx.afe_sampling_period_counter >= 10)
			{
				gAfe_ctx.afe_sampling_period_counter = 0;
				afe_gpio_input_status = NAFE_gpioReadData(nafeHdl);
				xQueueSendFromISR(afe_gpio_input_status_queue, &afe_gpio_input_status, pdFALSE);
			}
			gAfe_ctx.afe_sampling_period_counter++;
		}
		else
		{
			afe_gpio_input_status = NAFE_gpioReadData(nafeHdl);
			xQueueSendFromISR(afe_gpio_input_status_queue, &afe_gpio_input_status, pdFALSE);
		}

		if( xQueueReceiveFromISR(afe_gpio_command_queue, &afe_gpio_output_value, pdFALSE) == pdPASS)
		{
			NAFE_gpioConfig(nafeHdl);
			NAFE_gpioWriteData(nafeHdl, afe_gpio_output_value);
		}

		nafeHdl->sampleInProgress = 0;
		xTaskNotifyFromISR(AFE_TaskHandle, 1, eSetValueWithOverwrite, pdFALSE);
	}
	else if(nafeHdl->sampleMode == kNafeSampleMode_sccrNonBlock || nafeHdl->sampleMode == kNafeSampleMode_mccrNonBlock)
	{
		float meanV[AFE_MAX_INTERNAL_CHANNELS];
		float meanT[AFE_MAX_TEMP_CHANNELS];
		float stdevV[AFE_MAX_INTERNAL_CHANNELS];
		float merged_data[2*AFE_MAX_INTERNAL_CHANNELS];
		uint8_t channels_configured[AFE_MAX_INTERNAL_CHANNELS];

		memset(meanV, 0, sizeof(meanV));
		memset(stdevV, 0, sizeof(stdevV));
		memset(merged_data, 0, sizeof(merged_data));
		memset(channels_configured, 0, sizeof(channels_configured));

		for(uint8_t i = 0; i < nafeHdl->chnAmt; i++ )
		{
			channels_configured[(nafeHdl->chConfig[i].lv)-1] = 1;
		}

		if(nafeHdl->chConfig[0].inputSel == kNafeInputSel_lvsig)
		{
			AFE_CalculateMean(nafeHdl, channels_configured, meanV, 1);
			AFE_CalculateStdev(nafeHdl,channels_configured, meanV, stdevV);

			nafeHdl->sampleInProgress = 0;

			memcpy(merged_data, meanV, AFE_MAX_INTERNAL_CHANNELS * sizeof(float));
			memcpy(merged_data + AFE_MAX_INTERNAL_CHANNELS, stdevV, AFE_MAX_INTERNAL_CHANNELS * sizeof(float)); // copy 4 floats from y to total[4]...total[7]

			xQueueSendFromISR(afe_lvsig_sample_queue, merged_data, pdFALSE);
		}
		else if ((nafeHdl->chConfig[0].inputSel == kNafeInputSel_hvsig) && (nafeHdl->chConfig[0].viexVi == kNafeViexVi_current))
		{
			AFE_CalculateMean(nafeHdl, channels_configured, meanT, 0);

			nafeHdl->sampleInProgress = 0;

			xQueueSendFromISR(afe_temp_sample_queue, meanT, pdFALSE);
		}
	}
}

void TIMER_callback(void *data)
{
	AFE_ctx_t * afe_ctx = data;

	if(afe_ctx->NafeHdl->sampleInProgress)
	{
		afe_ctx->schedule_error++;
	}
	else
	{
		afe_ctx->NafeHdl->sampleInProgress = 1;
		NAFE_startSample(afe_ctx->NafeHdl);
	}
}

void AFE_task(void *data)
{
	AFE_ctx_t *afe_ctx = data;

	afe_command_t afe_command;
	memset(&afe_command, 0, sizeof(afe_command));
	uint8_t all_channels_disabled = 0;

	afe_ctx->NafeHdl->callback = AFE_callback;
	AFE_installPeriodicMeasurementCallback(afe_ctx->timerHandle, TIMER_callback, afe_ctx);

	while(1)
	{
		xQueueReceive(afe_command_queue, &afe_command, portMAX_DELAY);

		if(afe_command.afe_gpio_command.is_dirty)
		{
			afe_ctx->NafeHdl->gpioConfig->writeEnable = afe_command.afe_gpio_command.gpio_pin_direction;
			RIOP_SetAfeDigitalGpioSettingsStatus(&afe_command.afe_gpio_command);

			if(afe_ctx->periodical_measurment_ongoing)
			{
				xQueueSend(afe_gpio_command_queue, &afe_command.afe_gpio_command.gpio_output_value, 0);
			}
			else
			{
				if(!(afe_ctx->NafeHdl->sampleInProgress))
				{
					NAFE_gpioConfig(afe_ctx->NafeHdl);
					NAFE_gpioWriteData(afe_ctx->NafeHdl, afe_command.afe_gpio_command.gpio_output_value);
				}
			}
		}

		if(afe_command.afe_hvsig_command.is_dirty || afe_command.afe_lvsig_command.is_dirty || afe_command.afe_current_command.is_dirty || afe_command.afe_temperature_command.is_dirty
				|| afe_command.afe_calibration_command.is_dirty)
		{
			if(afe_ctx->periodical_measurment_ongoing || afe_ctx->NafeHdl->sampleInProgress)
			{
				AFE_StopPeriodicMeasurement();
				afe_ctx->periodical_measurment_ongoing = 0;

				if(afe_ctx->NafeHdl->sampleInProgress)
				{
					AFE_WaitForMeasurementComplete(&AFE_TaskHandle, 2);
				}
			}

			if(afe_command.afe_hvsig_command.is_dirty)
			{
				all_channels_disabled = 1;
				/*Check, if at least one channel is enabled*/
				for(uint8_t i = 0; i< AFE_MAX_EXTERNAL_CHANNELS; i++)
				{
					if(afe_command.afe_hvsig_command.channel_enable[i])
					{
						all_channels_disabled = 0;
						break;
					}
				}
				if(all_channels_disabled)
				{
					if(afe_ctx->periodical_measurment_ongoing)
					{
						AFE_StopPeriodicMeasurement();
						afe_ctx->periodical_measurment_ongoing = 0;
					}
				}
				else
				{
					afe_ctx->afe_sampling_period_us = AFE_SAMPLING_PERIOD_US_TSN;
					if(AFE_StartPeriodicMeasurement(afe_ctx->NafeHdl, &afe_command, afe_ctx->timerHandle, afe_ctx->afe_sampling_period_us) != kStatus_RIOP_Ok)
					{
						continue;
					}
					afe_ctx->periodical_measurment_ongoing = 1;
				}
			}
			else if(afe_command.afe_lvsig_command.is_dirty)
			{
				if (AFE_StartSingleMeasurement(afe_ctx->NafeHdl, &afe_command) != kStatus_RIOP_Ok)
				{
					continue;
				}
			}
			else if(afe_command.afe_current_command.is_dirty)
			{
				if(!afe_command.afe_current_command.measurement_enabled)
				{
					AFE_StopPeriodicMeasurement();
					afe_ctx->periodical_measurment_ongoing = 0;
				}
				else
				{
					afe_ctx->afe_sampling_period_us = AFE_SAMPLING_PERIOD_US_TSN;
					afe_ctx->afe_shunt = afe_command.afe_current_command.shunt_value;

					if(AFE_StartPeriodicMeasurement(afe_ctx->NafeHdl, &afe_command, afe_ctx->timerHandle, afe_ctx->afe_sampling_period_us) != kStatus_RIOP_Ok)
					{
						continue;
					}
					afe_ctx->periodical_measurment_ongoing = 1;
				}
			}
			else if(afe_command.afe_temperature_command.is_dirty)
			{
				if (AFE_StartSingleMeasurement(afe_ctx->NafeHdl, &afe_command) != kStatus_RIOP_Ok)
				{
					continue;
				}
			}
			else if(afe_command.afe_calibration_command.is_dirty)
			{
				AFE_Calibration(afe_ctx->NafeHdl, &afe_command);
			}
		}
	}
}
