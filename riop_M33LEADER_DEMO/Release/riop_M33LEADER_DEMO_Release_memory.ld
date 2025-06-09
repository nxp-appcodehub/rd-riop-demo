/*
 * GENERATED FILE - DO NOT EDIT
 * Copyright 2008-2013 Code Red Technologies Ltd,
 * Copyright 2013-2025 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 * Generated linker script file for MIMXRT1189xxxxx
 * Created from memory.ldt by FMCreateLinkMemory
 * Using Freemarker v2.3.30
 * MCUXpresso IDE v24.12 [Build 148] [2025-01-10] on May 19, 2025, 12:45:07 PM
 */

MEMORY
{
  /* Define each memory region */
  BOARD_FLASH (rx) : ORIGIN = 0x28000000, LENGTH = 0x1000000 /* 16M bytes (alias Flash) */  
  SRAM_DTC_cm33 (rwx) : ORIGIN = 0x20000000, LENGTH = 0x20000 /* 128K bytes (alias RAM) */  
  SRAM_ITC_cm33 (rwx) : ORIGIN = 0xffe0000, LENGTH = 0x20000 /* 128K bytes (alias RAM2) */  
  SRAM_OC1_1H (rwx) : ORIGIN = 0x20484000, LENGTH = 0x1c000 /* 112K bytes (alias RAM3) */  
  NCACHE_REGION (rwx) : ORIGIN = 0x204a0000, LENGTH = 0x20000 /* 128K bytes (alias RAM4) */  
  SHMEM_REGION (rwx) : ORIGIN = 0x20500000, LENGTH = 0x40000 /* 256K bytes (alias RAM5) */  
  M7_ITCM_ALIAS (rwx) : ORIGIN = 0x203c0000, LENGTH = 0x40000 /* 256K bytes (alias RAM6) */  
  BOARD_SDRAM (rwx) : ORIGIN = 0x80000000, LENGTH = 0x2000000 /* 32M bytes (alias RAM7) */  
  BOARD_HYPERRAM (rwx) : ORIGIN = 0x4000000, LENGTH = 0x800000 /* 8M bytes (alias RAM8) */  
}

  /* Define a symbol for the top of each memory region */
  __base_BOARD_FLASH = 0x28000000  ; /* BOARD_FLASH */  
  __base_Flash = 0x28000000 ; /* Flash */  
  __top_BOARD_FLASH = 0x28000000 + 0x1000000 ; /* 16M bytes */  
  __top_Flash = 0x28000000 + 0x1000000 ; /* 16M bytes */  
  __base_SRAM_DTC_cm33 = 0x20000000  ; /* SRAM_DTC_cm33 */  
  __base_RAM = 0x20000000 ; /* RAM */  
  __top_SRAM_DTC_cm33 = 0x20000000 + 0x20000 ; /* 128K bytes */  
  __top_RAM = 0x20000000 + 0x20000 ; /* 128K bytes */  
  __base_SRAM_ITC_cm33 = 0xffe0000  ; /* SRAM_ITC_cm33 */  
  __base_RAM2 = 0xffe0000 ; /* RAM2 */  
  __top_SRAM_ITC_cm33 = 0xffe0000 + 0x20000 ; /* 128K bytes */  
  __top_RAM2 = 0xffe0000 + 0x20000 ; /* 128K bytes */  
  __base_SRAM_OC1_1H = 0x20484000  ; /* SRAM_OC1_1H */  
  __base_RAM3 = 0x20484000 ; /* RAM3 */  
  __top_SRAM_OC1_1H = 0x20484000 + 0x1c000 ; /* 112K bytes */  
  __top_RAM3 = 0x20484000 + 0x1c000 ; /* 112K bytes */  
  __base_NCACHE_REGION = 0x204a0000  ; /* NCACHE_REGION */  
  __base_RAM4 = 0x204a0000 ; /* RAM4 */  
  __top_NCACHE_REGION = 0x204a0000 + 0x20000 ; /* 128K bytes */  
  __top_RAM4 = 0x204a0000 + 0x20000 ; /* 128K bytes */  
  __base_SHMEM_REGION = 0x20500000  ; /* SHMEM_REGION */  
  __base_RAM5 = 0x20500000 ; /* RAM5 */  
  __top_SHMEM_REGION = 0x20500000 + 0x40000 ; /* 256K bytes */  
  __top_RAM5 = 0x20500000 + 0x40000 ; /* 256K bytes */  
  __base_M7_ITCM_ALIAS = 0x203c0000  ; /* M7_ITCM_ALIAS */  
  __base_RAM6 = 0x203c0000 ; /* RAM6 */  
  __top_M7_ITCM_ALIAS = 0x203c0000 + 0x40000 ; /* 256K bytes */  
  __top_RAM6 = 0x203c0000 + 0x40000 ; /* 256K bytes */  
  __base_BOARD_SDRAM = 0x80000000  ; /* BOARD_SDRAM */  
  __base_RAM7 = 0x80000000 ; /* RAM7 */  
  __top_BOARD_SDRAM = 0x80000000 + 0x2000000 ; /* 32M bytes */  
  __top_RAM7 = 0x80000000 + 0x2000000 ; /* 32M bytes */  
  __base_BOARD_HYPERRAM = 0x4000000  ; /* BOARD_HYPERRAM */  
  __base_RAM8 = 0x4000000 ; /* RAM8 */  
  __top_BOARD_HYPERRAM = 0x4000000 + 0x800000 ; /* 8M bytes */  
  __top_RAM8 = 0x4000000 + 0x800000 ; /* 8M bytes */  
