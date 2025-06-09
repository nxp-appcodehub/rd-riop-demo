/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef API_RIOP_H_
#define API_RIOP_H_

#include "fsl_common.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "api_riop_common.h"
#include "riop_feature_config.h"
#include "api_siggen.h"


typedef enum
{
	kNone 		= 0U,
	kShunt100 	= 100U,
	kShunt125	= 125U,
	kShunt250 	= 250U,
}AFE_shunt_t;

typedef enum
{
	kVoltageCal = 0U,
	kResistanceCal,
}AFE_calibration_method_t;


/*Command structures*/

typedef struct _siggen_control_command
{
	uint8_t enable;
	uint8_t frequency;
	SIGGEN_signal_type_t type;
	uint8_t is_dirty;
}siggen_control_command_t;

typedef struct _afe_gpio_command
{
	uint16_t gpio_pin_direction;
	uint16_t gpio_output_value;
	uint8_t is_dirty;
}afe_gpio_command_t;

typedef struct _afe_hvsig_command
{
	uint8_t channel_enable[AFE_MAX_EXTERNAL_CHANNELS];
	uint8_t is_dirty;
}afe_hvsig_command_t;

typedef struct _afe_lvsig_command
{
	uint16_t samples_amt;
	uint8_t channel_enable[AFE_MAX_INTERNAL_CHANNELS];
	uint8_t is_dirty;
}afe_lvsig_command_t;

typedef struct _afe_current_command
{
	AFE_shunt_t shunt_value;
	uint8_t measurement_enabled;
	uint8_t is_dirty;
}afe_current_command_t;

typedef struct _afe_temperature_command
{
	uint16_t samples_amt;
	uint8_t measurement_enabled;
	uint8_t is_dirty;
}afe_temperature_command_t;

typedef struct _afe_calibration_command
{
	uint8_t calibration_start;
	double inserted_sample_1;
	double inserted_sample_2;
	AFE_calibration_method_t calibration_method;
	uint8_t is_dirty;
}afe_calibration_command_t;

typedef struct _afe_command
{
	afe_gpio_command_t afe_gpio_command;
	afe_hvsig_command_t afe_hvsig_command;
	afe_lvsig_command_t afe_lvsig_command;
	afe_current_command_t afe_current_command;
	afe_temperature_command_t afe_temperature_command;
	afe_calibration_command_t afe_calibration_command;
}afe_command_t;

typedef struct _mcu_digital_output_command
{
	uint8_t mcu_digital_output;
	uint8_t is_dirty;
}mcu_digital_output_command_t;

typedef struct _riop_command
{
	siggen_control_command_t siggen_control_command;
	afe_command_t afe_command;
	mcu_digital_output_command_t mcu_digital_output_command;
}riop_command_t;


/*Status structures*/

typedef struct _siggen_status
{
	uint8_t enabled;
	uint8_t frequency;
	SIGGEN_signal_type_t type;
}siggen_status_t;

typedef struct _afe_hvsig_status
{
	float channel_value[AFE_MAX_EXTERNAL_CHANNELS];
}afe_hvsig_status_t;

typedef struct _afe_lvsig_status
{
	float channel_mean[AFE_MAX_INTERNAL_CHANNELS];
	float channel_stdev[AFE_MAX_INTERNAL_CHANNELS];
}afe_lvsig_status_t;

typedef struct _afe_current_status
{
	float channel_value;
}afe_current_status_t;

typedef struct _afe_temperature_status
{
	float channel_value;
}afe_temperature_status_t;

typedef struct _afe_calibration_status
{
	uint32_t original_gain;
	uint32_t original_offset;
	uint32_t calculated_gain;
	int32_t calculated_offset;
	uint8_t calibration_phase;
}afe_calibration_status_t;

typedef struct _afe_status
{
	uint16_t afe_gpio_output_pins_status;
	uint16_t afe_gpio_pins_direction_status;
	uint16_t afe_gpio_input_pins_status;
	afe_hvsig_status_t hvsig_status;
	afe_lvsig_status_t lvsig_status;
	afe_current_status_t current_status;
	afe_temperature_status_t temperature_status;
	afe_calibration_status_t calibration_status;
} afe_status_t;

typedef struct _riop_board_status
{
	siggen_status_t siggen_status;
	uint8_t mcu_digital_input_pins_status;
	uint8_t	mcu_digital_output_pins_status;
	afe_status_t afe_status;
}riop_board_status_t;

typedef struct _riop_status_shm
{
	SemaphoreHandle_t *lock;
	riop_board_status_t plc_io_status;
}riop_status_shm_t;


/*API*/
riop_status_t RIOP_SetMcuDigitalInputPinsStatus(uint8_t mcu_pins);
riop_status_t RIOP_SetMcuDigitalOutputPinsStatus(uint8_t mcu_pins);
riop_status_t RIOP_SetAfeDigitalInputPinsStatus(uint16_t afe_pins);
riop_status_t RIOP_SetAfeDigitalGpioSettingsStatus(afe_gpio_command_t *afe_gpio_command);
riop_status_t RIOP_SetSiggenStatus(siggen_control_command_t *siggen_control_command);
riop_status_t RIOP_SetCalPhase(uint8_t cal_phase);
riop_status_t RIOP_SetCalibrationStatus(afe_calibration_status_t *calibration_status);
riop_status_t RIOP_GetBoardStatusForICC(riop_board_status_t *riop_status);


void RIOP_StatusShmInit(void);


#endif /* API_RIOP_H_ */
