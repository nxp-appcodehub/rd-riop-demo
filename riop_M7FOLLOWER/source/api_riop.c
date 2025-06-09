/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "api_riop.h"
#include "api_afe.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void RIOP_LockMutex(SemaphoreHandle_t *lock);
static void RIOP_UnlockMutex(SemaphoreHandle_t *lock);

/*******************************************************************************
 * Variables
 ******************************************************************************/

riop_status_shm_t gRiop_status_shm = {

		.plc_io_status = {
							.siggen_status = { 0, 0, 0},
							.mcu_digital_input_pins_status = 0,
							.mcu_digital_output_pins_status = 0,
							.afe_status = {

									.afe_gpio_output_pins_status = 0,
									.afe_gpio_pins_direction_status = GPIO_DEFAULT_DIRECTION,
									.afe_gpio_input_pins_status = 0,
									.hvsig_status = {{0,0,0,}},
									.lvsig_status = {
														.channel_mean = {0,0,0,0,0,},
														.channel_stdev = {0,0,0,0,0,},
													},
									.current_status = {0.0},
									.temperature_status = {0.0},
							},
						 },
};

extern SemaphoreHandle_t riop_status_mutex;
extern QueueHandle_t afe_hvsig_sample_queue;
extern QueueHandle_t afe_lvsig_sample_queue;
extern QueueHandle_t afe_current_sample_queue;
extern QueueHandle_t afe_temp_sample_queue;

/*******************************************************************************
 * Code
 ******************************************************************************/
void RIOP_StatusShmInit(void)
{
	gRiop_status_shm.lock = &riop_status_mutex;
}

riop_status_t RIOP_SetMcuDigitalInputPinsStatus(uint8_t mcu_pins)
{
	RIOP_LockMutex(gRiop_status_shm.lock);

	gRiop_status_shm.plc_io_status.mcu_digital_input_pins_status = mcu_pins;

	RIOP_UnlockMutex(gRiop_status_shm.lock);

	return kStatus_RIOP_Ok;
}

riop_status_t RIOP_SetMcuDigitalOutputPinsStatus(uint8_t mcu_pins)
{
	RIOP_LockMutex(gRiop_status_shm.lock);

	gRiop_status_shm.plc_io_status.mcu_digital_output_pins_status = mcu_pins;

	RIOP_UnlockMutex(gRiop_status_shm.lock);

	return kStatus_RIOP_Ok;
}

riop_status_t RIOP_SetAfeDigitalInputPinsStatus(uint16_t afe_pins)
{
	RIOP_LockMutex(gRiop_status_shm.lock);

	gRiop_status_shm.plc_io_status.afe_status.afe_gpio_input_pins_status = afe_pins;

	RIOP_UnlockMutex(gRiop_status_shm.lock);

	return kStatus_RIOP_Ok;
}

riop_status_t RIOP_SetAfeDigitalGpioSettingsStatus(afe_gpio_command_t *afe_gpio_command )
{
	if(afe_gpio_command == NULL)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

	RIOP_LockMutex(gRiop_status_shm.lock);

	gRiop_status_shm.plc_io_status.afe_status.afe_gpio_pins_direction_status = afe_gpio_command->gpio_pin_direction;
	gRiop_status_shm.plc_io_status.afe_status.afe_gpio_output_pins_status = afe_gpio_command->gpio_output_value;

	RIOP_UnlockMutex(gRiop_status_shm.lock);

	return kStatus_RIOP_Ok;
}

riop_status_t RIOP_SetSiggenStatus(siggen_control_command_t *siggen_control_command)
{
	if(siggen_control_command == NULL)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

	RIOP_LockMutex(gRiop_status_shm.lock);

	gRiop_status_shm.plc_io_status.siggen_status.enabled = siggen_control_command->enable;
	gRiop_status_shm.plc_io_status.siggen_status.frequency = siggen_control_command->frequency;
	gRiop_status_shm.plc_io_status.siggen_status.type = siggen_control_command->type;

	RIOP_UnlockMutex(gRiop_status_shm.lock);

	return kStatus_RIOP_Ok;
}

riop_status_t RIOP_SetCalPhase(uint8_t cal_phase)
{
	RIOP_LockMutex(gRiop_status_shm.lock);
	gRiop_status_shm.plc_io_status.afe_status.calibration_status.calibration_phase = cal_phase;
	RIOP_UnlockMutex(gRiop_status_shm.lock);

	return kStatus_RIOP_Ok;

}


riop_status_t RIOP_SetCalibrationStatus(afe_calibration_status_t * afe_calibration)
{
	RIOP_LockMutex(gRiop_status_shm.lock);
	gRiop_status_shm.plc_io_status.afe_status.calibration_status.original_gain = afe_calibration->original_gain;
	gRiop_status_shm.plc_io_status.afe_status.calibration_status.original_offset = afe_calibration->original_offset;
	gRiop_status_shm.plc_io_status.afe_status.calibration_status.calculated_gain = afe_calibration->calculated_gain;
	gRiop_status_shm.plc_io_status.afe_status.calibration_status.calculated_offset = afe_calibration->calculated_offset;
	RIOP_UnlockMutex(gRiop_status_shm.lock);

	return kStatus_RIOP_Ok;
}

riop_status_t RIOP_GetBoardStatusForICC(riop_board_status_t *riop_board_status)
{
	if(riop_board_status == NULL)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

	float channel_value[AFE_MAX_EXTERNAL_CHANNELS];
	float lvsig_values[2*AFE_MAX_INTERNAL_CHANNELS];
	float current_value = 0;
	float temperature_value = 0;

	memset(channel_value, 0, sizeof(channel_value));
	memset(lvsig_values, 0, sizeof(lvsig_values));

	RIOP_LockMutex(gRiop_status_shm.lock);

	if(xQueueReceive(afe_hvsig_sample_queue, channel_value , 0 ) == pdPASS)
	{
		memcpy(&gRiop_status_shm.plc_io_status.afe_status.hvsig_status.channel_value, channel_value, sizeof(afe_hvsig_status_t));
	}

	if(xQueueReceive(afe_lvsig_sample_queue, lvsig_values , 0 ) == pdPASS)
	{
		memcpy(&gRiop_status_shm.plc_io_status.afe_status.lvsig_status.channel_mean, lvsig_values, sizeof(lvsig_values)/2);
		memcpy(&gRiop_status_shm.plc_io_status.afe_status.lvsig_status.channel_stdev, lvsig_values+AFE_MAX_INTERNAL_CHANNELS , sizeof(lvsig_values)/2);
	}

	if(xQueueReceive(afe_current_sample_queue, &current_value , 0 ) == pdPASS)
	{
		memcpy(&gRiop_status_shm.plc_io_status.afe_status.current_status.channel_value, &current_value, sizeof(afe_current_status_t));
	}

	if(xQueueReceive(afe_temp_sample_queue, &temperature_value , 0 ) == pdPASS)
	{
		memcpy(&gRiop_status_shm.plc_io_status.afe_status.temperature_status.channel_value, &temperature_value, sizeof(afe_temperature_status_t));
	}

	memcpy(riop_board_status, &gRiop_status_shm.plc_io_status, sizeof(riop_board_status_t));

	RIOP_UnlockMutex(gRiop_status_shm.lock);

	return kStatus_RIOP_Ok;
}

static void RIOP_LockMutex(SemaphoreHandle_t *lock)
{
    (void)xSemaphoreTake(*lock, portMAX_DELAY);
}

static void RIOP_UnlockMutex(SemaphoreHandle_t *lock)
{
    (void)xSemaphoreGive(*lock);
}


