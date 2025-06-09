/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "SIGGEN_task.h"
#include "api_siggen.h"
#include "api_riop.h"
#include "math.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern QueueHandle_t siggen_command_queue;
extern SIGGEN_config_t gSIGGEN_config;

SIGGEN_ctx_t SIGGEN_ctx =
{
	.generate = 0,
	.siggenConfig = &gSIGGEN_config,
};


/*******************************************************************************
 * Code
 ******************************************************************************/
void SIGGEN_timerCallback(void *data)
{
	SIGGEN_config_t *siggen_config = data;

	static uint16_t sawtoothSamplePointer = 0;
	static uint16_t sine1SamplePointer = 0;
	static float sine1Val = 0;
	static uint16_t pwm1Val = 0;
	static uint16_t sine2SamplePointer = 0;
	static float sine2Val = 0;
	static uint16_t pwm2Val = 0;
	static uint16_t pwmVal = 0;
	static uint8_t phaseShiftFlag = 0;




	if(siggen_config->outputSignalType == kSignalSinus)
	{
		sine1Val = siggen_config->correctionFactor*0.65*sinf( (6.28/SAMPLE_CNT) * sine1SamplePointer );
		pwm1Val = 65535*0.4 + (65535*0.4 * sine1Val) ;

		PWM_UpdatePwmDutycycleHighAccuracy(PWM2, kPWM_Module_2, kPWM_PwmA, kPWM_EdgeAligned, pwm1Val);

		sine1SamplePointer++;

		if(sine1SamplePointer > (uint32_t)SAMPLE_CNT)
		{
			sine1SamplePointer = 0;
		}

		if(sine1SamplePointer == 50)
		{
			phaseShiftFlag = 1;
		}

		if(phaseShiftFlag == 1)
		{

			sine2Val = siggen_config->correctionFactor*0.65*sinf( (6.28/SAMPLE_CNT) * sine2SamplePointer );
			pwm2Val = 65535*0.4 + (65535*0.4 * sine2Val) ;

			PWM_UpdatePwmDutycycleHighAccuracy(PWM2, kPWM_Module_2, kPWM_PwmB, kPWM_EdgeAligned, pwm2Val);

			sine2SamplePointer++;

			if(sine2SamplePointer > (uint32_t)SAMPLE_CNT)
			{
				sine2SamplePointer = 0;
			}
		}

		PWM_SetPwmLdok(PWM2, kPWM_Control_Module_2, true);
	}
	else if(siggen_config->outputSignalType == kSignalSawtooth)
	{
		PWM_UpdatePwmDutycycleHighAccuracy(PWM2, kPWM_Module_2, kPWM_PwmA, kPWM_EdgeAligned, pwmVal);
		PWM_UpdatePwmDutycycleHighAccuracy(PWM2, kPWM_Module_2, kPWM_PwmB, kPWM_EdgeAligned, pwmVal);
		PWM_SetPwmLdok(PWM2, kPWM_Control_Module_2, true);

		sawtoothSamplePointer++;
		pwmVal = (pwmVal + 0.4*SAWTOOTH_DC_INCREMENT);
		if(sawtoothSamplePointer > (SAMPLE_CNT-1))
		{
			sawtoothSamplePointer = 0;
			pwmVal = 65535/8;
		}
	}
}


void SIGGEN_ControlTask(void *data)
{
	SIGGEN_ctx_t *siggen_ctx = data;
	siggen_control_command_t siggen_control_command;
	SIGGEN_config_t SIGGEN_config;

	memset(&siggen_control_command, 0, sizeof(siggen_control_command));
	memset(&SIGGEN_config, 0, sizeof(SIGGEN_config));

	while(1)
	{
		xQueueReceive(siggen_command_queue, &siggen_control_command, portMAX_DELAY);

		RIOP_SetSiggenStatus(&siggen_control_command);

		if(siggen_control_command.enable == 0)
		{
			SIGGEN_Stop();
		}
		else
		{
			if(siggen_ctx->generate)
			{
				SIGGEN_Stop();
			}
			siggen_ctx->siggenConfig->outputSignalFrequency = siggen_control_command.frequency;
			siggen_ctx->siggenConfig->outputSignalType = siggen_control_command.type;

			if(SIGGEN_ReInit(siggen_ctx->siggenConfig) != kStatus_RIOP_Ok)
			{
				continue;
			}

			if(SIGGEN_Start(siggen_ctx->siggenConfig, SIGGEN_timerCallback)!= kStatus_RIOP_Ok)
			{
				continue;
			}
			else
			{
				siggen_ctx->generate = 1;
			}
		}
	}
}
