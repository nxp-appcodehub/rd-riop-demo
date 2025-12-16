/*
 * Copyright 2021-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include "clock_config.h"
#include "fsl_common.h"
#include "fsl_rgpio.h"
#include "fsl_clock.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief The board name */
#define BOARD_NAME "MIMXRT1180-RIOP"
#ifndef DEBUG_CONSOLE_UART_INDEX
#define DEBUG_CONSOLE_UART_INDEX 1
#endif





/* The UART to use for debug messages. */
#define BOARD_DEBUG_UART_CLK_FREQ BOARD_DebugConsoleSrcFreq()
#define BOARD_DEBUG_UART_TYPE     kSerialPort_Uart
#ifndef BOARD_DEBUG_UART_CLK_ROOT
#define BOARD_DEBUG_UART_CLK_ROOT kCLOCK_Root_Lpuart0102
#endif
#ifndef BOARD_DEBUG_UART_BASEADDR
#define BOARD_DEBUG_UART_BASEADDR (uint32_t) LPUART1
#endif
#ifndef BOARD_DEBUG_UART_INSTANCE
#define BOARD_DEBUG_UART_INSTANCE 1U
#endif
#ifndef BOARD_UART_IRQ
#define BOARD_UART_IRQ LPUART1_IRQn
#endif
#ifndef BOARD_UART_IRQ_HANDLER
#define BOARD_UART_IRQ_HANDLER LPUART1_IRQHandler
#endif
#ifndef BOARD_DEBUG_UART_BAUDRATE
#define BOARD_DEBUG_UART_BAUDRATE (115200U)
#endif

/* Definitions for eRPC MU transport layer */
#if defined(FSL_FEATURE_MU_SIDE_A)
#define MU_BASE        MU1_MUA
#define MU_IRQ         MU1_IRQn
#define MU_IRQ_HANDLER MU1_IRQHandler
#endif
#if defined(FSL_FEATURE_MU_SIDE_B)
#define MU_BASE        MU1_MUB
#define MU_IRQ         MU1_IRQn
#define MU_IRQ_HANDLER MU1_IRQHandler
#endif
#define MU_IRQ_PRIORITY (2)

/*! @brief The USER_LED used for board */
#define LOGIC_LED_ON  (1U)
#define LOGIC_LED_OFF (0U)

#ifndef BOARD_USER_LED_RED_GPIO
#define BOARD_USER_LED_RED_GPIO LED_RED_PERIPHERAL
#endif
#ifndef BOARD_USER_LED_RED_GPIO_PIN
#define BOARD_USER_LED_RED_GPIO_PIN LED_RED_GPIO_PIN
#endif

#ifndef BOARD_USER_LED_BLUE_GPIO
#define BOARD_USER_LED_BLUE_GPIO LED_BLUE_GPIO
#endif
#ifndef BOARD_USER_LED_BLUE_GPIO_PIN
#define BOARD_USER_LED_BLUE_GPIO_PIN LED_BLUE_GPIO_PIN
#endif

#ifndef BOARD_USER_LED_GREEN_GPIO
#define BOARD_USER_LED_GREEN_GPIO LED_GREEN_GPIO
#endif
#ifndef BOARD_USER_LED_GREEN_GPIO_PIN
#define BOARD_USER_LED_GREEN_GPIO_PIN LED_GREEN_GPIO_PIN
#endif


#define USER_LED_RED_INIT(output)                                             \
    RGPIO_PinWrite(BOARD_USER_LED_RED_GPIO, BOARD_USER_LED_RED_GPIO_PIN, output); \
    BOARD_USER_LED_RED_GPIO->PDDR |= (1U << BOARD_USER_LED_RED_GPIO_PIN)                        /*!< Enable target USER_LED */
#define USER_LED_RED_OFF() \
    RGPIO_PortClear(BOARD_USER_LED_RED_GPIO, 1U << BOARD_USER_LED_RED_GPIO_PIN)                 /*!< Turn off target USER_LED */
#define USER_LED_RED_ON() RGPIO_PortSet(BOARD_USER_LED_RED_GPIO, 1U << BOARD_USER_LED_RED_GPIO_PIN) /*!<Turn on target USER_LED*/
#define USER_LED_RED_TOGGLE() \
    RGPIO_PortToggle(BOARD_USER_LED_RED_GPIO, 1u << BOARD_USER_LED_RED_GPIO_PIN)                /*!< Toggle target USER_LED */


#define USER_LED_GREEN_INIT(output)                                             \
    RGPIO_PinWrite(BOARD_USER_LED_GREEN_GPIO, BOARD_USER_LED_GREEN_GPIO_PIN, output); \
    BOARD_USER_LED_GREEN_GPIO->PDDR |= (1U << BOARD_USER_LED_GREEN_GPIO_PIN)                        /*!< Enable target USER_LED */
#define USER_LED_GREEN_OFF() \
    RGPIO_PortClear(BOARD_USER_LED_GREEN_GPIO, 1U << BOARD_USER_LED_GREEN_GPIO_PIN)                 /*!< Turn off target USER_LED */
#define USER_LED_GREEN_ON() RGPIO_PortSet(BOARD_USER_LED_GREEN_GPIO, 1U << BOARD_USER_LED_GREEN_GPIO_PIN) /*!<Turn on target USER_LED*/
#define USER_LED_GREEN_TOGGLE() \
    RGPIO_PortToggle(BOARD_USER_LED_GREEN_GPIO, 1u << BOARD_USER_LED_GREEN_GPIO_PIN)                /*!< Toggle target USER_LED */


#define USER_LED_BLUE_INIT(output)                                             \
    RGPIO_PinWrite(BOARD_USER_LED_BLUE_GPIO, BOARD_USER_LED_BLUE_GPIO_PIN, output); \
    BOARD_USER_LED_BLUE_GPIO->PDDR |= (1U << BOARD_USER_LED_BLUE_GPIO_PIN)                        /*!< Enable target USER_LED */
#define USER_LED_BLUE_OFF() \
    RGPIO_PortClear(BOARD_USER_LED_BLUE_GPIO, 1U << BOARD_USER_LED_BLUE_GPIO_PIN)                 /*!< Turn off target USER_LED */
#define USER_LED_BLUE_ON() RGPIO_PortSet(BOARD_USER_LED_BLUE_GPIO, 1U << BOARD_USER_LED_BLUE_GPIO_PIN) /*!<Turn on target USER_LED*/
#define USER_LED_BLUE_TOGGLE() \
    RGPIO_PortToggle(BOARD_USER_LED_BLUE_GPIO, 1u << BOARD_USER_LED_BLUE_GPIO_PIN)                /*!< Toggle target USER_LED */



/*! @brief Define the port interrupt number for the board switches */
#ifndef BOARD_USER_BUTTON_GPIO
#define BOARD_USER_BUTTON_GPIO USER_BUTTON_GPIO
#endif
#ifndef BOARD_USER_BUTTON_GPIO_PIN
#define BOARD_USER_BUTTON_GPIO_PIN USER_BUTTON_GPIO_PIN
#endif
#define BOARD_USER_BUTTON_IRQ         GPIO5_IRQn
#define BOARD_USER_BUTTON_IRQ_HANDLER GPIO5_IRQHandler
#define BOARD_USER_BUTTON_NAME        "SW7"


/*! @brief The DIGITAL OUTPUTs used for board */
#ifndef BOARD_DIG_OUT_1_GPIO
#define BOARD_DIG_OUT_1_GPIO DIGITAL_OUTPUT_01_PERIPHERAL
#endif
#ifndef BOARD_DIG_OUT_1_GPIO_PIN
#define BOARD_DIG_OUT_1_GPIO_PIN DIGITAL_OUTPUT_01_GPIO_PIN
#endif
#ifndef BOARD_DIG_OUT_2_GPIO
#define BOARD_DIG_OUT_2_GPIO DIGITAL_OUTPUT_02_PERIPHERAL
#endif
#ifndef BOARD_DIG_OUT_2_GPIO_PIN
#define BOARD_DIG_OUT_2_GPIO_PIN DIGITAL_OUTPUT_02_GPIO_PIN
#endif
#ifndef BOARD_DIG_OUT_3_GPIO
#define BOARD_DIG_OUT_3_GPIO DIGITAL_OUTPUT_03_PERIPHERAL
#endif
#ifndef BOARD_DIG_OUT_3_GPIO_PIN
#define BOARD_DIG_OUT_3_GPIO_PIN DIGITAL_OUTPUT_03_GPIO_PIN
#endif
#ifndef BOARD_DIG_OUT_4_GPIO
#define BOARD_DIG_OUT_4_GPIO DIGITAL_OUTPUT_04_PERIPHERAL
#endif
#ifndef BOARD_DIG_OUT_4_GPIO_PIN
#define BOARD_DIG_OUT_4_GPIO_PIN DIGITAL_OUTPUT_04_GPIO_PIN
#endif
#ifndef BOARD_DIG_OUT_5_GPIO
#define BOARD_DIG_OUT_5_GPIO DIGITAL_OUTPUT_05_PERIPHERAL
#endif
#ifndef BOARD_DIG_OUT_5_GPIO_PIN
#define BOARD_DIG_OUT_5_GPIO_PIN DIGITAL_OUTPUT_05_GPIO_PIN
#endif
#ifndef BOARD_DIG_OUT_6_GPIO
#define BOARD_DIG_OUT_6_GPIO DIGITAL_OUTPUT_06_PERIPHERAL
#endif
#ifndef BOARD_DIG_OUT_6_GPIO_PIN
#define BOARD_DIG_OUT_6_GPIO_PIN DIGITAL_OUTPUT_06_GPIO_PIN
#endif
#ifndef BOARD_DIG_OUT_7_GPIO
#define BOARD_DIG_OUT_7_GPIO DIGITAL_OUTPUT_07_PERIPHERAL
#endif
#ifndef BOARD_DIG_OUT_7_GPIO_PIN
#define BOARD_DIG_OUT_7_GPIO_PIN DIGITAL_OUTPUT_07_GPIO_PIN
#endif
#ifndef BOARD_DIG_OUT_8_GPIO
#define BOARD_DIG_OUT_8_GPIO DIGITAL_OUTPUT_08_PERIPHERAL
#endif
#ifndef BOARD_DIG_OUT_8_GPIO_PIN
#define BOARD_DIG_OUT_8_GPIO_PIN DIGITAL_OUTPUT_08_GPIO_PIN
#endif

#define DIG_OUT_1_OFF()\
    RGPIO_PortClear(BOARD_DIG_OUT_1_GPIO, 1U << BOARD_DIG_OUT_1_GPIO_PIN)
#define DIG_OUT_1_ON()\
	RRGPIO_PortSet(BOARD_DIG_OUT_1_GPIO, 1U << BOARD_DIG_OUT_1_GPIO_PIN)
#define DIG_OUT_1_TOGGLE()                                       \
    RGPIO_PortToggle(BOARD_DIG_OUT_1_GPIO, 1U << BOARD_DIG_OUT_1_GPIO_PIN)

#define DIG_OUT_2_OFF()\
    RGPIO_PortClear(BOARD_DIG_OUT_2_GPIO, 1U << BOARD_DIG_OUT_2_GPIO_PIN)
#define DIG_OUT_2_ON()\
	RGPIO_PortSet(BOARD_DIG_OUT_2_GPIO, 1U << BOARD_DIG_OUT_2_GPIO_PIN)
#define DIG_OUT_2_TOGGLE()                                       \
    RGPIO_PortToggle(BOARD_DIG_OUT_2_GPIO, 1U << BOARD_DIG_OUT_2_GPIO_PIN)

#define DIG_OUT_3_OFF()\
    RGPIO_PortClear(BOARD_DIG_OUT_3_GPIO, 1U << BOARD_DIG_OUT_3_GPIO_PIN)
#define DIG_OUT_3_ON()\
	RGPIO_PortSet(BOARD_DIG_OUT_3_GPIO, 1U << BOARD_DIG_OUT_3_GPIO_PIN)
#define DIG_OUT_3_TOGGLE()                                       \
    RGPIO_PortToggle(BOARD_DIG_OUT_3_GPIO, 1U << BOARD_DIG_OUT_3_GPIO_PIN)

#define DIG_OUT_4_OFF() \
    RGPIO_PortClear(BOARD_DIG_OUT_4_GPIO, 1U << BOARD_DIG_OUT_4_GPIO_PIN)
#define DIG_OUT_4_ON()\
	RGPIO_PortSet(BOARD_DIG_OUT_4_GPIO, 1U << BOARD_DIG_OUT_4_GPIO_PIN)
#define DIG_OUT_4_TOGGLE()                                       \
    RGPIO_PortToggle(BOARD_DIG_OUT_4_GPIO, 1U << BOARD_DIG_OUT_4_GPIO_PIN)

#define DIG_OUT_5_OFF()\
    RGPIO_PortClear(BOARD_DIG_OUT_5_GPIO, 1U << BOARD_DIG_OUT_5_GPIO_PIN)
#define DIG_OUT_5_ON()\
	RGPIO_PortSet(BOARD_DIG_OUT_5_GPIO, 1U << BOARD_DIG_OUT_5_GPIO_PIN)
#define DIG_OUT_5_TOGGLE()\
    RGPIO_PortToggle(BOARD_DIG_OUT_5_GPIO, 1U << BOARD_DIG_OUT_5_GPIO_PIN)

#define DIG_OUT_6_OFF()\
    RGPIO_PortClear(BOARD_DIG_OUT_6_GPIO, 1U << BOARD_DIG_OUT_6_GPIO_PIN)
#define DIG_OUT_6_ON()\
	RGPIO_PortSet(BOARD_DIG_OUT_6_GPIO, 1U << BOARD_DIG_OUT_6_GPIO_PIN)
#define DIG_OUT_6_TOGGLE()\
    RGPIO_PortToggle(BOARD_DIG_OUT_6_GPIO, 1U << BOARD_DIG_OUT_6_GPIO_PIN)

#define DIG_OUT_7_OFF()\
    RGPIO_PortClear(BOARD_DIG_OUT_7_GPIO, 1U << BOARD_DIG_OUT_7_GPIO_PIN)
#define DIG_OUT_7_ON()\
	RGPIO_PortSet(BOARD_DIG_OUT_7_GPIO, 1U << BOARD_DIG_OUT_7_GPIO_PIN)
#define DIG_OUT_7_TOGGLE()                                       \
    RGPIO_PortToggle(BOARD_DIG_OUT_7_GPIO, 1U << BOARD_DIG_OUT_7_GPIO_PIN)

#define DIG_OUT_8_OFF()\
    RGPIO_PortClear(BOARD_DIG_OUT_8_GPIO, 1U << BOARD_DIG_OUT_8_GPIO_PIN)
#define DIG_OUT_8_ON()\
	RGPIO_PortSet(BOARD_DIG_OUT_8_GPIO, 1U << BOARD_DIG_OUT_8_GPIO_PIN)
#define DIG_OUT_8_TOGGLE()                                       \
    RGPIO_PortToggle(BOARD_DIG_OUT_8_GPIO, 1U << BOARD_DIG_OUT_8_GPIO_PIN)


/*! @brief The DIGITAL INPUTs used for board */
#ifndef BOARD_DIG_IN1_GPIO
#define BOARD_DIG_IN1_GPIO 				DIGITAL_INPUT_01_PERIPHERAL
#endif
#ifndef BOARD_DIG_IN1_GPIO_PIN
#define BOARD_DIG_IN1_GPIO_PIN 			DIGITAL_INPUT_01_GPIO_PIN
#endif
#define BOARD_DIG_IN1_IRQ         		GPIO6_IRQn
#define BOARD_DIG_IN1_IRQ_HANDLER 		GPIO6_IRQHandler
#define BOARD_DIG_IN1_NAME        		"DIG_IN1"

#ifndef BOARD_DIG_IN2_GPIO
#define BOARD_DIG_IN2_GPIO 				DIGITAL_INPUT_02_PERIPHERAL
#endif
#ifndef BOARD_DIG_IN2_GPIO_PIN
#define BOARD_DIG_IN2_GPIO_PIN 			DIGITAL_INPUT_02_GPIO_PIN
#endif
#define BOARD_DIG_IN2_IRQ         		GPIO6_IRQn
#define BOARD_DIG_IN2_IRQ_HANDLER 		GPIO6_IRQHandler
#define BOARD_DIG_IN2_NAME        		"DIG_IN2"

#ifndef BOARD_DIG_IN3_GPIO
#define BOARD_DIG_IN3_GPIO 				DIGITAL_INPUT_03_PERIPHERAL
#endif
#ifndef BOARD_DIG_IN3_GPIO_PIN
#define BOARD_DIG_IN3_GPIO_PIN 			DIGITAL_INPUT_03_GPIO_PIN
#endif
#define BOARD_DIG_IN3_IRQ         		GPIO6_IRQn
#define BOARD_DIG_IN3_IRQ_HANDLER 		GPIO6_IRQHandler
#define BOARD_DIG_IN3_NAME        		"DIG_IN3"

#ifndef BOARD_DIG_IN4_GPIO
#define BOARD_DIG_IN4_GPIO 				DIGITAL_INPUT_04_PERIPHERAL
#endif
#ifndef BOARD_DIG_IN4_GPIO_PIN
#define BOARD_DIG_IN4_GPIO_PIN 			DIGITAL_INPUT_04_GPIO_PIN
#endif
#define BOARD_DIG_IN4_IRQ         		GPIO6_IRQn
#define BOARD_DIG_IN4_IRQ_HANDLER 		GPIO6_IRQHandler
#define BOARD_DIG_IN4_NAME        		"DIG_IN4"

#ifndef BOARD_DIG_IN5_GPIO
#define BOARD_DIG_IN5_GPIO 				DIGITAL_INPUT_05_PERIPHERAL
#endif
#ifndef BOARD_DIG_IN5_GPIO_PIN
#define BOARD_DIG_IN5_GPIO_PIN 			DIGITAL_INPUT_05_GPIO_PIN
#endif
#define BOARD_DIG_IN5_IRQ         		GPIO6_IRQn
#define BOARD_DIG_IN5_IRQ_HANDLER 		GPIO6_IRQHandler
#define BOARD_DIG_IN5_NAME        		"DIG_IN5"

#ifndef BOARD_DIG_IN6_GPIO
#define BOARD_DIG_IN6_GPIO 				DIGITAL_INPUT_06_PERIPHERAL
#endif
#ifndef BOARD_DIG_IN6_GPIO_PIN
#define BOARD_DIG_IN6_GPIO_PIN 			DIGITAL_INPUT_06_GPIO_PIN
#endif
#define BOARD_DIG_IN6_IRQ         		GPIO6_IRQn
#define BOARD_DIG_IN6_IRQ_HANDLER 		GPIO6_IRQHandler
#define BOARD_DIG_IN6_NAME        		"DIG_IN6"

#ifndef BOARD_DIG_IN7_GPIO
#define BOARD_DIG_IN7_GPIO 				DIGITAL_INPUT_07_PERIPHERAL
#endif
#ifndef BOARD_DIG_IN7_GPIO_PIN
#define BOARD_DIG_IN7_GPIO_PIN 			DIGITAL_INPUT_07_GPIO_PIN
#endif
#define BOARD_DIG_IN7_IRQ         		GPIO6_IRQn
#define BOARD_DIG_IN7_IRQ_HANDLER 		GPIO6_IRQHandler
#define BOARD_DIG_IN7_NAME        		"DIG_IN7"


#define BOARD_NAFE_DRDY_GPIO_MUX_CFG    IOMUXC_GPIO_AD_15_GPIO4_IO15
#define BOARD_NAFE_DRDY_FLEXIO_MUX_CFG  IOMUXC_GPIO_AD_15_GPIO4_IO15

#define BOARD_NAFE_DRDY_ISR             GPIO4_IRQHandler
#define BOARD_NAFE_DRDY_IRQN            GPIO4_IRQn

#define BOARD_NAFE_SPI_INSTANCE         LPSPI1
#define BOARD_NAFE_SPI_ISR              LPSPI1_IRQHandler
#define BOARD_NAFE_SPI_CLOCK_ROOT       kCLOCK_Root_Lpspi0102
#define BOARD_NAFE_SPI_IRQN             LPSPI1_IRQn


#define BOARD_NAFE_SPIRXD_DMA_IRQN      DMA3_CH0_IRQn
#define BOARD_NAFE_SPIRXD_DMA_ISR       DMA3_CH0_IRQHandler




/*! @brief The board flash size (64Mbit) */
#define BOARD_FLASH_SIZE (0x800000U)

/*! @brief The Ethernet PHY addresses. */
#define BOARD_EP0_PHY_ADDR       (0x03U)
#define BOARD_SWT_PORT2_PHY_ADDR (0x01U)
#define BOARD_SWT_PORT3_PHY_ADDR (0x00U)

/*! @brief The Ethernet PHY type of the board */
#define BOARD_USE_NETC_PHY_VSC8541

#define BOARD_HAS_SDCARD (0U)


/*! @brief The Ethernet port used by network examples, default use 1G port. */
/* Below comment is for test script to easily define which port to be used, please don't delete. */
/* @TEST_ANCHOR */
#ifndef BOARD_NETWORK_USE_100M_ENET_PORT
#define BOARD_NETWORK_USE_100M_ENET_PORT (0U)
#endif

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/
uint32_t BOARD_DebugConsoleSrcFreq(void);
void BOARD_InitDebugConsole(void);
void BOARD_ConfigMPU(void);
void BOARD_SD_Pin_Config(uint32_t speed, uint32_t strength);
void BOARD_MMC_Pin_Config(uint32_t speed, uint32_t strength);

void BOARD_FlexspiClockSafeConfig(void);
AT_QUICKACCESS_SECTION_CODE(void BOARD_SetFlexspiClock(FLEXSPI_Type *base, uint8_t src, uint32_t divider));
AT_QUICKACCESS_SECTION_CODE(void BOARD_DeinitFlash(FLEXSPI_Type *base));
AT_QUICKACCESS_SECTION_CODE(void BOARD_InitFlash(FLEXSPI_Type *base));
void EdgeLock_SetClock(uint8_t mux, uint8_t div);
void DCDC_SetVoltage(uint8_t core, uint8_t targetVoltage);

void BOARD_NETC_Init(void);
#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
