/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "api_riop_common.h"
#include "api_riop.h"
#include "api_afe.h"
#include "pin_mux.h"
#include "math.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define GPT_IRQ_ID             GPT2_IRQn
#define TIMER_ISR              GPT2_IRQHandler


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static riop_status_t TIMER_Init(void);
static riop_status_t TIMER_Start(uint32_t interval_us);
static void TIMER_Stop(void);
static void TIMER_HandleIRQ(void *irqHandle);


/*******************************************************************************
 * Variables
 ******************************************************************************/
static float 	gDataNonContinous[AFE_MAX_EXTERNAL_CHANNELS];
static uint32_t gDataContinous[AFE_MAX_INTERNAL_CHANNELS*AFE_LV_MAX_SAMPLES];
static float    gDataContinousCall[AFE_CALL_SAMPLES];
static float	gDataSingleSample[AFE_MAX_CURRENT_CHANNELS];

static NAFE_chnConfig_t gChnConfigArray_external[AFE_MAX_EXTERNAL_CHANNELS];
static NAFE_chnConfig_t gChnConfigArray_internal[AFE_MAX_INTERNAL_CHANNELS];
static NAFE_chnConfig_t gChnConfigArray_current[AFE_MAX_CURRENT_CHANNELS];
static NAFE_chnConfig_t gChnConfigArray_temp[AFE_MAX_TEMP_CHANNELS];
static NAFE_chnConfig_t gChnConfigArray_call;

static NAFE_gpioConfig_t gGpioConfig;
static NAFE_sysConfig_t gSysConfig;

static NAFE_HAL_hdl_t gHalHdl = {
    .spi                = BOARD_NAFE_SPI_INSTANCE,
    .spiBaudrate        = DEMO_SPI_BAUD_RATE,
    .csGpio             = AFE_CS_GPIO,
    .csGpioPin          = AFE_CS_GPIO_PIN,
    .drdyGpio           = AFE_DRDY_GPIO,
    .drdyGpioPin        = AFE_DRDY_GPIO_PIN,
    .syncGpio           = AFE_SYNC_GPIO,
    .syncGpioPin        = AFE_SYNC_GPIO_PIN,
    .rstbGpio           = AFE_RST_GPIO,
    .rstbGpioPin        = AFE_RST_GPIO_PIN,
};

NAFE_hdl_t gNafe_Hdl = {
	.sysConfig = &gSysConfig,
	.halHdl = &gHalHdl,
	.gpioConfig = &gGpioConfig,
	.chConfig = NULL,
	.devAddr = NAFE_DEV_ADDR,
	.readResultFrameMode = kNafeReadResult_singleFrame,
	.sampleMode = kNafeSampleMode_none,
	.data = NULL,
	.chnAmt = 0,
	.contSampleAmt = 0,
	.blockingCallback = NULL,
	.sampleInProgress = 0,
	.useRawData = 0,
};

afe_calibration_t gAfeCal;
afe_calibration_status_t gAfeCalStatus;


gpt_isr_t s_gptIsr;
void *s_timerHandle;


/*******************************************************************************
 * Code
 ******************************************************************************/
riop_status_t AFE_Init(NAFE_hdl_t *nafeHdl)
{
	if(nafeHdl == NULL)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

	status_t result = kStatus_Fail;

	nafeHdl->sysConfig->adcResolutionCode = kNafeAdcResolution_16bits;
	nafeHdl->sysConfig->triggerMode = kNafeTrigger_syncPin;
	nafeHdl->sysConfig->drdyPinSeqMode = kNafeDrdyPinSeqMode_onSequencer;

    for(uint8_t i = 0; i < AFE_MAX_EXTERNAL_CHANNELS; i++)
    {
    	gChnConfigArray_external[i].chnIndex = i;
    	gChnConfigArray_external[i].chChop = kNafeChChop_normalMode;
    	gChnConfigArray_external[i].inputSel = kNafeInputSel_hvsig;
    	gChnConfigArray_external[i].gain = kNafeChnGain_1x;
    	gChnConfigArray_external[i].gainCal = kNafeChnGain_1x_cal;
    	gChnConfigArray_external[i].dataRateCode = 4u;
    	gChnConfigArray_external[i].adcSinc = kNafeAdcSinc_sinc4;
    	gChnConfigArray_external[i].chDelayCode = 0x12u;
    	gChnConfigArray_external[i].adcSettling = kNafeAdcSettling_singleCycle;
    	gChnConfigArray_external[i].viexVi = kNafeViexVi_voltage;
    	gChnConfigArray_external[i].viexPol = kNafeViexPol_positive;
    	gChnConfigArray_external[i].viexMag = kNafeViexMag_0mV_0uA;
    	gChnConfigArray_external[i].viexChop = kNafeViexChop_disable;
    	gChnConfigArray_external[i].viexAipEn = kNafeViexAipEn_none;
    	gChnConfigArray_external[i].viexAinEn = kNafeViexAinEn_none;
    	gChnConfigArray_external[i].tcc  = kNafeTempCorr_Off;
    }

    for(uint8_t i = 0; i < AFE_MAX_INTERNAL_CHANNELS; i++)
	{
		gChnConfigArray_internal[i].chnIndex = i;
		gChnConfigArray_internal[i].chChop = kNafeChChop_normalMode;
		gChnConfigArray_internal[i].inputSel = kNafeInputSel_lvsig;
		gChnConfigArray_internal[i].gain = kNafeChnGain_0p2x;
		gChnConfigArray_internal[i].gainCal = kNafeChnGain_0p2x_cal;
		gChnConfigArray_internal[i].dataRateCode = 4u;
		gChnConfigArray_internal[i].adcSinc = kNafeAdcSinc_sinc4;
		gChnConfigArray_internal[i].chDelayCode = 0x12u;
		gChnConfigArray_internal[i].adcSettling = kNafeAdcSettling_singleCycle;
		gChnConfigArray_internal[i].viexVi = kNafeViexVi_voltage;
		gChnConfigArray_internal[i].viexPol = kNafeViexPol_positive;
		gChnConfigArray_internal[i].viexMag = kNafeViexMag_0mV_0uA;
		gChnConfigArray_internal[i].viexChop = kNafeViexChop_disable;
		gChnConfigArray_internal[i].viexAipEn = kNafeViexAipEn_none;
		gChnConfigArray_internal[i].viexAinEn = kNafeViexAinEn_none;
		gChnConfigArray_internal[i].tcc  = kNafeTempCorr_Off;
	}

	gChnConfigArray_current[0].chnIndex = 0;
	gChnConfigArray_current[0].chChop = kNafeChChop_normalMode;
	gChnConfigArray_current[0].inputSel = kNafeInputSel_hvsig;
	gChnConfigArray_current[0].lv 	= kNafeLvInput_halfRef_halfRef;
	gChnConfigArray_current[0].dataRateCode = 5u;
	gChnConfigArray_current[0].adcSinc = kNafeAdcSinc_sinc4;
	gChnConfigArray_current[0].chDelayCode = 11;
	gChnConfigArray_current[0].adcSettling = kNafeAdcSettling_singleCycle;
	gChnConfigArray_current[0].viexVi = kNafeViexVi_voltage;
	gChnConfigArray_current[0].viexPol = kNafeViexPol_positive;
	gChnConfigArray_current[0].viexMag = kNafeViexMag_0mV_0uA;
	gChnConfigArray_current[0].viexChop = kNafeViexChop_disable;
	gChnConfigArray_current[0].viexAipEn = kNafeViexAipEn_none;
	gChnConfigArray_current[0].viexAinEn = kNafeViexAinEn_none;
	gChnConfigArray_current[0].tcc  = kNafeTempCorr_On;


	gChnConfigArray_temp[0].chnIndex = 0;
	gChnConfigArray_temp[0].gain = kNafeChnGain_16x;
	gChnConfigArray_temp[0].inputSel = kNafeInputSel_hvsig;
	gChnConfigArray_temp[0].lv 	= kNafeLvInput_halfRef_halfRef;
	gChnConfigArray_temp[0].tcc  = kNafeTempCorr_Off;
	gChnConfigArray_temp[0].gainCal = kNafeChnGain_16x_cal;
	gChnConfigArray_temp[0].dataRateCode = 0x16u;
	gChnConfigArray_temp[0].adcSinc = kNafeAdcSinc_sinc4_sinc1;
	gChnConfigArray_temp[0].chDelayCode = 11;
	gChnConfigArray_temp[0].adcSettling = kNafeAdcSettling_normal;
	gChnConfigArray_temp[0].chChop = kNafeChChop_normalMode;
	gChnConfigArray_temp[0].viexVi = kNafeViexVi_current;
	gChnConfigArray_temp[0].viexPol = kNafeViexPol_positive;
	gChnConfigArray_temp[0].viexMag = kNafeViexMag_4p5V_750uA;
	gChnConfigArray_temp[0].viexChop = kNafeViexChop_disable;
	gChnConfigArray_temp[0].viexAipEn = kNafeViexAipEn_none;
	gChnConfigArray_temp[0].viexAinEn = kNafeViexAinEn_ai2n;


	gChnConfigArray_call.chnIndex = 0;
	gChnConfigArray_call.inputSel = kNafeInputSel_hvsig;
	gChnConfigArray_call.lv 	= kNafeLvInput_halfRef_halfRef;
	gChnConfigArray_call.chChop = kNafeChChop_normalMode;
	gChnConfigArray_call.viexPol = kNafeViexPol_positive;
	gChnConfigArray_call.viexChop = kNafeViexChop_disable;
	gChnConfigArray_call.viexAipEn = kNafeViexAipEn_none;




	nafeHdl->gpioConfig->connectToPin = 0x3FF;
	nafeHdl->gpioConfig->readEnable = 0x3FF;
	nafeHdl->gpioConfig->writeEnable = GPIO_DEFAULT_DIRECTION;

    result = NAFE_init(nafeHdl);
	if(result != kStatus_Success)
	{
		return kStatus_RIOP_Err;
	}

	if(TIMER_Init() != kStatus_RIOP_Ok)
	{
		return kStatus_RIOP_Err;
	}

    return kStatus_RIOP_Ok;
}

riop_status_t AFE_ChannelConfiguration(NAFE_hdl_t *nafeHdl, afe_command_t *afe_command)
{
	if(nafeHdl == NULL || afe_command == NULL)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

	uint8_t channels_used = 0;

	if(afe_command->afe_hvsig_command.is_dirty)
	{
		nafeHdl->chConfig = gChnConfigArray_external;
		nafeHdl->contSampleAmt = 1;
		nafeHdl->data = gDataNonContinous;
		nafeHdl->useRawData = 0;

		for(uint8_t i = 0; i < AFE_MAX_EXTERNAL_CHANNELS; i++)
		{
			if(afe_command->afe_hvsig_command.channel_enable[i])
			{
				if(i == 0)
				{
					gChnConfigArray_external[channels_used].hvAip = (NAFE_hvInputPos_t)(kNafeHvInputPos_ai1p);
					gChnConfigArray_external[channels_used].hvAin = (NAFE_hvInputNeg_t)(kNafeHvInputPos_aicom);
					channels_used++;
				}
				else if(i == 1)
				{
					gChnConfigArray_external[channels_used].hvAip = (NAFE_hvInputPos_t)(kNafeHvInputPos_aicom);
					gChnConfigArray_external[channels_used].hvAin = (NAFE_hvInputNeg_t)(kNafeHvInputNeg_ai1n);
					channels_used++;
				}
				else
				{
					gChnConfigArray_external[channels_used].hvAip = (NAFE_hvInputPos_t)(kNafeHvInputPos_ai1p);
					gChnConfigArray_external[channels_used].hvAin = (NAFE_hvInputNeg_t)(kNafeHvInputNeg_ai1n);
					channels_used++;
				}
			}
		}
		if(channels_used == 1)
		{
			nafeHdl->sampleMode = kNafeSampleMode_scsrNonBlock;
		}
		else
		{
			nafeHdl->sampleMode = kNafeSampleMode_mcmrNonBlock;
		}
	}
	else if(afe_command->afe_lvsig_command.is_dirty)
	{
		nafeHdl->chConfig = gChnConfigArray_internal;
		nafeHdl->useRawData = 1;
		nafeHdl->data = gDataContinous;

		if(afe_command->afe_lvsig_command.samples_amt <= AFE_LV_MAX_SAMPLES)
		{
			nafeHdl->contSampleAmt = afe_command->afe_lvsig_command.samples_amt;
		}
		else
		{
			nafeHdl->contSampleAmt = 2000;
		}

		for(uint8_t i = 0; i < AFE_MAX_INTERNAL_CHANNELS; i++)
		{
			if(afe_command->afe_lvsig_command.channel_enable[i])
			{
				gChnConfigArray_internal[channels_used].hvAip = (NAFE_hvInputPos_t)(i+0xA);
				gChnConfigArray_internal[channels_used].hvAin = (NAFE_hvInputNeg_t)(i+0xA);
				gChnConfigArray_internal[channels_used].lv = (NAFE_lvInput_t)(kNafeLvInput_gpio0_gpio1 + i);
				channels_used++;
			}
		}
		if(channels_used == 1)
		{
			nafeHdl->sampleMode = kNafeSampleMode_sccrNonBlock;
		}
		else
		{
			nafeHdl->sampleMode = kNafeSampleMode_mccrNonBlock;
		}
	}
	else if(afe_command->afe_current_command.is_dirty)
	{
		nafeHdl->chConfig = gChnConfigArray_current;
		nafeHdl->contSampleAmt = 1;
		nafeHdl->data = &gDataSingleSample;
		nafeHdl->useRawData = 0;
		nafeHdl->sampleMode = kNafeSampleMode_scsrNonBlock;
		channels_used = 1;

		gChnConfigArray_current[0].hvAip = (NAFE_hvInputPos_t)(kNafeHvInputPos_ai3p);
		gChnConfigArray_current[0].hvAin = (NAFE_hvInputNeg_t)(kNafeHvInputNeg_ai3n);

		switch(afe_command->afe_current_command.shunt_value)
		{
			case kShunt100:
				gChnConfigArray_current[0].gain = kNafeChnGain_1x;
				break;

			case kShunt125:
				gChnConfigArray_current[0].gain = kNafeChnGain_0p8x;
				break;

			case kShunt250:
				gChnConfigArray_current[0].gain = kNafeChnGain_0p4x;
				break;

			default:
				return kStatus_RIOP_ErrArgInvalid;
				break;
		}
	}
	else if(afe_command->afe_temperature_command.is_dirty)
	{
		nafeHdl->chConfig = gChnConfigArray_temp;
		nafeHdl->useRawData = 1;
		nafeHdl->data = gDataContinous;
		nafeHdl->sampleMode = kNafeSampleMode_sccrNonBlock;
		channels_used = 1;

		gChnConfigArray_temp[0].hvAip = (NAFE_hvInputPos_t)(kNafeHvInputPos_ai4p);
		gChnConfigArray_temp[0].hvAin = (NAFE_hvInputNeg_t)(kNafeHvInputNeg_ai4n);

		if(afe_command->afe_temperature_command.samples_amt <= AFE_TEMP_MAX_SAMPLES)
		{
			nafeHdl->contSampleAmt = afe_command->afe_temperature_command.samples_amt;
		}
		else
		{
			nafeHdl->contSampleAmt = 2000;
		}
	}
	else if(afe_command->afe_calibration_command.is_dirty)
	{

		nafeHdl->chConfig = &gChnConfigArray_call;
		nafeHdl->useRawData = 0;
		nafeHdl->data = gDataContinousCall;
		nafeHdl->sampleMode = kNafeSampleMode_sccrBlock;
		channels_used = 1;

		if(afe_command->afe_calibration_command.calibration_method == kVoltageCal)
		{
			gChnConfigArray_call.gain = kNafeChnGain_1x;
			gChnConfigArray_call.gainCal = kNafeChnGain_1x_cal;
			gChnConfigArray_call.dataRateCode = 4u;
			gChnConfigArray_call.adcSinc = kNafeAdcSinc_sinc4;
			gChnConfigArray_call.chDelayCode = 0x12;
			gChnConfigArray_call.adcSettling = kNafeAdcSettling_singleCycle;
			gChnConfigArray_call.viexVi = kNafeViexVi_voltage;
			gChnConfigArray_call.viexMag = kNafeViexMag_0mV_0uA;
			gChnConfigArray_call.viexAinEn = kNafeViexAinEn_none;
			gChnConfigArray_call.tcc  = kNafeTempCorr_Off;

			gChnConfigArray_call.hvAip = (NAFE_hvInputPos_t)(kNafeHvInputPos_ai1p);
			gChnConfigArray_call.hvAin = (NAFE_hvInputNeg_t)(kNafeHvInputPos_aicom);
		}
		else
		{
			gChnConfigArray_call.gain = kNafeChnGain_16x;
			gChnConfigArray_call.gainCal = kNafeChnGain_16x_cal;
			gChnConfigArray_call.dataRateCode = 0x16u;
			gChnConfigArray_call.adcSinc = kNafeAdcSinc_sinc4_sinc1;
			gChnConfigArray_call.chDelayCode = 11;
			gChnConfigArray_call.adcSettling = kNafeAdcSettling_normal;
			gChnConfigArray_call.viexVi = kNafeViexVi_current;
			gChnConfigArray_call.viexMag = kNafeViexMag_4p5V_750uA;
			gChnConfigArray_call.viexAinEn = kNafeViexAinEn_ai2n;
			gChnConfigArray_call.tcc  = kNafeTempCorr_Off;

			gChnConfigArray_call.hvAip = (NAFE_hvInputPos_t)(kNafeHvInputPos_ai4p);
			gChnConfigArray_call.hvAin = (NAFE_hvInputNeg_t)(kNafeHvInputNeg_ai4n);
		}

		nafeHdl->contSampleAmt = AFE_CALL_SAMPLES;
	}

	nafeHdl->chnAmt = channels_used;

	if(NAFE_init(nafeHdl) != 0)
	{
		return kStatus_RIOP_Err;
	}
	return kStatus_RIOP_Ok;
}


riop_status_t AFE_StartPeriodicMeasurement(NAFE_hdl_t *nafeHdl, afe_command_t *afe_command, timer_handle_t *timer_handle, uint32_t period)
{
	if(nafeHdl == NULL || afe_command == NULL || timer_handle == NULL)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

	if(period < AFE_MIN_SAMPLING_PERIOD_US_TSN)
	{
		period = AFE_MIN_SAMPLING_PERIOD_US_TSN;
	}

	if(afe_command->afe_hvsig_command.is_dirty)
	{
		if(AFE_ChannelConfiguration(nafeHdl, afe_command) != kStatus_RIOP_Ok)
		{
			return kStatus_RIOP_Err;
		}

	}
	else if(afe_command->afe_current_command.is_dirty)
	{
		if(AFE_ChannelConfiguration(nafeHdl, afe_command) != kStatus_RIOP_Ok)
		{
			return kStatus_RIOP_Err;
		}
	}


	if (TIMER_Start(period) != kStatus_RIOP_Ok)
	{
		return kStatus_RIOP_Err;
	}

	return kStatus_RIOP_Ok;

}

void AFE_StopPeriodicMeasurement(void)
{
	TIMER_Stop();
}

riop_status_t AFE_StartSingleMeasurement(NAFE_hdl_t *nafeHdl, afe_command_t *afe_command)
{
	if(nafeHdl == NULL || afe_command == NULL)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

	if(AFE_ChannelConfiguration(nafeHdl, afe_command) != kStatus_RIOP_Ok)
	{
		return kStatus_RIOP_Err;
	}

	nafeHdl->sampleInProgress = 1;
	NAFE_startSample(nafeHdl);

	return kStatus_RIOP_Ok;
}


riop_status_t AFE_installPeriodicMeasurementCallback(timer_handle_t *timer_handle, timer_callback_t callback, void *userData)
{
	if(timer_handle == NULL || userData == NULL)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

	timer_handle->callback = callback;
	timer_handle->userData = userData;

	s_timerHandle = timer_handle;
	s_gptIsr = TIMER_HandleIRQ;

	return kStatus_RIOP_Ok;
}

riop_status_t AFE_WaitForMeasurementComplete(TaskHandle_t *afe_task_handle, uint8_t timeout)
{
	if(afe_task_handle == NULL)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

	xTaskNotifyStateClear(*afe_task_handle);
	xTaskNotifyWait(0, 1, NULL, pdMS_TO_TICKS(timeout));

	return kStatus_RIOP_Ok;
}

riop_status_t AFE_AdcCode2VoltageInternal(NAFE_hdl_t *nafeHdl, uint32_t input_data, float *result, uint8_t channelNum)
{
	if(nafeHdl == NULL || result == NULL || channelNum > 4)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

    float adcCode2VoltageFactor = (10.0 / 2.5 / 0x1000000); //Voltage factor for lv signals and 24bit value

    float f32Result;
    float VG;

    int32_t s32Tmp;
    uint32_t u32Tmp;

    /*Get signed value from unsigned*/
    u32Tmp = input_data;
    u32Tmp <<= 16u;

    /*Shift back to get 16bit value*/
    s32Tmp = (int32_t)u32Tmp;
    s32Tmp >>= 8u;

    /*Calculate voltage gain*/
    VG = s32Tmp * adcCode2VoltageFactor;

    /*Calculate real voltage value based on ADC code conversion formulas table from DS*/
    switch(channelNum)
    {
      case 0:
          f32Result = VG;
          break;
      case 1:
          f32Result = 2.0f*(VG+1.5f);
          break;
      case 2:
          f32Result = 2.0f*(VG+1.5f);
          break;
      case 3:
          f32Result = 32.0f*(VG+0.25f);
          break;
      case 4:
          f32Result = -32.0f*(VG-0.25f);
          break;
    }
    *result = f32Result;

    return kStatus_RIOP_Ok;
}

riop_status_t AFE_AdcCode2TemperatureInternal(NAFE_hdl_t *nafeHdl, uint32_t input_data, float *result)
{
	if(nafeHdl == NULL || result == NULL)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

	float adcCode2VoltageFactor = (10.0 / 16 / 0x1000000); //Voltage factor for temp signals and 24bit value

	float VG;
	float temp;

	int32_t s32Tmp;
	uint32_t u32Tmp;

	/*Get signed value from unsigned*/
	u32Tmp = input_data;
	u32Tmp <<= 16u;

	/*Shift back to get 16bit value*/
	s32Tmp = (int32_t)u32Tmp;
	s32Tmp >>= 8u;

	/*Calculate voltage gain*/
	VG = (s32Tmp * adcCode2VoltageFactor);

	temp = ((VG/(750.0/1000000))-100)/(((3.908)/1000)*100);

	*result = temp;

	return kStatus_RIOP_Ok;
}


riop_status_t AFE_CalculateMean(NAFE_hdl_t *nafeHdl, uint8_t * channels_configured, float * meanV, uint8_t voltage)
{
	if(nafeHdl == NULL || channels_configured == NULL || meanV == NULL)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

	uint16_t *pDataArray = nafeHdl->data;
	uint64_t mean = 0;
	uint8_t channel_offset = 0;


	/*Calculate mean from measured values*/
	for(uint8_t i = 0; i < nafeHdl->chnAmt; i++)
	{

		while(channels_configured[channel_offset] == 0)
		{
			channel_offset++;
		}
		for(uint16_t j = 0; j < (nafeHdl->contSampleAmt); j++)
		{
		  mean += *pDataArray++;
		}
		mean /= nafeHdl->contSampleAmt;

		if(voltage == 1)
		{
			AFE_AdcCode2VoltageInternal(nafeHdl, (uint32_t)mean, &meanV[channel_offset], channel_offset);
		}
		else
		{
			AFE_AdcCode2TemperatureInternal(nafeHdl, (uint32_t)mean, &meanV[0]);
		}
		mean = 0;

		channel_offset++;
	}

	return kStatus_RIOP_Ok;
}

riop_status_t AFE_CalculateStdev(NAFE_hdl_t *nafeHdl, uint8_t * channels_configured, float * meanV, float *stdevV)
{
	if(nafeHdl == NULL || channels_configured == NULL || meanV == NULL || stdevV == NULL)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

	uint8_t channel_offset = 0;
	uint16_t *pDataArray = nafeHdl->data;

	float sum = 0;
	float temp1 = 0;
	float delta = 0;

	/*Calculate standard deviation*/
	for(uint16_t i = 0; i < nafeHdl->chnAmt; i++)
	{
		while(channels_configured[channel_offset] == 0)
		{
			channel_offset++;
		}

		for(uint16_t j = 0; j < (nafeHdl->contSampleAmt); j++)
		{
		   AFE_AdcCode2VoltageInternal(nafeHdl, *pDataArray, &temp1, channel_offset);
		   delta = temp1 - meanV[channel_offset];
		   delta *= delta;
		   delta /= (nafeHdl->contSampleAmt);
		   sum = sum + delta;
		   pDataArray++;
		}
		stdevV[channel_offset] = sqrt(sum);

		sum = 0;
		delta = 0;
		channel_offset++;
	}

	return kStatus_RIOP_Ok;
}

riop_status_t AFE_Calibration(NAFE_hdl_t *nafeHdl, afe_command_t *afe_command)
{
	if(nafeHdl == NULL || afe_command == NULL)
	{
		return kStatus_RIOP_ErrArgInvalid;
	}

	if((afe_command->afe_calibration_command.calibration_start == 1) && (gAfeCal.phase == kCalStart))
	{
		AFE_ChannelConfiguration(nafeHdl, afe_command);

		memset(&gAfeCal, 0, sizeof(afe_calibration_t));
		memset(&gAfeCalStatus,0, sizeof(afe_calibration_status_t));

		/*SAve current value of the gain and offset*/
		NAFE_readRegBlock(nafeHdl, (nafeHdl->chConfig->gainCal + NAFE_REG_GAIN_COEF0), &gAfeCalStatus.original_gain, 24u);
		NAFE_readRegBlock(nafeHdl, (nafeHdl->chConfig->gainCal + NAFE_REG_OFFSET_COEF0), &gAfeCalStatus.original_offset, 24u);

		NAFE_writeRegBlock(&gNafe_Hdl, (nafeHdl->chConfig->gainCal + NAFE_REG_GAIN_COEF0), 0x400000, 24u);
		NAFE_writeRegBlock(&gNafe_Hdl, (nafeHdl->chConfig->gainCal + NAFE_REG_OFFSET_COEF0), 0, 24u);

		/*Move in state machine to next step*/
		gAfeCal.phase = kCalMeasurementPositive;

		/*Send TSN controller start phase is complete*/
		RIOP_SetCalPhase((uint8_t)gAfeCal.phase);
	}
	else if (gAfeCal.phase == kCalMeasurementPositive)
	{
		gAfeCal.inserted_sample[0] = afe_command->afe_calibration_command.inserted_sample_1;
		AFE_StartSingleMeasurement(nafeHdl, afe_command);
		nafeHdl->sampleInProgress = 0;

		for(uint8_t i = 0; i < AFE_CALL_SAMPLES; i++)
		{
			gAfeCal.measured_sample[0] += gDataContinousCall[i];
		}

		gAfeCal.measured_sample[0] /= AFE_CALL_SAMPLES;

		gAfeCal.phase = kCalMeasurementNegative;
		RIOP_SetCalPhase((uint8_t)gAfeCal.phase);
	}
	else if(gAfeCal.phase == kCalMeasurementNegative)
	{
		double GA = 0;
		double GC = 0;
		double OC = 0;
		double LSB = 0;

		gAfeCal.inserted_sample[1] = afe_command->afe_calibration_command.inserted_sample_2;
		AFE_StartSingleMeasurement(nafeHdl, afe_command);
		nafeHdl->sampleInProgress = 0;

		for(uint8_t i = 0; i < AFE_CALL_SAMPLES; i++)
		{
			gAfeCal.measured_sample[1] += gDataContinousCall[i];
		}

		gAfeCal.measured_sample[1] /= AFE_CALL_SAMPLES;

		GA = (gAfeCal.measured_sample[0]-gAfeCal.measured_sample[1])/(gAfeCal.inserted_sample[0]-gAfeCal.inserted_sample[1]);
		GC = 1/GA;
		OC = (gAfeCal.measured_sample[1]*GC) - gAfeCal.inserted_sample[1];

		gAfeCalStatus.calculated_gain = GC/(1.0/4194304);
		LSB = (50.0/1)/16777216;
		gAfeCalStatus.calculated_offset = OC/LSB;

		gAfeCalStatus.calculated_offset &= 0x00FFFFFF;

		NAFE_writeRegBlock(nafeHdl, nafeHdl->chConfig->gainCal + NAFE_REG_GAIN_COEF0, gAfeCalStatus.calculated_gain, 24u);
		NAFE_writeRegBlock(nafeHdl, nafeHdl->chConfig->gainCal + NAFE_REG_OFFSET_COEF0, gAfeCalStatus.calculated_offset, 24u);

		gAfeCal.phase = kCalStart;

		RIOP_SetCalibrationStatus(&gAfeCalStatus);
		RIOP_SetCalPhase((uint8_t)gAfeCal.phase);
	}

	return kStatus_RIOP_Ok;
}


static void TIMER_HandleIRQ(void *irqHandle)
{
	timer_handle_t *handle = (timer_handle_t *)irqHandle;

	if(handle->callback != NULL)
	{
		handle->callback(handle->userData);
	}
}

static riop_status_t TIMER_Init(void)
{
	gpt_config_t gptConfig;
	GPT_GetDefaultConfig(&gptConfig);
	GPT_Init(GPT2, &gptConfig);
	GPT_SetClockDivider(GPT2, 3);

	return kStatus_RIOP_Ok;
}

static riop_status_t TIMER_Start(uint32_t interval_us)
{
	uint32_t gptFreq = 0;

	if(interval_us < 1000)
	{
		interval_us = 1000;
	}

	gptFreq = CLOCK_GetRootClockFreq(kCLOCK_Root_Gpt2)/3000000;
	gptFreq *= interval_us;

	/* Set both GPT modules to 1 second duration */
	GPT_SetOutputCompareValue(GPT2, kGPT_OutputCompare_Channel1, gptFreq);

	/* Enable GPT Output Compare1 interrupt */
	GPT_EnableInterrupts(GPT2, kGPT_OutputCompare1InterruptEnable);

	/* Enable at the Interrupt */
	EnableIRQ(GPT_IRQ_ID);
	IRQ_SetPriority(GPT_IRQ_ID, 5);

	GPT_StartTimer(GPT2);

	return kStatus_RIOP_Ok;
}

static void TIMER_Stop(void)
{
	GPT_StopTimer(GPT2);
}

void TIMER_ISR(void)
{
	GPT_ClearStatusFlags(GPT2, kGPT_OutputCompare1Flag);

	s_gptIsr(s_timerHandle);

	SDK_ISR_EXIT_BARRIER;
}

void BOARD_NAFE_SPI_ISR(void)
{
    NAFE_irqHandle(&gNafe_Hdl, kNafeInterrupt_spi);
    SDK_ISR_EXIT_BARRIER;
}

void BOARD_NAFE_DRDY_ISR(void)
{
    NAFE_irqHandle(&gNafe_Hdl, kNafeInterrupt_drdyPin);
    SDK_ISR_EXIT_BARRIER;
}

