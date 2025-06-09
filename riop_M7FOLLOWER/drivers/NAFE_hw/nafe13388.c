/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "nafe13388.h"
#include "nafe_hal.h"

/*******************************************************************************
 * Static Function Prototypes
 ******************************************************************************/
static void NAFE_resetDevice(NAFE_hdl_t *nafeHdl);
static void NAFE_combCmdForAccessRegs(uint16_t devAddr, uint16_t regAddr, NAFE_regAccessType_t accessType, uint16_t *combCmd);
static void NAFE_combCmdForInstructionCmds(uint16_t devAddr, uint16_t regAddr, uint16_t *combCmd);
static inline void NAFE_readRegNonBlock(NAFE_hdl_t *nafeHdl, uint16_t regAddr, uint32_t regSize);
static inline void NAFE_sendCmdNonBlock(NAFE_hdl_t *nafeHdl, uint32_t cmd);
static inline void NAFE_sendCmdBlock(NAFE_hdl_t *nafeHdl, uint32_t cmd, uint8_t csEndLevel);
static void NAFE_triggerSample(NAFE_hdl_t *nafeHdl);
static void NAFE_adcCode2Voltage(NAFE_hdl_t *nafeHdl);
static void NAFE_scsrBlock(NAFE_hdl_t *nafeHdl);
static void NAFE_sccrBlock(NAFE_hdl_t *nafeHdl);
static void NAFE_mcmrBlock(NAFE_hdl_t *nafeHdl);
static void NAFE_mccrBlock(NAFE_hdl_t *nafeHdl);
AT_QUICKACCESS_SECTION_CODE(static void NAFE_readResult(NAFE_hdl_t *nafeHdl));

/*******************************************************************************
 * Static Variables.
 ******************************************************************************/
/* Initialization gains based on PGA setting for back calculation the results to volt units */
static const float g_adcCode2VoltageFactor[8] = {
    10.0 / 0.2 / 0x1000000,  /* input_range / PGA_gain_0.2  / 2^24) */
    10.0 / 0.4 / 0x1000000,  /* input_range / PGA_gain_0.4  / 2^24) */
    10.0 / 0.8 / 0x1000000,  /* input_range / PGA_gain_0.8  / 2^24) */
    10.0 / 1.0 / 0x1000000,  /* input_range / PGA_gain_1.0  / 2^24) */
    10.0 / 2.0 / 0x1000000,  /* input_range / PGA_gain_2.0  / 2^24) */
    10.0 / 4.0 / 0x1000000,  /* input_range / PGA_gain_4.0  / 2^24) */
    10.0 / 8.0 / 0x1000000,  /* input_range / PGA_gain_8.0  / 2^24) */
    10.0 / 16.0 / 0x1000000, /* input_range / PGA_gain_16.0 / 2^24) */
};

/*******************************************************************************
 * Functions
 ******************************************************************************/
/* Initialize the device. */
int32_t NAFE_init(NAFE_hdl_t *nafeHdl)
{
    uint32_t u32Tmp = 0;

    assert(nafeHdl->sysConfig != NULL);
    assert(nafeHdl->halHdl != NULL);

    if (nafeHdl->sysConfig->adcResolutionCode == kNafeAdcResolution_24bits)
    {
        nafeHdl->adcResolutionBits = 24u;
    }
    else
    {
        nafeHdl->adcResolutionBits = 16u;
    }
    switch (nafeHdl->sampleMode)
    {
    case kNafeSampleMode_scsrBlock:
    case kNafeSampleMode_sccrBlock:
    case kNafeSampleMode_mcmrBlock:
    case kNafeSampleMode_mccrBlock:
        nafeHdl->halHdl->readRxdMethod = kNafeHalReadRxdMethod_polling;
        break;
    case kNafeSampleMode_scsrNonBlock:
    case kNafeSampleMode_sccrNonBlock:
    case kNafeSampleMode_mcmrNonBlock:
    case kNafeSampleMode_mccrNonBlock:
        nafeHdl->halHdl->readRxdMethod = kNafeHalReadRxdMethod_interrupt;
        break;
    case kNafeSampleMode_sccrDma:
    case kNafeSampleMode_mccrDma:
        nafeHdl->halHdl->readRxdMethod = kNafeHalReadRxdMethod_dma;
        nafeHdl->halHdl->dmaData = nafeHdl->data;
        nafeHdl->halHdl->dmaChnAmt = nafeHdl->chnAmt;
        nafeHdl->halHdl->dmaContSampleAmt = nafeHdl->contSampleAmt;
        break;
    default:
        nafeHdl->halHdl->readRxdMethod = kNafeHalReadRxdMethod_none;
        break;
    }
    nafeHdl->halHdl->adcResolutionBits = nafeHdl->adcResolutionBits;
    NAFE_HAL_init(nafeHdl->halHdl);

    /* Reset the device. */
    NAFE_resetDevice(nafeHdl);
    NAFE_HAL_DelayAtLeastUs(100u * 1000u);

    /* Configure channel registers. */
    for (int32_t i = 0; i < nafeHdl->chnAmt; i++)
    {
        NAFE_sendCmdBlock(nafeHdl, NAFE_CMD_CH0 + nafeHdl->chConfig[i].chnIndex, 1u);
        NAFE_writeRegBlock(nafeHdl, NAFE_REG_CH_CONFIG0,
                           (nafeHdl->chConfig[i].hvAip << 12u) | (nafeHdl->chConfig[i].hvAin << 8u)
                         | (nafeHdl->chConfig[i].gain << 5u) | (nafeHdl->chConfig[i].inputSel << 4u)
                         | (nafeHdl->chConfig[i].lv << 1u) | (nafeHdl->chConfig[i].tcc),
                           16u);
        NAFE_writeRegBlock(nafeHdl, NAFE_REG_CH_CONFIG1,
        				   nafeHdl->chConfig[i].gainCal << 12u | nafeHdl->chConfig[i].dataRateCode << 3u
                         | nafeHdl->chConfig[i].adcSinc,
                           16u);
        NAFE_writeRegBlock(nafeHdl, NAFE_REG_CH_CONFIG2,
                           nafeHdl->chConfig[i].chDelayCode << 10u | nafeHdl->chConfig[i].adcSettling << 9u
                         | nafeHdl->chConfig[i].filterReset << 8u | nafeHdl->chConfig[i].chChop << 7u,
                           16u);
        NAFE_writeRegBlock(nafeHdl, NAFE_REG_CH_CONFIG3,
                           nafeHdl->chConfig[i].viexVi << 15u | nafeHdl->chConfig[i].viexPol << 14u
                         | nafeHdl->chConfig[i].viexMag << 10u | nafeHdl->chConfig[i].viexChop << 6u
                         | nafeHdl->chConfig[i].viexAipEn << 3u | nafeHdl->chConfig[i].viexAinEn,
                           16u);
    }

    /* Configure system registers. */
    uint32_t enabledChnMask = 0;
    for (uint32_t i = 0; i < nafeHdl->chnAmt; i++)
    {
        enabledChnMask |= 1u << nafeHdl->chConfig[i].chnIndex;
    }

    NAFE_readRegBlock(nafeHdl, NAFE_REG_SYS_CONFIG0, &u32Tmp, 16u);
    u32Tmp &= ~(1u << 4u);
    u32Tmp |= nafeHdl->sysConfig->drdyPinSeqMode << 4u;
    u32Tmp &= ~(1u << 5u);
    u32Tmp |= nafeHdl->sysConfig->triggerMode << 5u;
    u32Tmp &= ~(1u << 14u);
    u32Tmp |= nafeHdl->sysConfig->adcResolutionCode << 14;
    NAFE_writeRegBlock(nafeHdl, NAFE_REG_SYS_CONFIG0, u32Tmp, 16u);
    NAFE_HAL_DelayAtLeastUs(100u);
    NAFE_writeRegBlock(nafeHdl, NAFE_REG_CH_CONFIG4, enabledChnMask, 16u);

    if (nafeHdl->gpioConfig != NULL)
    {
        NAFE_gpioConfig(nafeHdl);
    }

    NAFE_HAL_DelayAtLeastUs(100u);

    return 0;
}

void NAFE_GetDefaultConfig(NAFE_hdl_t *nafeHdl)
{
    if (nafeHdl->sysConfig != NULL)
    {
        nafeHdl->sysConfig->adcResolutionCode = kNafeAdcResolution_24bits;
        nafeHdl->sysConfig->triggerMode = kNafeTrigger_syncPin;
        nafeHdl->sysConfig->drdyPinSeqMode = kNafeDrdyPinSeqMode_onSequencer;
    }

    if (nafeHdl->chConfig != NULL)
    {
        for (uint32_t i = 0; i < nafeHdl->chnAmt; i++)
        {
            nafeHdl->chConfig[i].chnIndex = i;
            nafeHdl->chConfig[i].inputSel = kNafeInputSel_hvsig;
            nafeHdl->chConfig[i].hvAip = (NAFE_hvInputPos_t)(kNafeHvInputPos_ai1p + i);
            nafeHdl->chConfig[i].hvAin = (NAFE_hvInputNeg_t)(kNafeHvInputNeg_ai1n + i);
            nafeHdl->chConfig[i].gain = kNafeChnGain_1x;
            nafeHdl->chConfig[i].dataRateCode = 4u;
            nafeHdl->chConfig[i].adcSinc = kNafeAdcSinc_sinc4;
            nafeHdl->chConfig[i].chDelayCode = 0u;
            nafeHdl->chConfig[i].adcSettling = kNafeAdcSettling_singleCycle;
            nafeHdl->chConfig[i].viexVi = kNafeViexVi_voltage;
            nafeHdl->chConfig[i].viexPol = kNafeViexPol_positive;
            nafeHdl->chConfig[i].viexMag = kNafeViexMag_0mV_0uA;
            nafeHdl->chConfig[i].viexChop = kNafeViexChop_disable;
            nafeHdl->chConfig[i].viexAipEn = kNafeViexAipEn_none;
            nafeHdl->chConfig[i].viexAinEn = kNafeViexAinEn_none;
        }
    }

    if (nafeHdl->gpioConfig != NULL)
    {
        nafeHdl->gpioConfig->connectToPin = 0u;
        nafeHdl->gpioConfig->readEnable = 0u;
        nafeHdl->gpioConfig->writeEnable = 0u;
    }

    nafeHdl->devAddr = 0u;
    nafeHdl->readResultFrameMode = kNafeReadResult_singleFrame;
    nafeHdl->devSampleMode = kNafeSampleMode_none;
}

void NAFE_startSample(NAFE_hdl_t *nafeHdl)
{
    uint32_t spiInterrupts = 0u;
    uint32_t rxFifoTrigLevel = 1u;
    bool drdyInterrupt = false;

    NAFE_HAL_spiClearInterruptFlags(nafeHdl->halHdl);
    NAFE_HAL_drdyClearInterruptFlag(nafeHdl->halHdl);

    nafeHdl->contSampleCnt = 0u;                                     /* For continuous sample modes. */
    nafeHdl->chnSampleCnt = 0u;                                      /* For multi-channel sample modes. */
    nafeHdl->nonBlockSampleState = kNafeNonBlockSampleState_initial; /* For non-block sample modes. */
    nafeHdl->halHdl->dmaContSampleCnt = 0;                           /* For continuous DMA sample modes. */

    switch (nafeHdl->sampleMode)
    {
    case kNafeSampleMode_scsrBlock:
    case kNafeSampleMode_sccrBlock:
        assert(nafeHdl->chnAmt == 1u);
        break;
    case kNafeSampleMode_mcmrBlock:
    case kNafeSampleMode_mccrBlock:
        assert(nafeHdl->chnAmt <= NAFE_CHN_AMT_MAX);
        break;
    case kNafeSampleMode_scsrNonBlock:
    case kNafeSampleMode_sccrNonBlock:
        assert(nafeHdl->chnAmt == 1u);
        spiInterrupts = kNafeHalInterruptType_frameComplete;
        drdyInterrupt = true;
        break;
    case kNafeSampleMode_mcmrNonBlock:
    case kNafeSampleMode_mccrNonBlock:
        assert(nafeHdl->chnAmt <= NAFE_CHN_AMT_MAX);
        spiInterrupts = kNafeHalInterruptType_frameComplete;
        drdyInterrupt = true;
        break;
    case kNafeSampleMode_sccrDma:
        assert(nafeHdl->chnAmt == 1u);
        NAFE_HAL_prepareDmaXfer(nafeHdl->halHdl);
        break;
    case kNafeSampleMode_mccrDma:
        assert(nafeHdl->chnAmt <= NAFE_CHN_AMT_MAX);
        NAFE_HAL_prepareDmaXfer(nafeHdl->halHdl);
        rxFifoTrigLevel = nafeHdl->chnAmt;
        break;
    default:
        break;
    }

    NAFE_HAL_configSpiRxFifoTrigLevel(nafeHdl->halHdl, rxFifoTrigLevel);
    NAFE_HAL_configSpiInterrupt(nafeHdl->halHdl, spiInterrupts);
    if (drdyInterrupt)
    {
        NAFE_HAL_drdyEnableInterrupt(nafeHdl->halHdl);
    }
    else
    {
        NAFE_HAL_drdyDisableInterrupt(nafeHdl->halHdl);
    }

    switch (nafeHdl->sampleMode)
    {
    case kNafeSampleMode_scsrBlock:
        NAFE_scsrBlock(nafeHdl);
        break;
    case kNafeSampleMode_sccrBlock:
        NAFE_sccrBlock(nafeHdl);
        break;
    case kNafeSampleMode_mcmrBlock:
        NAFE_mcmrBlock(nafeHdl);
        break;
    case kNafeSampleMode_mccrBlock:
        NAFE_mccrBlock(nafeHdl);
        break;
    case kNafeSampleMode_scsrNonBlock:
    case kNafeSampleMode_sccrNonBlock:
    case kNafeSampleMode_mcmrNonBlock:
    case kNafeSampleMode_mccrNonBlock:
    case kNafeSampleMode_sccrDma:
    case kNafeSampleMode_mccrDma:
        NAFE_triggerSample(nafeHdl);
        break;
    default:
        break;
    }
}

void NAFE_terminateSample(NAFE_hdl_t *nafeHdl)
{
    switch (nafeHdl->sampleMode)
    {
    case kNafeSampleMode_scsrBlock:
    case kNafeSampleMode_mcmrBlock:
    case kNafeSampleMode_scsrNonBlock:
    case kNafeSampleMode_mcmrNonBlock:
        NAFE_HAL_spiCsWrite(nafeHdl->halHdl, 1u);
        break;
    case kNafeSampleMode_sccrBlock:
    case kNafeSampleMode_mccrBlock:
    case kNafeSampleMode_sccrNonBlock:
    case kNafeSampleMode_mccrNonBlock:
    case kNafeSampleMode_sccrDma:
    case kNafeSampleMode_mccrDma:
        NAFE_sendCmdBlock(nafeHdl, NAFE_CMD_ABORT, 1u);
        NAFE_HAL_DelayAtLeastUs(2u);
        break;
    default:
        break;
    }

    NAFE_HAL_drdyClearInterruptFlag(nafeHdl->halHdl);

    nafeHdl->devSampleMode = kNafeSampleMode_none;
}

static void NAFE_resetDevice(NAFE_hdl_t *nafeHdl)
{
    NAFE_HAL_SendPinRst(nafeHdl->halHdl);
    // NAFE_sendCmdBlock(nafeHdl, NAFE_CMD_RESET, 1u);
}

/* Generate combined command for accessing registers, including device address, W/R access type,
   and register address.*/
static void NAFE_combCmdForAccessRegs(uint16_t devAddr, uint16_t regAddr, NAFE_regAccessType_t accessType, uint16_t *combCmd)
{
    assert(devAddr == 0u || devAddr == 1u);

    *combCmd = devAddr;
    *combCmd <<= 15u;              /*bit15: hardware address */
    *combCmd |= accessType << 14u; /*bit14: W = 0, R = 1 */
    *combCmd |= regAddr << 1u;     /*bit1~bit13: register address */
}

/* Generate combined command for instruction commands, including device address and instruction commands.*/
static void NAFE_combCmdForInstructionCmds(uint16_t devAddr, uint16_t cmd, uint16_t *combCmd)
{
    assert(devAddr == 0u || devAddr == 1u);

    *combCmd = devAddr;
    *combCmd <<= 15u;      /*bit15: hardware address */
    *combCmd |= cmd << 1u; /*bit1~bit14: instruction command */
}

void NAFE_writeRegBlock(NAFE_hdl_t *nafeHdl, uint16_t regAddr, uint32_t data, uint32_t regSize)
{
    uint16_t combCmd = 0;

    NAFE_combCmdForAccessRegs(nafeHdl->devAddr, regAddr, kNafeRegAccess_write, &combCmd);
    NAFE_HAL_writeRegBlock(nafeHdl->halHdl, combCmd, data, regSize);
}

static inline void NAFE_readRegNonBlock(NAFE_hdl_t *nafeHdl, uint16_t regAddr, uint32_t regSize)
{
    uint16_t combCmd = 0;

    NAFE_combCmdForAccessRegs(nafeHdl->devAddr, regAddr, kNafeRegAccess_read, &combCmd);
    NAFE_HAL_readRegNonBlock(nafeHdl->halHdl, combCmd, regSize);
}

void NAFE_readRegBlock(NAFE_hdl_t *nafeHdl, uint16_t regAddr, uint32_t *data, uint32_t regSize)
{
    uint16_t combCmd = 0;

    NAFE_combCmdForAccessRegs(nafeHdl->devAddr, regAddr, kNafeRegAccess_read, &combCmd);
    NAFE_HAL_readRegBlock(nafeHdl->halHdl, combCmd, data, regSize);
}

static inline void NAFE_sendCmdNonBlock(NAFE_hdl_t *nafeHdl, uint32_t cmd)
{
    uint16_t combCmd = 0;

    NAFE_combCmdForInstructionCmds(nafeHdl->devAddr, cmd, &combCmd);
    NAFE_HAL_sendCmdNonBlock(nafeHdl->halHdl, combCmd);
}

static inline void NAFE_sendCmdBlock(NAFE_hdl_t *nafeHdl, uint32_t cmd, uint8_t csEndLevel)
{
    uint16_t combCmd = 0;

    NAFE_combCmdForInstructionCmds(nafeHdl->devAddr, cmd, &combCmd);
    NAFE_HAL_sendCmdBlock(nafeHdl->halHdl, combCmd, csEndLevel);
}

/* Trigger to start the sample and conversion. */
static void NAFE_triggerSample(NAFE_hdl_t *nafeHdl)
{
    uint16_t cmd = 0;

    switch (nafeHdl->sampleMode)
    {
    case kNafeSampleMode_scsrBlock:
    case kNafeSampleMode_sccrBlock:
    case kNafeSampleMode_mcmrBlock:
    case kNafeSampleMode_mccrBlock:
    case kNafeSampleMode_sccrDma:
    case kNafeSampleMode_mccrDma:
        if (nafeHdl->devSampleMode != nafeHdl->sampleMode || nafeHdl->sysConfig->triggerMode != kNafeTrigger_syncPin)
        {
            if (nafeHdl->sampleMode == kNafeSampleMode_sccrDma)
            {
                cmd = NAFE_CMD_SC;
            }
            else if (nafeHdl->sampleMode == kNafeSampleMode_mccrDma)
            {
                cmd = NAFE_CMD_MC;
            }
            else
            {
                cmd = nafeHdl->sampleMode - kNafeSampleMode_scsrBlock + NAFE_CMD_SS;
            }

            /* Send the command. */
            if (nafeHdl->readResultFrameMode == kNafeReadResult_multiFrame)
            {
                NAFE_sendCmdBlock(nafeHdl, cmd, 1u);
            }
            else
            {
                if (nafeHdl->sysConfig->triggerMode == kNafeTrigger_syncPin)
                {
                    /* Wait cmd frame to be end, then send the sync signal.*/
                    NAFE_sendCmdBlock(nafeHdl, cmd, 0u);
                }
                else
                {
                    /* No need to wait cmd frame to be end. */
                    NAFE_sendCmdNonBlock(nafeHdl, cmd);
                }
            }

            nafeHdl->devSampleMode = nafeHdl->sampleMode;
        }
        /* Trigger the sample with SYNC signal. */
        if (nafeHdl->sysConfig->triggerMode == kNafeTrigger_syncPin)
        {
            NAFE_HAL_sendSyncPulse(nafeHdl->halHdl);
        }
        break;
    case kNafeSampleMode_scsrNonBlock:
    case kNafeSampleMode_sccrNonBlock:
    case kNafeSampleMode_mcmrNonBlock:
    case kNafeSampleMode_mccrNonBlock:
        if (nafeHdl->devSampleMode != nafeHdl->sampleMode || nafeHdl->sysConfig->triggerMode == kNafeTrigger_spiCmd)
        {
            cmd = nafeHdl->sampleMode - kNafeSampleMode_scsrNonBlock + NAFE_CMD_SS;
            NAFE_sendCmdNonBlock(nafeHdl, cmd);
            nafeHdl->devSampleMode = nafeHdl->sampleMode;
            nafeHdl->nonBlockSampleState = kNafeNonBlockSampleState_modeCmdSent;
        }
        else
        {
            NAFE_HAL_sendSyncPulse(nafeHdl->halHdl);
            nafeHdl->nonBlockSampleState = kNafeNonBlockSampleState_waitForReady;
        }
        break;
    default:
        break;
    }
}

/* Convert result array from 24-bit signed int type to float type. */
static void NAFE_adcCode2Voltage(NAFE_hdl_t *nafeHdl)
{
    void *data = nafeHdl->data;
    uint32_t dataSizeInBits = nafeHdl->adcResolutionBits;
    uint32_t len = 0;
    float adcCode2VoltageFactor = 0.0;

    volatile uint32_t *u32Result;
    volatile uint16_t *u16Result;
    volatile float *f32Result;
    int32_t s32Tmp = 0;
    uint32_t u32Tmp = 0;

    for (int32_t i = nafeHdl->chnAmt - 1; i >= 0; i--)
    {
        len = nafeHdl->contSampleAmt;
        f32Result = (float *)data + len * (i + 1) - 1u;
        adcCode2VoltageFactor = g_adcCode2VoltageFactor[nafeHdl->chConfig[i].gain];

        if (dataSizeInBits == 24u)
        {
            u32Result = (uint32_t *)data + len * (i + 1) - 1u;

            while (len--)
            {
                *u32Result <<= 8u;
                s32Tmp = (int32_t)*u32Result;
                s32Tmp >>= 8u;
                *f32Result = s32Tmp * adcCode2VoltageFactor;
                u32Result--;
                f32Result--;
            }
        }
        else if (dataSizeInBits == 16u)
        {
            u16Result = (uint16_t *)data + len * (i + 1) - 1u;

            while (len--)
            {
                u32Tmp = *u16Result;
                u32Tmp <<= 16u;
                s32Tmp = (int32_t)u32Tmp;
                s32Tmp >>= 8u;
                *f32Result = s32Tmp * adcCode2VoltageFactor;
                u16Result--;
                f32Result--;
            }
        }
    }
}

static void NAFE_scsrBlock(NAFE_hdl_t *nafeHdl)
{
    NAFE_triggerSample(nafeHdl);

    /* Wait for the conversion to be done. */
    while (!NAFE_HAL_drdyGetInterruptFlag(nafeHdl->halHdl))
    {
    }
    NAFE_HAL_drdyClearInterruptFlag(nafeHdl->halHdl);

    /* Read the result. */
    if (nafeHdl->readResultFrameMode == kNafeReadResult_singleFrame)
    {
        bool terminateFrame = (nafeHdl->sysConfig->triggerMode == kNafeTrigger_spiCmd) ? true : false;
        NAFE_HAL_receiveResultBlock(nafeHdl->halHdl, nafeHdl->data, (uint32_t)nafeHdl->adcResolutionBits, 1u, terminateFrame);
    }
    else
    {
        NAFE_readRegBlock(nafeHdl, NAFE_REG_CH_DATA0 + nafeHdl->chConfig[0].chnIndex, nafeHdl->data, nafeHdl->adcResolutionBits);
    }

    NAFE_adcCode2Voltage(nafeHdl);

    /* Do not call NAFE_terminateSample() or rest nafeHdl->devSampleMode,
       So that keeping the sample status in SCSR. And enable the sync pin trigger
       next if required.
    */
}

static void NAFE_sccrBlock(NAFE_hdl_t *nafeHdl)
{
    void *data = NULL;
    NAFE_triggerSample(nafeHdl);

    while (nafeHdl->contSampleCnt < nafeHdl->contSampleAmt)
    {
        /* Wait for the conversion to be done. */
        while (!NAFE_HAL_drdyGetInterruptFlag(nafeHdl->halHdl))
        {
        }
        NAFE_HAL_drdyClearInterruptFlag(nafeHdl->halHdl);

        if (nafeHdl->adcResolutionBits == 24u)
        {
            data = (uint32_t *)nafeHdl->data + nafeHdl->contSampleCnt;
        }
        else
        {
            data = (uint16_t *)nafeHdl->data + nafeHdl->contSampleCnt;
        }

        /* Read the result. */
        if (nafeHdl->readResultFrameMode == kNafeReadResult_singleFrame)
        {
            NAFE_HAL_receiveResultBlock(nafeHdl->halHdl, data, (uint32_t)nafeHdl->adcResolutionBits, 1u, false);
        }
        else
        {
            NAFE_readRegBlock(nafeHdl, NAFE_REG_CH_DATA0 + nafeHdl->chConfig[0].chnIndex, data, nafeHdl->adcResolutionBits);
        }

        nafeHdl->contSampleCnt++;

        if (NULL != nafeHdl->blockingCallback)
        {
            nafeHdl->blockingCallback();
        }
    }

    if (nafeHdl->readResultFrameMode == kNafeReadResult_singleFrame)
    {
        NAFE_HAL_spiCsWrite(nafeHdl->halHdl, 1u);
    }
    NAFE_terminateSample(nafeHdl);

    NAFE_adcCode2Voltage(nafeHdl);
}

static void NAFE_mcmrBlock(NAFE_hdl_t *nafeHdl)
{
    bool terminateFrame = false;
    void *data = NULL;

    NAFE_triggerSample(nafeHdl);

    if (nafeHdl->sysConfig->drdyPinSeqMode == kNafeDrdyPinSeqMode_onSequencer)
    {
        /* Wait for all conversions to be done. */
        while (!NAFE_HAL_drdyGetInterruptFlag(nafeHdl->halHdl))
        {
        }
        NAFE_HAL_drdyClearInterruptFlag(nafeHdl->halHdl);
    }
    for (uint32_t i = 0u; i < nafeHdl->chnAmt; i++)
    {
        if (nafeHdl->sysConfig->drdyPinSeqMode == kNafeDrdyPinSeqMode_onConversion)
        {
            /* Wait for each conversion to be done. */
            while (!NAFE_HAL_drdyGetInterruptFlag(nafeHdl->halHdl))
            {
            }
            NAFE_HAL_drdyClearInterruptFlag(nafeHdl->halHdl);
        }

        if (nafeHdl->adcResolutionBits == 24u)
        {
            data = (uint32_t *)nafeHdl->data + i;
        }
        else
        {
            data = (uint16_t *)nafeHdl->data + i;
        }

        /* Read the result. */
        if (nafeHdl->readResultFrameMode == kNafeReadResult_singleFrame)
        {
            if (i == nafeHdl->chnAmt - 1u && nafeHdl->sysConfig->triggerMode == kNafeTrigger_spiCmd)
            {
                terminateFrame = true;
            }
            NAFE_HAL_receiveResultBlock(nafeHdl->halHdl, data, (uint32_t)nafeHdl->adcResolutionBits, 1u, terminateFrame);
        }
        else
        {
            NAFE_readRegBlock(nafeHdl, NAFE_REG_CH_DATA0 + nafeHdl->chConfig[i].chnIndex, data, nafeHdl->adcResolutionBits);
        }
    }

    NAFE_adcCode2Voltage(nafeHdl);

    /* Do not call NAFE_terminateSample() or rest nafeHdl->devSampleMode,
       So that keeping the sample status in MCMR. And enable the sync pin trigger
       next if required (for Rev.B).
    */
}

static void NAFE_mccrBlock(NAFE_hdl_t *nafeHdl)
{
    void *data = NULL;

    NAFE_triggerSample(nafeHdl);

    while (nafeHdl->contSampleCnt < nafeHdl->contSampleAmt)
    {
        if (nafeHdl->sysConfig->drdyPinSeqMode == kNafeDrdyPinSeqMode_onSequencer)
        {
            /* Wait for all conversions to be done. */
            while (!NAFE_HAL_drdyGetInterruptFlag(nafeHdl->halHdl))
            {
            }
            NAFE_HAL_drdyClearInterruptFlag(nafeHdl->halHdl);
        }
        for (uint32_t i = 0u; i < nafeHdl->chnAmt; i++)
        {
            if (nafeHdl->sysConfig->drdyPinSeqMode == kNafeDrdyPinSeqMode_onConversion)
            {
                /* Wait for each conversion to be done. */
                while (!NAFE_HAL_drdyGetInterruptFlag(nafeHdl->halHdl))
                {
                }
                NAFE_HAL_drdyClearInterruptFlag(nafeHdl->halHdl);
            }

            if (nafeHdl->adcResolutionBits == 24u)
            {
                data = (uint32_t *)nafeHdl->data + nafeHdl->contSampleAmt * i + nafeHdl->contSampleCnt;
            }
            else
            {
                data = (uint16_t *)nafeHdl->data + nafeHdl->contSampleAmt * i + nafeHdl->contSampleCnt;
            }

            /* Read the result. */
            if (nafeHdl->readResultFrameMode == kNafeReadResult_singleFrame)
            {
                NAFE_HAL_receiveResultBlock(nafeHdl->halHdl, data, (uint32_t)nafeHdl->adcResolutionBits, 1u, false);
            }
            else
            {
                NAFE_readRegBlock(nafeHdl, NAFE_REG_CH_DATA0 + nafeHdl->chConfig[i].chnIndex, data, nafeHdl->adcResolutionBits);
            }
        }
        nafeHdl->contSampleCnt++;

        if (NULL != nafeHdl->blockingCallback)
        {
            nafeHdl->blockingCallback();
        }
    }

    if (nafeHdl->readResultFrameMode == kNafeReadResult_singleFrame)
    {
        NAFE_HAL_spiCsWrite(nafeHdl->halHdl, 1u);
    }
    NAFE_terminateSample(nafeHdl);

    NAFE_adcCode2Voltage(nafeHdl);
}

/* Send SPI Rx clock pattern or read result registers to read the conversion results. */
AT_QUICKACCESS_SECTION_CODE(static void NAFE_readResult(NAFE_hdl_t *nafeHdl))
{
    if (nafeHdl->readResultFrameMode == kNafeReadResult_singleFrame)
    {
        if ((nafeHdl->sampleMode == kNafeSampleMode_mcmrNonBlock || nafeHdl->sampleMode == kNafeSampleMode_mccrNonBlock)
         && nafeHdl->sysConfig->drdyPinSeqMode == kNafeDrdyPinSeqMode_onSequencer)
        {
            for (uint32_t i = 0u; i < nafeHdl->chnAmt; i++)
            {
                NAFE_HAL_receiveResultNonBlock(nafeHdl->halHdl, (uint32_t)nafeHdl->adcResolutionBits);
            }
        }
        else
        {
            NAFE_HAL_receiveResultNonBlock(nafeHdl->halHdl, (uint32_t)nafeHdl->adcResolutionBits);
        }
    }
    else
    {
        uint32_t chConfigIndex = 0;

        if (nafeHdl->sampleMode == kNafeSampleMode_scsrNonBlock || nafeHdl->sampleMode == kNafeSampleMode_sccrNonBlock)
        {
            chConfigIndex = 0u;
        }
        else
        {
            chConfigIndex = nafeHdl->chnSampleCnt;
        }

        NAFE_readRegNonBlock(nafeHdl, NAFE_REG_CH_DATA0 + nafeHdl->chConfig[chConfigIndex].chnIndex, nafeHdl->adcResolutionBits);
    }
}

/* DRDY pin and SPI RX IRQ handle. Process the non-block (interrupt) approach
   state machine. */
AT_QUICKACCESS_SECTION_CODE(void NAFE_irqHandle(NAFE_hdl_t *nafeHdl, NAFE_interruptSrc_t interrupt))
{
    void *data = NULL;
    uint32_t rxFifoTrigLevel = 0;
    uint32_t spiInterrupts = 0;
    bool readResult = false;
    bool roundSampleEnd = false;
    bool contSampleEnd = false;
    bool timingError = false;

    switch (nafeHdl->nonBlockSampleState)
    {
    case kNafeNonBlockSampleState_modeCmdSent:
        if (interrupt != kNafeInterrupt_spi)
        {
            timingError = true;
            break;
        }
        NAFE_HAL_spiClearInterruptFlags(nafeHdl->halHdl);
        if (nafeHdl->readResultFrameMode == kNafeReadResult_multiFrame)
        {
            NAFE_HAL_spiCsWrite(nafeHdl->halHdl, 1u);
        }
        if (nafeHdl->sysConfig->triggerMode == kNafeTrigger_syncPin)
        {
            NAFE_triggerSample(nafeHdl); /* Output the sync pulse. */
        }
        else
        {
            nafeHdl->nonBlockSampleState = kNafeNonBlockSampleState_waitForReady;
        }
        break;
    case kNafeNonBlockSampleState_waitForReady:
        if (interrupt != kNafeInterrupt_drdyPin)
        {
            timingError = true;
            break;
        }
        NAFE_HAL_drdyClearInterruptFlag(nafeHdl->halHdl);
        nafeHdl->nonBlockSampleState = kNafeNonBlockSampleState_ready;
        /* No "break" statement here. */
    case kNafeNonBlockSampleState_ready:
        /* Set Rx FIFO trigger level, and enable Rx FIFO interrupt,
           and disable frame complete interrupt. */
        spiInterrupts = kNafeHalInterruptType_receiveData;

        if ((nafeHdl->sampleMode == kNafeSampleMode_mcmrNonBlock || nafeHdl->sampleMode == kNafeSampleMode_mccrNonBlock)
         && nafeHdl->readResultFrameMode == kNafeReadResult_singleFrame
         && nafeHdl->sysConfig->drdyPinSeqMode == kNafeDrdyPinSeqMode_onSequencer)
        {
            rxFifoTrigLevel = nafeHdl->chnAmt;
        }
        else
        {
            rxFifoTrigLevel = 1u;
        }
        NAFE_HAL_configSpiRxFifoTrigLevel(nafeHdl->halHdl, rxFifoTrigLevel);
        NAFE_HAL_configSpiInterrupt(nafeHdl->halHdl, spiInterrupts);
        NAFE_readResult(nafeHdl);
        nafeHdl->nonBlockSampleState = kNafeNonBlockSampleState_rxClkSending;
        break;
    case kNafeNonBlockSampleState_rxClkSending:
        if (interrupt != kNafeInterrupt_spi)
        {
            timingError = true;
            break;
        }
        nafeHdl->nonBlockSampleState = kNafeNonBlockSampleState_dataReceived;

        if (nafeHdl->sampleMode == kNafeSampleMode_scsrNonBlock)
        {
            NAFE_HAL_readSpiRxd(nafeHdl->halHdl, nafeHdl->data, (uint32_t)nafeHdl->adcResolutionBits, 1u);
        }
        else if (nafeHdl->sampleMode == kNafeSampleMode_sccrNonBlock)
        {
            if (nafeHdl->adcResolutionBits == 24u)
            {
                data = (uint32_t *)nafeHdl->data + nafeHdl->contSampleCnt;
            }
            else
            {
                data = (uint16_t *)nafeHdl->data + nafeHdl->contSampleCnt;
            }
            NAFE_HAL_readSpiRxd(nafeHdl->halHdl, data, (uint32_t)nafeHdl->adcResolutionBits, 1u);
        }
        else if (nafeHdl->sampleMode == kNafeSampleMode_mcmrNonBlock || nafeHdl->sampleMode == kNafeSampleMode_mccrNonBlock)
        {
            uint32_t dataArrayBaseOffset = 0;
            uint32_t dataArrayContOffset = 0;
            uint32_t u32Result[NAFE_CHN_AMT_MAX];

            if (nafeHdl->sampleMode == kNafeSampleMode_mcmrNonBlock)
            {
                dataArrayBaseOffset = 1u;
                dataArrayContOffset = 0u;
            }
            else /* kNafeSampleMode_mccrNonBlock */
            {
                dataArrayBaseOffset = nafeHdl->contSampleAmt;
                dataArrayContOffset = nafeHdl->contSampleCnt;
            }

            if (nafeHdl->readResultFrameMode == kNafeReadResult_singleFrame && nafeHdl->sysConfig->drdyPinSeqMode == kNafeDrdyPinSeqMode_onSequencer)
            {
                NAFE_HAL_readSpiRxd(nafeHdl->halHdl, u32Result, (uint32_t)nafeHdl->adcResolutionBits, nafeHdl->chnAmt);

                for (uint32_t i = 0u; i < nafeHdl->chnAmt; i++)
                {
                    if (nafeHdl->adcResolutionBits == 24u)
                    {
                        *((uint32_t *)nafeHdl->data + dataArrayBaseOffset * i + dataArrayContOffset) = u32Result[i];
                    }
                    else if (nafeHdl->adcResolutionBits == 16u)
                    {
                        *((uint16_t *)nafeHdl->data + dataArrayBaseOffset * i + dataArrayContOffset) = *((uint16_t *)u32Result + i);
                    }
                }

                roundSampleEnd = true;
            }
            else
            {
                NAFE_HAL_readSpiRxd(nafeHdl->halHdl, u32Result, (uint32_t)nafeHdl->adcResolutionBits, 1);

                if (nafeHdl->adcResolutionBits == 24u)
                {
                    *((uint32_t *)nafeHdl->data + dataArrayBaseOffset * nafeHdl->chnSampleCnt + dataArrayContOffset) = u32Result[0];
                }
                else if (nafeHdl->adcResolutionBits == 16u)
                {
                    *((uint16_t *)nafeHdl->data + dataArrayBaseOffset * nafeHdl->chnSampleCnt + dataArrayContOffset) = *((uint16_t *)u32Result);
                }

                if (nafeHdl->chnSampleCnt < nafeHdl->chnAmt - 1u)
                {
                    nafeHdl->chnSampleCnt++;
                    if (nafeHdl->sysConfig->drdyPinSeqMode == kNafeDrdyPinSeqMode_onConversion)
                    {
                        nafeHdl->nonBlockSampleState = kNafeNonBlockSampleState_waitForReady;
                    }
                    else
                    {
                        nafeHdl->nonBlockSampleState = kNafeNonBlockSampleState_ready;
                        readResult = true;
                    }
                }
                else
                {
                    roundSampleEnd = true;
                    nafeHdl->chnSampleCnt = 0u;
                }
            }
        }

        if (nafeHdl->sampleMode == kNafeSampleMode_sccrNonBlock || (nafeHdl->sampleMode == kNafeSampleMode_mccrNonBlock && roundSampleEnd))
        {
            nafeHdl->contSampleCnt++;
            if (nafeHdl->contSampleCnt < nafeHdl->contSampleAmt)
            {
                nafeHdl->nonBlockSampleState = kNafeNonBlockSampleState_waitForReady;
            }
            else
            {
                contSampleEnd = true;
            }
        }

        /* Pull SPI CS high. */
        if (nafeHdl->readResultFrameMode == kNafeReadResult_multiFrame || (nafeHdl->readResultFrameMode == kNafeReadResult_singleFrame
         && (nafeHdl->sysConfig->triggerMode == kNafeTrigger_spiCmd && (nafeHdl->sampleMode == kNafeSampleMode_scsrNonBlock
         || (nafeHdl->sampleMode == kNafeSampleMode_mcmrNonBlock && roundSampleEnd)))) || contSampleEnd)
        {
            NAFE_HAL_spiCsWrite(nafeHdl->halHdl, 1u);
        }

        if (readResult)
        {
            NAFE_readResult(nafeHdl);
            nafeHdl->nonBlockSampleState = kNafeNonBlockSampleState_rxClkSending;
        }

        if (nafeHdl->nonBlockSampleState == kNafeNonBlockSampleState_dataReceived)
        {
            NAFE_HAL_configSpiInterrupt(nafeHdl->halHdl, 0u);
            NAFE_HAL_drdyDisableInterrupt(nafeHdl->halHdl);

            /* Terminate continuous sample. */
            if (contSampleEnd)
            {
                NAFE_terminateSample(nafeHdl);
            }

            if(!nafeHdl->useRawData)
			{
			  NAFE_adcCode2Voltage(nafeHdl);
			}
            nafeHdl->nonBlockSampleState = kNafeNonBlockSampleState_done;
            NAFE_HAL_configSpiRxFifoTrigLevel(nafeHdl->halHdl, 1);


            if(nafeHdl->callback != NULL)
            {
            	if(nafeHdl->sampleMode == kNafeSampleMode_scsrNonBlock || nafeHdl->sampleMode == kNafeSampleMode_mcmrNonBlock)
            	{
            		nafeHdl->callback(nafeHdl);
            	}
            	else if((nafeHdl->sampleMode == kNafeSampleMode_sccrNonBlock || nafeHdl->sampleMode == kNafeSampleMode_mccrNonBlock) && contSampleEnd)
            	{
            		nafeHdl->callback(nafeHdl);
            	}
            }


        }
        break;
    default:
        break;
    }

    if (timingError)
    {
        NAFE_terminateSample(nafeHdl);
    }
}

/* DMA major loop done IRQ handle. */
void NAFE_dmaIrqHandle(NAFE_hdl_t *nafeHdl)
{
    NAFE_HAL_dmaIrqHandle(nafeHdl->halHdl);

    if (nafeHdl->halHdl->dmaContSampleCnt == nafeHdl->halHdl->dmaContSampleAmt)
    {
        NAFE_HAL_spiCsWrite(nafeHdl->halHdl, 1u);
        NAFE_terminateSample(nafeHdl);
        NAFE_adcCode2Voltage(nafeHdl);
        nafeHdl->nonBlockSampleState = kNafeNonBlockSampleState_done;
    }
}

void NAFE_gpioConfig(NAFE_hdl_t *nafeHdl)
{
    NAFE_writeRegBlock(nafeHdl, NAFE_REG_GPIO_CONFIG0, nafeHdl->gpioConfig->writeEnable << 6u, 16u);
    NAFE_writeRegBlock(nafeHdl, NAFE_REG_GPIO_CONFIG1, nafeHdl->gpioConfig->connectToPin << 6u, 16u);
    NAFE_writeRegBlock(nafeHdl, NAFE_REG_GPIO_CONFIG2, nafeHdl->gpioConfig->readEnable << 6u, 16u);
}

uint16_t NAFE_gpioReadData(NAFE_hdl_t *nafeHdl)
{
    uint32_t gpiData = 0;

    NAFE_readRegBlock(nafeHdl, NAFE_REG_GPI_DATA, &gpiData, 16u);

    return gpiData >> 6u;
}

uint16_t NAFE_gpioReadPosEdge(NAFE_hdl_t *nafeHdl)
{
    uint32_t gpiPosEdge = 0;

    NAFE_readRegBlock(nafeHdl, NAFE_REG_GPI_EDGE_POS, &gpiPosEdge, 16u);

    return gpiPosEdge >> 6u;
}

uint16_t NAFE_gpioReadNegEdge(NAFE_hdl_t *nafeHdl)
{
    uint32_t gpiNegEdge = 0;

    NAFE_readRegBlock(nafeHdl, NAFE_REG_GPI_EDGE_NEG, &gpiNegEdge, 16u);

    return gpiNegEdge >> 6u;
}

void NAFE_gpioWriteData(NAFE_hdl_t *nafeHdl, uint16_t gpoData)
{
    NAFE_writeRegBlock(nafeHdl, NAFE_REG_GPO_DATA, gpoData << 6u, 16u);
}
