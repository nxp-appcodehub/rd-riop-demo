/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_clock.h"
#include "fsl_edma.h"
#include "fsl_flexio.h"
#include "fsl_iomuxc.h"
#include "fsl_lpspi.h"
#include "fsl_memory.h"
#include "edma_permission.h"
#include "nafe_hal.h"

/*******************************************************************************
 * Static Function Prototypes
 ******************************************************************************/
static void NAFE_HAL_initSpi(NAFE_HAL_hdl_t *halHdl);
static void NAFE_HAL_initFlexIO(NAFE_HAL_hdl_t *halHdl);
static void NAFE_HAL_initDma(NAFE_HAL_hdl_t *halHdl);

/*******************************************************************************
 * Functions
 ******************************************************************************/
void NAFE_HAL_init(NAFE_HAL_hdl_t *halHdl)
{
    uint32_t mask = 0;

    NAFE_HAL_initSpi(halHdl);

    if (halHdl->drdyGpio != 0)
    {
        NAFE_HAL_drdyDisableInterrupt(halHdl);
    }

    if (halHdl->readRxdMethod == kNafeHalReadRxdMethod_polling)
    {
        IOMUXC_SetPinMux(BOARD_NAFE_DRDY_GPIO_MUX_CFG, 0U);
    }
    else if (halHdl->readRxdMethod == kNafeHalReadRxdMethod_interrupt)
    {
        IOMUXC_SetPinMux(BOARD_NAFE_DRDY_GPIO_MUX_CFG, 0U);
        /* Enable SPI and drdy Pin IRQ For interrupt methods. */
        NVIC_ClearPendingIRQ(BOARD_NAFE_SPI_IRQN);
        NVIC_ClearPendingIRQ(BOARD_NAFE_DRDY_IRQN);
        NVIC_EnableIRQ(BOARD_NAFE_SPI_IRQN);
        NVIC_EnableIRQ(BOARD_NAFE_DRDY_IRQN);
        NVIC_SetPriority(BOARD_NAFE_SPI_IRQN, 5);
		NVIC_SetPriority(BOARD_NAFE_DRDY_IRQN, 6);

    }
    else if (halHdl->readRxdMethod == kNafeHalReadRxdMethod_dma)
    {
        NAFE_HAL_initFlexIO(halHdl);
        NAFE_HAL_initDma(halHdl);

        /* Enable DMA permission and IRQ. */
        NVIC_EnableIRQ(BOARD_NAFE_SPIRXD_DMA_IRQN);

        mask |= halHdl->drdyDma == DMA3 || halHdl->spiRxdDma == DMA3 ? kPermissionEdma3 : 0;
        mask |= halHdl->drdyDma == DMA4 || halHdl->spiRxdDma == DMA4 ? kPermissionEdma4 : 0;
        EDMA_SetPermission(mask);

        IOMUXC_SetPinMux(BOARD_NAFE_DRDY_FLEXIO_MUX_CFG, 0U);
    }
}

static void NAFE_HAL_initSpi(NAFE_HAL_hdl_t *halHdl)
{
    lpspi_master_config_t lpspi_master_config = {0};

    LPSPI_MasterGetDefaultConfig(&lpspi_master_config);

    lpspi_master_config.baudRate = halHdl->spiBaudrate;
    lpspi_master_config.cpha = kLPSPI_ClockPhaseSecondEdge;
    lpspi_master_config.dataOutConfig = kLpspiDataOutTristate;

    LPSPI_MasterInit(halHdl->spi, &lpspi_master_config, CLOCK_GetRootClockFreq(BOARD_NAFE_SPI_CLOCK_ROOT));

    halHdl->spiTcrBaseline = halHdl->spi->TCR & ~(LPSPI_TCR_FRAMESZ_MASK);
}

void NAFE_HAL_SendPinRst(NAFE_HAL_hdl_t *halHdl)
{
    RGPIO_PinWrite(halHdl->rstbGpio, halHdl->rstbGpioPin, 0u);
    NAFE_HAL_DelayAtLeastUs(1u);
    RGPIO_PinWrite(halHdl->rstbGpio, halHdl->rstbGpioPin, 1u);
}

void NAFE_HAL_sendSyncPulse(NAFE_HAL_hdl_t *halHdl)
{
    RGPIO_PinWrite(halHdl->syncGpio, halHdl->syncGpioPin, 1u);
    NAFE_HAL_DelayAtLeastUs(1u);
    RGPIO_PinWrite(halHdl->syncGpio, halHdl->syncGpioPin, 0u);
}

void NAFE_HAL_writeRegNonBlock(NAFE_HAL_hdl_t *halHdl, uint16_t cmd, uint32_t data, uint32_t dataBits)
{
    NAFE_HAL_spiClearInterruptFlags(halHdl);

    /* Set frame size to 16. */
    halHdl->spi->TCR = halHdl->spiTcrBaseline | LPSPI_TCR_CONT(1u) | LPSPI_TCR_RXMSK_MASK | LPSPI_TCR_FRAMESZ(15u);

    NAFE_HAL_spiCsWrite(halHdl, 0u);

    /* Transfer the 16-bit command. */
    halHdl->spi->TDR = cmd;

    /* Set frame size to register size. */
    halHdl->spi->TCR = halHdl->spiTcrBaseline | LPSPI_TCR_CONTC(1u) | LPSPI_TCR_RXMSK_MASK | LPSPI_TCR_FRAMESZ(dataBits - 1u);

    /* Transfer the data. */
    halHdl->spi->TDR = data;
}

void NAFE_HAL_writeRegBlock(NAFE_HAL_hdl_t *halHdl, uint16_t cmd,
                            uint32_t data, uint32_t dataBits)
{
    NAFE_HAL_writeRegNonBlock(halHdl, cmd, data, dataBits);

    /* Wait for the frame transfer to be completed.*/
    while (!NAFE_HAL_checkSpiStatus(halHdl, kNafeHalInterruptType_frameComplete))
    {
    }

    NAFE_HAL_spiCsWrite(halHdl, 1u);
}

void NAFE_HAL_readRegNonBlock(NAFE_HAL_hdl_t *halHdl, uint16_t cmd, uint32_t dataBits)
{
    NAFE_HAL_spiClearInterruptFlags(halHdl);

    /* Set frame size to 16. */
    halHdl->spi->TCR = halHdl->spiTcrBaseline | LPSPI_TCR_CONT(1u) | LPSPI_TCR_RXMSK_MASK | LPSPI_TCR_FRAMESZ(15u);

    NAFE_HAL_spiCsWrite(halHdl, 0u);

    /* Transfer the 16-bit command. */
    halHdl->spi->TDR = cmd;

    /* Set frame size and start the reading transfer. */
    halHdl->spi->TCR = halHdl->spiTcrBaseline | LPSPI_TCR_CONTC(1u) | LPSPI_TCR_TXMSK_MASK | LPSPI_TCR_FRAMESZ(dataBits - 1u);
}

void NAFE_HAL_readRegBlock(NAFE_HAL_hdl_t *halHdl, uint16_t cmd, uint32_t *data, uint32_t dataBits)
{
    NAFE_HAL_readRegNonBlock(halHdl, cmd, dataBits);

    /* Wait for the transfer to be completed.*/
    while (!NAFE_HAL_checkSpiStatus(halHdl, kNafeHalInterruptType_receiveData))
    {
    }

    NAFE_HAL_spiCsWrite(halHdl, 1u);

    NAFE_HAL_readSpiRxd(halHdl, data, dataBits, 1u);
}

void NAFE_HAL_sendCmdNonBlock(NAFE_HAL_hdl_t *halHdl, uint16_t cmd)
{
    NAFE_HAL_spiClearInterruptFlags(halHdl);

    uint32_t tcr_reg = halHdl->spiTcrBaseline | LPSPI_TCR_RXMSK_MASK | LPSPI_TCR_FRAMESZ(15u);

    halHdl->spi->TCR = tcr_reg;

    NAFE_HAL_spiCsWrite(halHdl, 0u);

    /* Transfer the 16-bit command. */
    halHdl->spi->TDR = cmd;
}

void NAFE_HAL_sendCmdBlock(NAFE_HAL_hdl_t *halHdl, uint16_t cmd, uint8_t csEndLevel)
{
    NAFE_HAL_sendCmdNonBlock(halHdl, cmd);

    /* Wait for the transfer to be completed.*/
    while (!NAFE_HAL_checkSpiStatus(halHdl, kNafeHalInterruptType_frameComplete))
    {
    }

    NAFE_HAL_spiCsWrite(halHdl, csEndLevel);
}

void NAFE_HAL_receiveResultBlock(NAFE_HAL_hdl_t *halHdl, void *data, uint32_t dataBits, uint32_t len, bool terminateFrame)
{
    NAFE_HAL_receiveResultNonBlock(halHdl, dataBits);

    /* Wait for the transfer to be completed.*/
    while (!NAFE_HAL_checkSpiStatus(halHdl, kNafeHalInterruptType_receiveData))
    {
    }

    if (terminateFrame)
    {
        NAFE_HAL_spiCsWrite(halHdl, 1u);
    }

    NAFE_HAL_readSpiRxd(halHdl, data, dataBits, len);
}

/* Configure FlexIO to use DRDY pin to trigger DMA requests. */
static void NAFE_HAL_initFlexIO(NAFE_HAL_hdl_t *halHdl)
{
    flexio_config_t flexio_config = {0};
    flexio_shifter_config_t shifterConfig = {0};
    flexio_timer_config_t timerConfig = {0};

    FLEXIO_GetDefaultConfig(&flexio_config);
    FLEXIO_Init(halHdl->flexio, &flexio_config);

    shifterConfig.timerSelect   = halHdl->flexioTimerIndex;
    shifterConfig.pinConfig     = kFLEXIO_PinConfigOutputDisabled;
    shifterConfig.shifterMode   = kFLEXIO_ShifterModeReceive;
    shifterConfig.shifterStop   = kFLEXIO_ShifterStopBitDisable;
    shifterConfig.shifterStart  = kFLEXIO_ShifterStartBitDisabledLoadDataOnEnable;
    shifterConfig.timerPolarity = kFLEXIO_ShifterTimerPolarityOnPositive;

    FLEXIO_SetShifterConfig(halHdl->flexio, halHdl->flexioShifterIndex, &shifterConfig);

    timerConfig.triggerSelect   = FLEXIO_TIMER_TRIGGER_SEL_PININPUT(halHdl->flexioPin);
    timerConfig.triggerPolarity = kFLEXIO_TimerTriggerPolarityActiveLow;
    timerConfig.triggerSource   = kFLEXIO_TimerTriggerSourceInternal;
    timerConfig.pinConfig       = kFLEXIO_PinConfigOutputDisabled;
    timerConfig.pinSelect       = halHdl->flexioPin;
    timerConfig.pinPolarity     = kFLEXIO_PinActiveHigh;
    timerConfig.timerMode       = kFLEXIO_TimerModeSingle16Bit;
    timerConfig.timerOutput     = kFLEXIO_TimerOutputZeroNotAffectedByReset;
    timerConfig.timerDecrement  = kFLEXIO_TimerDecSrcOnPinInputShiftPinInput;
    timerConfig.timerReset      = kFLEXIO_TimerResetOnTimerTriggerRisingEdge;
    timerConfig.timerEnable     = kFLEXIO_TimerEnableOnTriggerRisingEdge;
    timerConfig.timerStop       = kFLEXIO_TimerStopBitDisabled;
    timerConfig.timerDisable    = kFLEXIO_TimerDisableOnTriggerFallingEdge;
    timerConfig.timerStart      = kFLEXIO_TimerStartBitDisabled;
    timerConfig.timerCompare    = 0xFFFF;

    FLEXIO_SetTimerConfig(halHdl->flexio, halHdl->flexioTimerIndex, &timerConfig);

    /* Enable FlexIO. */
    halHdl->flexio->CTRL = FLEXIO_CTRL_DBGE_MASK | FLEXIO_CTRL_FLEXEN_MASK;
}

static void NAFE_HAL_initDma(NAFE_HAL_hdl_t *halHdl)
{
    edma_config_t dmaConfig = {0};
    edma_tcd_t tcdMemory[2];
    edma_tcd_t *tcd = (edma_tcd_t *)(((uint32_t)(tcdMemory) + 32U) & ~0x1FU); /* Used for 32byte aligned */
    edma_transfer_size_t dmaTransferSizeCode = 0;
    uint32_t adcResultBytes = 0;
    static uint32_t dummyReadSink = 0;
    static uint32_t lpspiTcr = 0;

    lpspiTcr = halHdl->spiTcrBaseline | LPSPI_TCR_TXMSK_MASK | LPSPI_TCR_FRAMESZ(halHdl->adcResolutionBits - 1u);

    if (halHdl->adcResolutionBits == 24u)
    {
        dmaTransferSizeCode = kEDMA_TransferSize4Bytes;
        adcResultBytes = 4u;
    }
    else if (halHdl->adcResolutionBits == 16u)
    {
        dmaTransferSizeCode = kEDMA_TransferSize2Bytes;
        adcResultBytes = 2u;
    }

    EDMA_GetDefaultConfig(&dmaConfig);
    dmaConfig.enableMasterIdReplication = true;

    EDMA_Init(halHdl->drdyDma, &dmaConfig);
    if (halHdl->drdyDma != halHdl->spiRxdDma)
    {
        EDMA_Init(halHdl->spiRxdDma, &dmaConfig);
    }

    /* Configure DRDY DMA channel. */
    EDMA_EnableChannelMasterIDReplication(halHdl->drdyDma, halHdl->drdyDmaChn, true);
    EDMA_SetChannelProtectionLevel(halHdl->drdyDma, halHdl->drdyDmaChn, kEDMA_ChannelProtectionLevelPrivileged);
    EDMA_SetChannelSecurityLevel(halHdl->drdyDma, halHdl->drdyDmaChn, kEDMA_ChannelSecurityLevelSecure);
    EDMA_SetChannelMux(halHdl->drdyDma, halHdl->drdyDmaChn, halHdl->drdyDmaReqSrc);


    tcd->TCD_REGS.edma4_tcd.SADDR = MEMORY_ConvertMemoryMapAddress((uint32_t)halHdl->flexio->SHIFTBUF + halHdl->flexioShifterIndex * 4u, kMEMORY_Local2DMA);
    tcd->TCD_REGS.edma4_tcd.SOFF = 0u;
    tcd->TCD_REGS.edma4_tcd.ATTR = DMA_ATTR_SSIZE(kEDMA_TransferSize4Bytes) | DMA_ATTR_DSIZE(kEDMA_TransferSize4Bytes);
    tcd->TCD_REGS.edma4_tcd.NBYTES = 4u;
    tcd->TCD_REGS.edma4_tcd.SLAST = 0u;
    tcd->TCD_REGS.edma4_tcd.DADDR = MEMORY_ConvertMemoryMapAddress((uint32_t)&dummyReadSink, kMEMORY_Local2DMA);
    tcd->TCD_REGS.edma4_tcd.DOFF = 0u;
    tcd->TCD_REGS.edma4_tcd.CITER = 0u;
    tcd->TCD_REGS.edma4_tcd.DLAST_SGA = 0u;
    tcd->TCD_REGS.edma4_tcd.CSR = DMA_CSR_MAJORELINK_MASK | DMA_CSR_MAJORLINKCH(halHdl->spiRxcDmaChn) | DMA_CSR_DREQ_MASK;
    tcd->TCD_REGS.edma4_tcd.BITER = 0u;
    EDMA_InstallTCD(halHdl->drdyDma, halHdl->drdyDmaChn, tcd);

    /* Configure SPI RXC DMA channel. */
    EDMA_EnableChannelMasterIDReplication(halHdl->drdyDma, halHdl->spiRxcDmaChn, true);
    EDMA_SetChannelProtectionLevel(halHdl->drdyDma, halHdl->spiRxcDmaChn, kEDMA_ChannelProtectionLevelPrivileged);
    EDMA_SetChannelSecurityLevel(halHdl->drdyDma, halHdl->spiRxcDmaChn, kEDMA_ChannelSecurityLevelSecure);

    tcd->TCD_REGS.edma4_tcd.SADDR = MEMORY_ConvertMemoryMapAddress((uint32_t)&lpspiTcr, kMEMORY_Local2DMA);
    tcd->TCD_REGS.edma4_tcd.SOFF = 0u;
    tcd->TCD_REGS.edma4_tcd.ATTR = DMA_ATTR_SSIZE(kEDMA_TransferSize4Bytes) | DMA_ATTR_DSIZE(kEDMA_TransferSize4Bytes);
    tcd->TCD_REGS.edma4_tcd.NBYTES = DMA_NBYTES_MLOFFNO_NBYTES(halHdl->dmaChnAmt * 4u);
    tcd->TCD_REGS.edma4_tcd.SLAST = 0u;
    tcd->TCD_REGS.edma4_tcd.DADDR = MEMORY_ConvertMemoryMapAddress((uint32_t)&halHdl->spi->TCR, kMEMORY_Local2DMA);
    tcd->TCD_REGS.edma4_tcd.DOFF = 0u;
    tcd->TCD_REGS.edma4_tcd.CITER = 0u;
    tcd->TCD_REGS.edma4_tcd.DLAST_SGA = 0u;
    tcd->TCD_REGS.edma4_tcd.CSR = DMA_CSR_DREQ_MASK;
    tcd->TCD_REGS.edma4_tcd.BITER = 0u;
    EDMA_InstallTCD(halHdl->drdyDma, halHdl->spiRxcDmaChn, tcd);

    /* Configure spiRxdDmaChn, used to read LPSPI Rx FIFO. */
    EDMA_EnableChannelMasterIDReplication(halHdl->spiRxdDma, halHdl->spiRxdDmaChn, true);
    EDMA_SetChannelProtectionLevel(halHdl->spiRxdDma, halHdl->spiRxdDmaChn, kEDMA_ChannelProtectionLevelPrivileged);
    EDMA_SetChannelSecurityLevel(halHdl->spiRxdDma, halHdl->spiRxdDmaChn, kEDMA_ChannelSecurityLevelSecure);
    EDMA_SetChannelMux(halHdl->spiRxdDma, halHdl->spiRxdDmaChn, halHdl->spiRxDmaReqSrc);

    tcd->TCD_REGS.edma4_tcd.SADDR = MEMORY_ConvertMemoryMapAddress((uint32_t)&halHdl->spi->RDR, kMEMORY_Local2DMA);
    tcd->TCD_REGS.edma4_tcd.SOFF = 0u;
    tcd->TCD_REGS.edma4_tcd.ATTR = DMA_ATTR_SSIZE(dmaTransferSizeCode) | DMA_ATTR_DSIZE(dmaTransferSizeCode);
    if (halHdl->dmaChnAmt == 1u)
    {
        tcd->TCD_REGS.edma4_tcd.NBYTES = adcResultBytes;
    }
    else
    {
        tcd->TCD_REGS.edma4_tcd.NBYTES = DMA_NBYTES_MLOFFYES_DMLOE_MASK
         | DMA_NBYTES_MLOFFYES_MLOFF(-halHdl->dmaContSampleAmt * halHdl->dmaChnAmt * adcResultBytes + adcResultBytes)
         | DMA_NBYTES_MLOFFYES_NBYTES(halHdl->dmaChnAmt * adcResultBytes);
    }
    tcd->TCD_REGS.edma4_tcd.SLAST = 0u;
    tcd->TCD_REGS.edma4_tcd.DADDR = MEMORY_ConvertMemoryMapAddress((uint32_t)halHdl->dmaData, kMEMORY_Local2DMA);
    if (halHdl->dmaChnAmt == 1u)
    {
        tcd->TCD_REGS.edma4_tcd.DOFF = adcResultBytes;
    }
    else
    {
        tcd->TCD_REGS.edma4_tcd.DOFF = halHdl->dmaContSampleAmt * adcResultBytes;
    }
    tcd->TCD_REGS.edma4_tcd.CITER = 0u;
    tcd->TCD_REGS.edma4_tcd.DLAST_SGA = 0u;
    tcd->TCD_REGS.edma4_tcd.CSR = DMA_CSR_DREQ_MASK | DMA_CSR_INTMAJOR_MASK;
    tcd->TCD_REGS.edma4_tcd.BITER = 0u;
    EDMA_InstallTCD(halHdl->spiRxdDma, halHdl->spiRxdDmaChn, tcd);
}

#define MAX_MAJOR_LOOP_SIZE DMA_TCD_BITER_ELINKYES_BITER_MASK

static void NAFE_HAL_refreshDmaTcd(NAFE_HAL_hdl_t *halHdl)
{
    static uint32_t dmaMajorLoopSize = 0;
    uint32_t remainSamples = 0;
    uint32_t adcResultBytes = halHdl->adcResolutionBits == 24u ? 4u : 2u;
    edma_tcd_t *drdyDmaTcd = EDMA_TCD_BASE(halHdl->drdyDma, halHdl->drdyDmaChn);
    edma_tcd_t *spiRxcDmaTcd = EDMA_TCD_BASE(halHdl->drdyDma, halHdl->spiRxcDmaChn);
    edma_tcd_t *spiRxdDmaTcd = EDMA_TCD_BASE(halHdl->spiRxdDma, halHdl->spiRxdDmaChn);

    remainSamples = halHdl->dmaContSampleAmt - halHdl->dmaContSampleCnt;
    dmaMajorLoopSize = remainSamples <= MAX_MAJOR_LOOP_SIZE ? remainSamples : MAX_MAJOR_LOOP_SIZE;

    drdyDmaTcd->TCD_REGS.edma4_tcd.CITER = dmaMajorLoopSize;
    drdyDmaTcd->TCD_REGS.edma4_tcd.BITER = dmaMajorLoopSize;
    spiRxcDmaTcd->TCD_REGS.edma4_tcd.CITER = dmaMajorLoopSize;
    spiRxcDmaTcd->TCD_REGS.edma4_tcd.BITER = dmaMajorLoopSize;
    spiRxdDmaTcd->TCD_REGS.edma4_tcd.CITER = dmaMajorLoopSize;
    spiRxdDmaTcd->TCD_REGS.edma4_tcd.BITER = dmaMajorLoopSize;
    spiRxdDmaTcd->TCD_REGS.edma4_tcd.DADDR = MEMORY_ConvertMemoryMapAddress((uint32_t)halHdl->dmaData +
                          halHdl->dmaContSampleCnt * adcResultBytes, kMEMORY_Local2DMA);

    EDMA_SetChannelLink(halHdl->drdyDma, halHdl->drdyDmaChn, kEDMA_MinorLink, halHdl->spiRxcDmaChn);

    halHdl->dmaContSampleCnt += dmaMajorLoopSize;
}

void NAFE_HAL_prepareDmaXfer(NAFE_HAL_hdl_t *halHdl)
{
    NAFE_HAL_refreshDmaTcd(halHdl);

    /* Enable DMA requests. */
    EDMA_EnableChannelRequest(halHdl->drdyDma, halHdl->drdyDmaChn);
    EDMA_EnableChannelRequest(halHdl->drdyDma, halHdl->spiRxcDmaChn);
    EDMA_EnableChannelRequest(halHdl->spiRxdDma, halHdl->spiRxdDmaChn);

    NAFE_HAL_spiClearInterruptFlags(halHdl);
    LPSPI_EnableDMA(halHdl->spi, LPSPI_DER_RDDE_MASK); /* Enable SPI RXD DMA. */

    FLEXIO_ClearShifterStatusFlags(halHdl->flexio, 1u << halHdl->flexioShifterIndex);      /* Clear FlexIO shifter status. */
    FLEXIO_EnableShifterStatusDMA(halHdl->flexio, 1u << halHdl->flexioShifterIndex, true); /* Enable FlexIO shifter DMA. */
}

void NAFE_HAL_dmaIrqHandle(NAFE_HAL_hdl_t *halHdl)
{
    if (halHdl->dmaContSampleCnt < halHdl->dmaContSampleAmt)
    {
        NAFE_HAL_refreshDmaTcd(halHdl);
    }
    else
    {
        FLEXIO_EnableShifterStatusDMA(halHdl->flexio, 1u << halHdl->flexioShifterIndex, false); /* Disable FlexIO shifter DMA. */
        LPSPI_DisableDMA(halHdl->spi, LPSPI_DER_RDDE_MASK);                                     /* Disable SPI RXD DMA. */

        /* Disable DMA requests. */
        EDMA_DisableChannelRequest(halHdl->drdyDma, halHdl->drdyDmaChn);
        EDMA_DisableChannelRequest(halHdl->drdyDma, halHdl->spiRxcDmaChn);
        EDMA_DisableChannelRequest(halHdl->spiRxdDma, halHdl->spiRxdDmaChn);
    }

    /* Clear flags. */
    EDMA_ClearChannelStatusFlags(halHdl->drdyDma, halHdl->drdyDmaChn, kEDMA_DoneFlag);
    EDMA_ClearChannelStatusFlags(halHdl->drdyDma, halHdl->spiRxcDmaChn, kEDMA_DoneFlag);
    EDMA_ClearChannelStatusFlags(halHdl->spiRxdDma, halHdl->spiRxdDmaChn, kEDMA_DoneFlag | kEDMA_InterruptFlag);
}
