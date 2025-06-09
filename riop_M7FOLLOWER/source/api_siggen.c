/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "api_siggen.h"
#include "fsl_xbar.h"
#include "math.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PWM_FREQUENCY_HZ 			30000UL
#define SIGGEN_TIMER_ISR			LPIT1_IRQHandler


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static riop_status_t PWM_InitSignals();
static riop_status_t SIGGEN_InitLpit(SIGGEN_config_t *SIGGEN_config);
static void SIGGEN_GetDefaultConfig(SIGGEN_config_t *SIGGEN_config);


/*******************************************************************************
 * Variables
 ******************************************************************************/
static pwm_config_t pwmConfig;
static pwm_fault_param_t faultConfig;
static SIGGEN_timer_handle_t SIGGEN_timer_handle;

SIGGEN_config_t gSIGGEN_config;


/*******************************************************************************
 * Code
 ******************************************************************************/
riop_status_t SIGGEN_Init(SIGGEN_config_t *SIGGEN_config)
{
	if(SIGGEN_config == NULL)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

	SIGGEN_GetDefaultConfig(SIGGEN_config);

	XBAR_Init(kXBAR_DSC1);
	if(XBAR_SetSignalsConnection(kXBAR1_InputLogicHigh, kXBAR1_OutputFlexpwm2Fault0) != kStatus_Success)
	{
		return kStatus_RIOP_Err;
	}

	if(XBAR_SetSignalsConnection(kXBAR1_InputLogicHigh, kXBAR1_OutputFlexpwm2Fault1) != kStatus_Success)
	{
		return kStatus_RIOP_Err;
	}

	if(XBAR_SetSignalsConnection(kXBAR1_InputLogicHigh, kXBAR1_OutputFlexpwm1234Fault2) != kStatus_Success)
	{
		return kStatus_RIOP_Err;
	}

	if(XBAR_SetSignalsConnection(kXBAR1_InputLogicHigh, kXBAR1_OutputFlexpwm1234Fault3)!= kStatus_Success)
	{
		return kStatus_RIOP_Err;
	}

	PWM_GetDefaultConfig(&pwmConfig);

	pwmConfig.prescale = kPWM_Prescale_Divide_4;

	pwmConfig.reloadLogic = kPWM_ReloadPwmFullCycle;
	/* PWM A & PWM B form a complementary PWM pair */
	pwmConfig.pairOperation   = kPWM_Independent;
	pwmConfig.enableDebugMode = true;

	/* Initialize submodule 2 */
	if (PWM_Init(PWM2, kPWM_Module_2, &pwmConfig) == kStatus_Fail)
	{
		return kStatus_RIOP_Err;
	}

	PWM_FaultDefaultConfig(&faultConfig);

	/* Sets up the PWM fault protection */
	PWM_SetupFaults(PWM2, kPWM_Fault_0, &faultConfig);
	PWM_SetupFaults(PWM2, kPWM_Fault_1, &faultConfig);
	PWM_SetupFaults(PWM2, kPWM_Fault_2, &faultConfig);
	PWM_SetupFaults(PWM2, kPWM_Fault_3, &faultConfig);

	/* Set PWM fault disable mapping for submodule 0/1/2 */
	PWM_SetupFaultDisableMap(PWM2, kPWM_Module_2, kPWM_PwmA, kPWM_faultchannel_0,
	                         kPWM_FaultDisable_0 | kPWM_FaultDisable_1 | kPWM_FaultDisable_2 | kPWM_FaultDisable_3);

	/* Call the init function with demo configuration */
	if(PWM_InitSignals() != kStatus_RIOP_Ok)
	{
		return kStatus_RIOP_Err;
	}

	/* Set the load okay bit for all submodules to load registers from their buffer */
	PWM_SetPwmLdok(PWM2, kPWM_Control_Module_2, true);

	if(SIGGEN_InitLpit(SIGGEN_config) != kStatus_RIOP_Ok)
	{
		return kStatus_RIOP_Err;
	}

	SIGGEN_config->correctionFactor = (0.000006*(SIGGEN_config->outputSignalFrequency*SIGGEN_config->outputSignalFrequency) + 0.0008*SIGGEN_config->outputSignalFrequency + 1.0121 );

	return kStatus_RIOP_Ok;
}


riop_status_t SIGGEN_ReInit(SIGGEN_config_t *SIGGEN_config)
{
	if(SIGGEN_config == NULL)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

	SIGGEN_Stop();
	SIGGEN_config->correctionFactor = (0.000006*(SIGGEN_config->outputSignalFrequency*SIGGEN_config->outputSignalFrequency) + 0.0008*SIGGEN_config->outputSignalFrequency + 1.0121 );

	return kStatus_RIOP_Ok;
}


riop_status_t SIGGEN_GetConfig(SIGGEN_config_t *SIGGEN_config)
{
	if(SIGGEN_config == NULL)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

	SIGGEN_config = &gSIGGEN_config;

	return kStatus_RIOP_Ok;
}


riop_status_t SIGGEN_Start(SIGGEN_config_t *SIGGEN_config, SIGGEN_timerCallback_t callback)
{
	if(SIGGEN_config == NULL || callback == NULL)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

	SIGGEN_timer_handle.timerCallback = callback;
	PWM_StartTimer(PWM2, kPWM_Control_Module_2);
	SIGGEN_InitLpit(SIGGEN_config);
	LPIT_StartTimer(LPIT1, kLPIT_Chnl_0);

	return kStatus_RIOP_Ok;
}

void SIGGEN_Stop(void)
{
	PWM_StopTimer(PWM2, kPWM_Control_Module_2);
	LPIT_StopTimer(LPIT1, kLPIT_Chnl_0);
}


static riop_status_t PWM_InitSignals(uint32_t PWM_frequency)
{
	uint16_t deadTimeVal;
	pwm_signal_param_t pwmSignal[2];
	uint32_t pwmSourceClockInHz = CLOCK_GetRootClockFreq(kCLOCK_Root_Bus_Wakeup);;

	/* Set deadtime count, we set this to about 650ns */
	deadTimeVal = ((uint64_t)pwmSourceClockInHz * 650) / 1000000000;

	pwmSignal[0].pwmChannel       = kPWM_PwmA;
	pwmSignal[0].level            = kPWM_HighTrue;
	pwmSignal[0].dutyCyclePercent = 50; /* 1 percent dutycycle */
	pwmSignal[0].deadtimeValue    = deadTimeVal;
	pwmSignal[0].faultState       = kPWM_PwmFaultState0;
	pwmSignal[0].pwmchannelenable = true;

	pwmSignal[1].pwmChannel = kPWM_PwmB;
	pwmSignal[1].level      = kPWM_HighTrue;
	/* Dutycycle field of PWM B does not matter as we are running in PWM A complementary mode */
	pwmSignal[1].dutyCyclePercent = 50;
	pwmSignal[1].deadtimeValue    = deadTimeVal;
	pwmSignal[1].faultState       = kPWM_PwmFaultState0;
	pwmSignal[1].pwmchannelenable = true;

	/*********** PWMA_SM0 - phase A, configuration, setup 2 channel as an example ************/
	if(PWM_SetupPwm(PWM2, kPWM_Module_2, pwmSignal, 2, kPWM_EdgeAligned, PWM_FREQUENCY_HZ, pwmSourceClockInHz) != kStatus_Success)
	{
		return kStatus_RIOP_Err;
	}

	return kStatus_RIOP_Ok;
}

static riop_status_t SIGGEN_InitLpit(SIGGEN_config_t *SIGGEN_config)
{
	if(SIGGEN_config == NULL)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

	LPIT_GetDefaultConfig(&SIGGEN_timer_handle.lpitConfig);

	LPIT_Init(LPIT1, &SIGGEN_timer_handle.lpitConfig);

	SIGGEN_timer_handle.lpitChannelConfig.chainChannel          = false;
	SIGGEN_timer_handle.lpitChannelConfig.enableReloadOnTrigger = false;
	SIGGEN_timer_handle.lpitChannelConfig.enableStartOnTrigger  = false;
	SIGGEN_timer_handle.lpitChannelConfig.enableStopOnTimeout   = false;
	SIGGEN_timer_handle.lpitChannelConfig.timerMode             = kLPIT_PeriodicCounter;

	/* Set default values for the trigger source */
	SIGGEN_timer_handle.lpitChannelConfig.triggerSelect = kLPIT_Trigger_TimerChn0;
	SIGGEN_timer_handle.lpitChannelConfig.triggerSource = kLPIT_TriggerSource_External;

	/* Init lpit channel 0 */
	if(LPIT_SetupChannel(LPIT1, kLPIT_Chnl_0, &SIGGEN_timer_handle.lpitChannelConfig) != kStatus_Success)
	{
		return kStatus_RIOP_Err;
	}

	uint32_t lpit_period = USEC_TO_COUNT( ((1.0/SIGGEN_config->outputSignalFrequency/(SAMPLE_CNT)) * 1000000), CLOCK_GetRootClockFreq(kCLOCK_Root_Bus_Aon));

	/* Set timer period for channel 0 */
	LPIT_SetTimerPeriod(LPIT1, kLPIT_Chnl_0, lpit_period); //10000 for 1 Hz;

	LPIT_EnableInterrupts(LPIT1, kLPIT_Channel0TimerInterruptEnable);

	EnableIRQ(LPIT1_IRQn);
	IRQ_SetPriority(LPIT1_IRQn, 2);

	return kStatus_RIOP_Ok;
}

static void SIGGEN_GetDefaultConfig(SIGGEN_config_t *SIGGEN_config)
{
	SIGGEN_config->outputSignalFrequency = 1;
	SIGGEN_config->outputSignalType = kSignalSinus;
}


void SIGGEN_TIMER_ISR(void)
{
	__disable_irq();
	LPIT_ClearStatusFlags(LPIT1, kLPIT_Channel0TimerFlag);
	SIGGEN_timer_handle.timerCallback(&gSIGGEN_config);
	__enable_irq();
	SDK_ISR_EXIT_BARRIER;
}




