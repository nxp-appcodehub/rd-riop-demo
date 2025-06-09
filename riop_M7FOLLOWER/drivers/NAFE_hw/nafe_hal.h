/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _NAFE_HAL_H_
#define _NAFE_HAL_H_

#include "fsl_device_registers.h"
#include "board.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

typedef enum
{
    kNafeHalInterruptType_frameComplete = LPSPI_IER_FCIE_MASK,
    kNafeHalInterruptType_receiveData = LPSPI_IER_RDIE_MASK,
} NAFE_HAL_spiStatus_t;

typedef enum
{
    kNafeHalReadRxdMethod_none,
    kNafeHalReadRxdMethod_polling,
    kNafeHalReadRxdMethod_interrupt,
    kNafeHalReadRxdMethod_dma
} NAFE_HAL_readRxdMethod_t;

typedef struct
{
    /* SPI and GPIO */
    LPSPI_Type *spi;
    uint32_t spiTcrBaseline;
    uint32_t spiBaudrate;
    RGPIO_Type *csGpio;
    uint32_t csGpioPin;
    RGPIO_Type *drdyGpio;
    uint32_t drdyGpioPin;
    RGPIO_Type *syncGpio;
    uint32_t syncGpioPin;
    RGPIO_Type *rstbGpio;
    uint32_t rstbGpioPin;

    /* DMA approaches. */
    FLEXIO_Type *flexio;
    void *drdyDma;
    void *spiRxdDma;
    uint32_t drdyDmaChn;
    uint32_t spiRxcDmaChn;
    uint32_t spiRxdDmaChn;
    uint32_t drdyDmaReqSrc;
    uint32_t spiRxDmaReqSrc;
    uint32_t flexioTimerIndex;
    uint32_t flexioShifterIndex;
    uint32_t flexioPin;

    /* NAFE driver layer data */
    NAFE_HAL_readRxdMethod_t readRxdMethod;
    uint32_t adcResolutionBits;
    void *dmaData;
    uint32_t dmaChnAmt;
    uint32_t dmaContSampleAmt;
    uint32_t dmaContSampleCnt;
} NAFE_HAL_hdl_t;

/*******************************************************************************
 * APIs
 ******************************************************************************/
void NAFE_HAL_init(NAFE_HAL_hdl_t *halHdl);

static void inline NAFE_HAL_DelayAtLeastUs(uint32_t us)
{
    SDK_DelayAtLeastUs(us, BOARD_BOOTCLOCKRUN_CORE_CLOCK);
}

__STATIC_FORCEINLINE bool NAFE_HAL_checkSpiStatus(NAFE_HAL_hdl_t *halHdl, uint32_t status)
{
    bool rel = false;

    if (status & kNafeHalInterruptType_frameComplete)
    {
        rel = halHdl->spi->SR &= LPSPI_SR_FCF_MASK;
    }
    else if (status & kNafeHalInterruptType_receiveData)
    {
        rel = halHdl->spi->SR &= LPSPI_SR_RDF_MASK;
    }

    return rel;
}

__STATIC_FORCEINLINE void NAFE_HAL_spiClearInterruptFlags(NAFE_HAL_hdl_t *halHdl)
{
    /* Reset RX FIFO and TX FIFO. */
    halHdl->spi->CR |= LPSPI_CR_RRF_MASK | LPSPI_CR_RTF_MASK;

    /* Clear Status Register. */
    halHdl->spi->SR |= LPSPI_SR_FCF_MASK;
}

__STATIC_FORCEINLINE void NAFE_HAL_configSpiRxFifoTrigLevel(NAFE_HAL_hdl_t *halHdl, uint32_t rxFifoTrigLevel)
{
    halHdl->spi->FCR = LPSPI_FCR_RXWATER(rxFifoTrigLevel - 1u);
}

__STATIC_FORCEINLINE void NAFE_HAL_configSpiInterrupt(NAFE_HAL_hdl_t *halHdl, uint32_t interrupts)
{
    halHdl->spi->IER = interrupts;
}

__STATIC_FORCEINLINE void NAFE_HAL_readSpiRxd(NAFE_HAL_hdl_t *halHdl, void *data, uint32_t dataBits, uint32_t len)
{
    if (dataBits == 24u)
    {
        uint32_t *u32Data = (uint32_t *)data;

        while (len--)
        {
            *u32Data++ = halHdl->spi->RDR;
        }
    }
    else if (dataBits == 16u)
    {
        uint16_t *u16Data = (uint16_t *)data;

        while (len--)
        {
            *u16Data++ = halHdl->spi->RDR;
        }
    }
}

__STATIC_FORCEINLINE void NAFE_HAL_spiCsWrite(NAFE_HAL_hdl_t *halHdl, uint8_t output)
{
    if (output == 0U)
    {
        halHdl->csGpio->PCOR = 1UL << halHdl->csGpioPin;
    }
    else
    {
        halHdl->csGpio->PSOR = 1UL << halHdl->csGpioPin;
    }
}

__STATIC_FORCEINLINE void NAFE_HAL_drdyEnableInterrupt(NAFE_HAL_hdl_t *halHdl)
{
    halHdl->drdyGpio->ICR[halHdl->drdyGpioPin] = (halHdl->drdyGpio->ICR[halHdl->drdyGpioPin]
    & ~(RGPIO_ICR_IRQC_MASK | RGPIO_ICR_IRQS_MASK)) | RGPIO_ICR_IRQC(kRGPIO_InterruptRisingEdge) | RGPIO_ICR_IRQS(kRGPIO_InterruptOutput0);
}

__STATIC_FORCEINLINE void NAFE_HAL_drdyDisableInterrupt(NAFE_HAL_hdl_t *halHdl)
{
    halHdl->drdyGpio->ICR[halHdl->drdyGpioPin] = (halHdl->drdyGpio->ICR[halHdl->drdyGpioPin]
    & ~(RGPIO_ICR_IRQC_MASK | RGPIO_ICR_IRQS_MASK)) | RGPIO_ICR_IRQC(kRGPIO_FlagRisingEdge) | RGPIO_ICR_IRQS(kRGPIO_InterruptOutput0);
}

__STATIC_FORCEINLINE void NAFE_HAL_drdyClearInterruptFlag(NAFE_HAL_hdl_t *halHdl)
{
    halHdl->drdyGpio->ISFR[(uint8_t)kRGPIO_InterruptOutput0] = 1u << halHdl->drdyGpioPin;
}

__STATIC_FORCEINLINE bool NAFE_HAL_drdyGetInterruptFlag(NAFE_HAL_hdl_t *halHdl)
{
    return (halHdl->drdyGpio->ISFR[(uint8_t)kRGPIO_InterruptOutput0] & (1u << halHdl->drdyGpioPin)) ? true : false;
}

__STATIC_FORCEINLINE void NAFE_HAL_receiveResultNonBlock(NAFE_HAL_hdl_t *halHdl, uint32_t dataBits)
{
    /* Start the RX transfer. */
    halHdl->spi->TCR = halHdl->spiTcrBaseline | LPSPI_TCR_TXMSK_MASK | LPSPI_TCR_FRAMESZ(dataBits - 1u);
}

void NAFE_HAL_SendPinRst(NAFE_HAL_hdl_t *halHdl);
void NAFE_HAL_sendSyncPulse(NAFE_HAL_hdl_t *halHdl);
void NAFE_HAL_writeRegNonBlock(NAFE_HAL_hdl_t *halHdl, uint16_t cmd, uint32_t data, uint32_t dataBits);
void NAFE_HAL_writeRegBlock(NAFE_HAL_hdl_t *halHdl, uint16_t cmd, uint32_t data, uint32_t dataBits);
void NAFE_HAL_readRegNonBlock(NAFE_HAL_hdl_t *halHdl, uint16_t cmd, uint32_t dataBits);
void NAFE_HAL_readRegBlock(NAFE_HAL_hdl_t *halHdl, uint16_t cmd, uint32_t *data, uint32_t dataBits);
void NAFE_HAL_sendCmdNonBlock(NAFE_HAL_hdl_t *halHdl, uint16_t cmd);
void NAFE_HAL_sendCmdBlock(NAFE_HAL_hdl_t *halHdl, uint16_t cmd, uint8_t csEndLevel);
void NAFE_HAL_receiveResultBlock(NAFE_HAL_hdl_t *halHdl, void *data, uint32_t dataBits, uint32_t len, bool terminateFrame);
void NAFE_HAL_prepareDmaXfer(NAFE_HAL_hdl_t *halHdl);
void NAFE_HAL_dmaIrqHandle(NAFE_HAL_hdl_t *halHdl);

#endif
