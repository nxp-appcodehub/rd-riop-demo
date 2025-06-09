/*
 * GENERATED FILE - DO NOT EDIT
 * Copyright 2008-2013 Code Red Technologies Ltd,
 * Copyright 2013-2025 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 * Generated linker script file for MIMXRT1189xxxxx
 * Created from memory.ldt by FMCreateLinkMemory
 * Using Freemarker v2.3.30
 * MCUXpresso IDE v24.12 [Build 148] [2025-01-10] on May 14, 2025, 4:35:52 PM
 */

MEMORY
{
  /* Define each memory region */
  SRAM_ITC_cm7 (rwx) : ORIGIN = 0x0, LENGTH = 0x40000 /* 256K bytes (alias RAM) */  
  SRAM_DTC_cm7 (rwx) : ORIGIN = 0x20000000, LENGTH = 0x40000 /* 256K bytes (alias RAM2) */  
  NCACHE_REGION (rwx) : ORIGIN = 0x204e0000, LENGTH = 0x20000 /* 128K bytes (alias RAM3) */  
  SHMEM_REGION (rwx) : ORIGIN = 0x20500000, LENGTH = 0x40000 /* 256K bytes (alias RAM4) */  
}

  /* Define a symbol for the top of each memory region */
  __base_SRAM_ITC_cm7 = 0x0  ; /* SRAM_ITC_cm7 */  
  __base_RAM = 0x0 ; /* RAM */  
  __top_SRAM_ITC_cm7 = 0x0 + 0x40000 ; /* 256K bytes */  
  __top_RAM = 0x0 + 0x40000 ; /* 256K bytes */  
  __base_SRAM_DTC_cm7 = 0x20000000  ; /* SRAM_DTC_cm7 */  
  __base_RAM2 = 0x20000000 ; /* RAM2 */  
  __top_SRAM_DTC_cm7 = 0x20000000 + 0x40000 ; /* 256K bytes */  
  __top_RAM2 = 0x20000000 + 0x40000 ; /* 256K bytes */  
  __base_NCACHE_REGION = 0x204e0000  ; /* NCACHE_REGION */  
  __base_RAM3 = 0x204e0000 ; /* RAM3 */  
  __top_NCACHE_REGION = 0x204e0000 + 0x20000 ; /* 128K bytes */  
  __top_RAM3 = 0x204e0000 + 0x20000 ; /* 128K bytes */  
  __base_SHMEM_REGION = 0x20500000  ; /* SHMEM_REGION */  
  __base_RAM4 = 0x20500000 ; /* RAM4 */  
  __top_SHMEM_REGION = 0x20500000 + 0x40000 ; /* 256K bytes */  
  __top_RAM4 = 0x20500000 + 0x40000 ; /* 256K bytes */  
