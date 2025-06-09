/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "board.h"
#include "pin_mux.h"
#include "FreeRTOS.h"
#include "task.h"
#include "api_riop.h"
#include "riop_feature_config.h"
#include "api_afe.h"
#include "fsl_rgpio.h"
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
extern QueueHandle_t gpio_command_queue;
extern QueueHandle_t afe_gpio_input_status_queue;

/*******************************************************************************
 * Code
 ******************************************************************************/
void GPIO_ControlTask(void *data)
{
	AFE_ctx_t *afe_ctx = data;

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	uint8_t mcu_digital_input_pin[MAX_MCU_DIG_GPIO_INPUT_PINS];
	uint8_t mcu_digital_input_status = 0;
	uint8_t mcu_digital_output_command = 0;
	uint8_t mask = 0x01;
	uint16_t afe_digital_input_status = 0;

	while(1)
	{
		mcu_digital_input_status = 0;

		mcu_digital_input_pin[0] = RGPIO_PinRead(BOARD_DIG_IN1_GPIO, BOARD_DIG_IN1_GPIO_PIN);
		mcu_digital_input_pin[1] = RGPIO_PinRead(BOARD_DIG_IN2_GPIO, BOARD_DIG_IN2_GPIO_PIN);
		mcu_digital_input_pin[2] = RGPIO_PinRead(BOARD_DIG_IN3_GPIO, BOARD_DIG_IN3_GPIO_PIN);
		mcu_digital_input_pin[3] = RGPIO_PinRead(BOARD_DIG_IN4_GPIO, BOARD_DIG_IN4_GPIO_PIN);
		mcu_digital_input_pin[4] = RGPIO_PinRead(BOARD_DIG_IN5_GPIO, BOARD_DIG_IN5_GPIO_PIN);
		mcu_digital_input_pin[5] = RGPIO_PinRead(BOARD_DIG_IN6_GPIO, BOARD_DIG_IN6_GPIO_PIN);
		mcu_digital_input_pin[6] = RGPIO_PinRead(BOARD_DIG_IN7_GPIO, BOARD_DIG_IN7_GPIO_PIN);

		for(int8_t i = 6; i >= 0; i--)
		{
			mcu_digital_input_status |= (mcu_digital_input_pin[i] << i);
		}

		/*Set status of the digital input pins into the gRiop_status_shm*/
		RIOP_SetMcuDigitalInputPinsStatus(mcu_digital_input_status);
		if(!afe_ctx->periodical_measurment_ongoing && !afe_ctx->NafeHdl->sampleInProgress )
		{
			RIOP_SetAfeDigitalInputPinsStatus(NAFE_gpioReadData(afe_ctx->NafeHdl));
		}
		else if (afe_ctx->periodical_measurment_ongoing)
		{
			if(xQueueReceive(afe_gpio_input_status_queue, &afe_digital_input_status, 0) == pdPASS)
			{
				RIOP_SetAfeDigitalInputPinsStatus(afe_digital_input_status);
			}
		}
		else if(afe_ctx->NafeHdl->sampleInProgress)
		{

		}

		if(xQueueReceive(gpio_command_queue, &mcu_digital_output_command, 0) == pdPASS)
		{
			for(uint8_t i = 0; i < MAX_MCU_DIG_GPIO_OUTPUT_PINS; i++)
			{
				if(mcu_digital_output_command & (mask << i))
				{
					RGPIO_PinWrite(RGPIO2, i+21, 1);
				}
				else
				{
					RGPIO_PinWrite(RGPIO2, i+21, 0);
				}
			}
			RIOP_SetMcuDigitalOutputPinsStatus(mcu_digital_output_command);
		}
		/*Periodically reports status of the digital input pins*/
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));
	}
}

