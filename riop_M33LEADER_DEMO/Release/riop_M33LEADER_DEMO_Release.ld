/*
 * GENERATED FILE - DO NOT EDIT
 * Copyright 2008-2013 Code Red Technologies Ltd,
 * Copyright 2013-2025 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 * Generated linker script file for MIMXRT1189xxxxx
 * Created from linkscript.ldt by FMCreateLinkLibraries
 * Using Freemarker v2.3.30
 * MCUXpresso IDE v24.12 [Build 148] [2025-01-10] on May 19, 2025, 12:45:07 PM
 */

INCLUDE "riop_M33LEADER_DEMO_Release_library.ld"
INCLUDE "riop_M33LEADER_DEMO_Release_memory.ld"

ENTRY(ResetISR)

SECTIONS
{
     /* Boot header - FCB, XMCD, Program Image Container for booting from external flash */
    .boot_hdr : ALIGN(4)
    {
        FILL(0x00)
        __boot_hdr_start__ = ABSOLUTE(.) ;

        /* FCB */
        . = 0x0400 ;
        __boot_hdr_conf__ = ABSOLUTE(.) ;
        KEEP(*(.boot_hdr.conf))

        /* XMCD */
        . = 0x0800 ;
        __boot_hdr_xmcd__ = ABSOLUTE(.) ;
        KEEP(*(.boot_hdr.xmcd_data))

        /* Container */
        . = 0x1000 ;
        __boot_hdr_container__  =  ABSOLUTE(.) ;
        KEEP(*(.boot_hdr.container))

        . = 0xB000 ;
        app_image_offset = (. - __boot_hdr_container__) ;
        __boot_hdr_end__ = ABSOLUTE(.) ;
    } >BOARD_FLASH

/* MAIN TEXT SECTION */
.text : ALIGN(4)
{
	FILL(0xff)
	__vectors_start__ = ABSOLUTE(.) ;
	KEEP(*(.isr_vector))
        /* Global Section Table */
        . = ALIGN(4) ;
        __section_table_start = .;
        __data_section_table = .;
        LONG(LOADADDR(.data));
        LONG(    ADDR(.data));
        LONG(  SIZEOF(.data));
        LONG(LOADADDR(.data_RAM2));
        LONG(    ADDR(.data_RAM2));
        LONG(  SIZEOF(.data_RAM2));
        LONG(LOADADDR(.data_RAM3));
        LONG(    ADDR(.data_RAM3));
        LONG(  SIZEOF(.data_RAM3));
        LONG(LOADADDR(.data_RAM4));
        LONG(    ADDR(.data_RAM4));
        LONG(  SIZEOF(.data_RAM4));
        LONG(LOADADDR(.data_RAM5));
        LONG(    ADDR(.data_RAM5));
        LONG(  SIZEOF(.data_RAM5));
        LONG(LOADADDR(.data_RAM6_core_m7slave_text));
        LONG(    ADDR(.data_RAM6_core_m7slave_text));
        LONG(  SIZEOF(.data_RAM6_core_m7slave_text));
        LONG(LOADADDR(.data_RAM6_core_m7slave_ARM_extab));
        LONG(    ADDR(.data_RAM6_core_m7slave_ARM_extab));
        LONG(  SIZEOF(.data_RAM6_core_m7slave_ARM_extab));
        LONG(LOADADDR(.data_RAM6_core_m7slave_ARM_exidx));
        LONG(    ADDR(.data_RAM6_core_m7slave_ARM_exidx));
        LONG(  SIZEOF(.data_RAM6_core_m7slave_ARM_exidx));
        LONG(LOADADDR(.data_RAM6_core_m7slave_data));
        LONG(    ADDR(.data_RAM6_core_m7slave_data));
        LONG(  SIZEOF(.data_RAM6_core_m7slave_data));
        LONG(LOADADDR(.data_RAM6));
        LONG(    ADDR(.data_RAM6));
        LONG(  SIZEOF(.data_RAM6));
        LONG(LOADADDR(.data_RAM7));
        LONG(    ADDR(.data_RAM7));
        LONG(  SIZEOF(.data_RAM7));
        LONG(LOADADDR(.data_RAM8));
        LONG(    ADDR(.data_RAM8));
        LONG(  SIZEOF(.data_RAM8));
        __data_section_table_end = .;
        __bss_section_table = .;
        LONG(    ADDR(.bss));
        LONG(  SIZEOF(.bss));
        LONG(    ADDR(.bss_RAM2));
        LONG(  SIZEOF(.bss_RAM2));
        LONG(    ADDR(.bss_RAM3));
        LONG(  SIZEOF(.bss_RAM3));
        LONG(    ADDR(.bss_RAM4));
        LONG(  SIZEOF(.bss_RAM4));
        LONG(    ADDR(.bss_RAM5));
        LONG(  SIZEOF(.bss_RAM5));
        LONG(    ADDR(.bss_RAM6));
        LONG(  SIZEOF(.bss_RAM6));
        LONG(    ADDR(.bss_RAM7));
        LONG(  SIZEOF(.bss_RAM7));
        LONG(    ADDR(.bss_RAM8));
        LONG(  SIZEOF(.bss_RAM8));
        __bss_section_table_end = .;
        __section_table_end = . ;
        /* End of Global Section Table */

	*(.after_vectors*)

*(EXCLUDE_FILE(	
	
	*cyclic_task.o*
	*rpmsg_lite*
	*freemaster*
	*rtos*
	*lwip*
	*libcr_eabihelpers.a:*

) *.text*) 

 *(EXCLUDE_FILE( 	
	
	*cyclic_task.o*
	*rpmsg_lite*
	*rtos*
	*lwip*
	*libcr_eabihelpers.a:*
	*freemaster*
	
) *.rodata*) 

*(.constdata .constdata.*)
} > BOARD_FLASH
    /*
     * for exception handling/unwind - some Newlib functions (in common
     * with C++ and STDC++) use this.
     */
    .ARM.extab : ALIGN(4)
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > BOARD_FLASH

    .ARM.exidx : ALIGN(4)
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > BOARD_FLASH
 
    _etext = .;
        

    /* Reserve and place Heap within memory map */
    _HeapSize = 0x9000;
    .heap (NOLOAD) :  ALIGN(4)
    {
        _pvHeapStart = .;
        . += _HeapSize;
        . = ALIGN(4); 
        _pvHeapLimit = .;
    } > SRAM_DTC_cm33
    /* DATA section for SRAM_ITC_cm33 */

    .data_RAM2 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM2 = .) ;
        PROVIDE(__start_data_SRAM_ITC_cm33 = .) ;
        *(.ramfunc.$RAM2)
        *(.ramfunc.$SRAM_ITC_cm33)
       *(CodeQuickAccess)
	*(.data.$RAM2)
	*(.data.$SRAM_ITC_cm33)
	*(.data.$RAM2.*)
	*(.data.$SRAM_ITC_cm33.*)
	
	KEEP(*(CodeQuickAccess))
	
	*cyclic_task.o*(*.text*)
	*rpmsg_lite*(*.text*)
	*rtos*(*.text*)
	*lwip*(*.text*)
	*libcr_eabihelpers.a:*(*.text*)
	*freemaster*(*.text*)
	
	*cyclic_task.o*(*.rodata*)
	*rpmsg_lite*(*.rodata*)
	*rtos*(*.rodata*)
	*lwip*(*.rodata*)
	*libcr_eabihelpers.a:*(*.rodata*)
	*freemaster*(*.rodata*)
	






        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM2 = .) ;
        PROVIDE(__end_data_SRAM_ITC_cm33 = .) ;
     } > SRAM_ITC_cm33 AT>BOARD_FLASH

    /* DATA section for SRAM_OC1_1H */

    .data_RAM3 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM3 = .) ;
        PROVIDE(__start_data_SRAM_OC1_1H = .) ;
        *(.ramfunc.$RAM3)
        *(.ramfunc.$SRAM_OC1_1H)

	*(.data.$RAM3)
	*(.data.$SRAM_OC1_1H)
	*(.data.$RAM3.*)
	*(.data.$SRAM_OC1_1H.*)





        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM3 = .) ;
        PROVIDE(__end_data_SRAM_OC1_1H = .) ;
     } > SRAM_OC1_1H AT>BOARD_FLASH

    /* DATA section for NCACHE_REGION */

    .data_RAM4 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM4 = .) ;
        PROVIDE(__start_data_NCACHE_REGION = .) ;
        *(.ramfunc.$RAM4)
        *(.ramfunc.$NCACHE_REGION)
       *(NonCacheable.init)


	*(.data.$RAM4)
	*(.data.$NCACHE_REGION)
	*(.data.$RAM4.*)
	*(.data.$NCACHE_REGION.*)
	
	*(NonCacheable.init)
	
	




        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM4 = .) ;
        PROVIDE(__end_data_NCACHE_REGION = .) ;
     } > NCACHE_REGION AT>BOARD_FLASH

    /* DATA section for SHMEM_REGION */

    .data_RAM5 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM5 = .) ;
        PROVIDE(__start_data_SHMEM_REGION = .) ;
        *(.ramfunc.$RAM5)
        *(.ramfunc.$SHMEM_REGION)





        *(.data.$RAM5)
        *(.data.$SHMEM_REGION)
        *(.data.$RAM5.*)
        *(.data.$SHMEM_REGION.*)



        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM5 = .) ;
        PROVIDE(__end_data_SHMEM_REGION = .) ;
     } > SHMEM_REGION AT>BOARD_FLASH

    /* DATA section for M7_ITCM_ALIAS */
    .data_RAM6_core_m7slave_text : 
    {
       FILL(0xff)
       PROVIDE(__start_data_RAM6 = .) ;
       __core_m7slave_START__ = .; /* start of slave image */
       KEEP(*(.core_m7slave))
    } > M7_ITCM_ALIAS AT>BOARD_FLASH

    /* M7SLAVE extab and exidx sections */
    .data_RAM6_core_m7slave_ARM_extab :
    {
        FILL(0xff)
        KEEP(*(.core_m7slave.ARM.extab))
    } > M7_ITCM_ALIAS AT>BOARD_FLASH 

    .data_RAM6_core_m7slave_ARM_exidx :
    {
        FILL(0xff)
        KEEP(*(.core_m7slave.ARM.exidx))
    } > M7_ITCM_ALIAS AT>BOARD_FLASH 

    /* M7SLAVE data section */
    .data_RAM6_core_m7slave_data :
    {
        FILL(0xff)
        KEEP(*(.core_m7slave.data_*)) KEEP(*(.core_m7slave.data))
        __core_m7slave_END__ = .; /* end of slave image */

        /* perform some simple sanity checks */
        ASSERT(!(__core_m7slave_START__ == __core_m7slave_END__), "No slave code for _core_m7slave");
        ASSERT(/* Memory ranges seen at same address by the Master and Slave cores */
               (ABSOLUTE(__core_m7slave_START__) == __vectors_start___core_m7slave) ||
               /* Memory ranges seen at different addresses by the Master and Slave cores */
               /* M33 I-TCM <-> M7 alias */
               ((ABSOLUTE(__core_m7slave_START__) >= 0x0FFC0000) && (ABSOLUTE(__core_m7slave_START__) <= 0x0FFFFFFF) && (ABSOLUTE(__core_m7slave_START__) + (0x201C0000 - 0x0FFC0000) == __vectors_start___core_m7slave)) ||
               /* [SEC addr] M33 I-TCM <-> M7 alias */
               ((ABSOLUTE(__core_m7slave_START__) >= 0x1FFC0000) && (ABSOLUTE(__core_m7slave_START__) <= 0x1FFFFFFF) && (ABSOLUTE(__core_m7slave_START__) + (0x201C0000 - 0x1FFC0000) == __vectors_start___core_m7slave)) ||
               /* M33 D-TCM <-> M7 alias */
               ((ABSOLUTE(__core_m7slave_START__) >= 0x20000000) && (ABSOLUTE(__core_m7slave_START__) <= 0x203FFFFF) && (ABSOLUTE(__core_m7slave_START__) + (0x20200000 - 0x20000000) == __vectors_start___core_m7slave)) ||
               /* [SEC addr] M33 D-TCM <-> M7 alias */
               ((ABSOLUTE(__core_m7slave_START__) >= 0x30000000) && (ABSOLUTE(__core_m7slave_START__) <= 0x303FFFFF) && (ABSOLUTE(__core_m7slave_START__) + (0x20200000 - 0x30000000) == __vectors_start___core_m7slave)) ||
               /* M33 alias <-> M7 D-TCM */
               ((ABSOLUTE(__core_m7slave_START__) >= 0x20400000) && (ABSOLUTE(__core_m7slave_START__) <= 0x2047FFFF) && (ABSOLUTE(__core_m7slave_START__) - (0x20400000 - 0x20000000) == __vectors_start___core_m7slave)) ||
               /* [SEC addr] M33 alias <-> M7 D-TCM */
               ((ABSOLUTE(__core_m7slave_START__) >= 0x30400000) && (ABSOLUTE(__core_m7slave_START__) <= 0x3047FFFF) && (ABSOLUTE(__core_m7slave_START__) - (0x30400000 - 0x20000000) == __vectors_start___core_m7slave)) ||
               /* M33 alias <-> M7 I-TCM */
               ((ABSOLUTE(__core_m7slave_START__) >= 0x203C0000) && (ABSOLUTE(__core_m7slave_START__) <= 0x203FFFFF) && (ABSOLUTE(__core_m7slave_START__) - (0x203C0000 - 0x00000000) == __vectors_start___core_m7slave)) ||
               /* [SEC addr] M33 alias <-> M7 I-TCM */
               ((ABSOLUTE(__core_m7slave_START__) >= 0x303C0000) && (ABSOLUTE(__core_m7slave_START__) <= 0x303FFFFF) && (ABSOLUTE(__core_m7slave_START__) - (0x303C0000 - 0x00000000) == __vectors_start___core_m7slave)) ||
               /* M33 alias <-> M7 I-TCM opt */
               ((ABSOLUTE(__core_m7slave_START__) >= 0x20380000) && (ABSOLUTE(__core_m7slave_START__) <= 0x203BFFFF) && (ABSOLUTE(__core_m7slave_START__) - (0x20380000 - 0x00040000) == __vectors_start___core_m7slave)) ||
               /* [SEC addr] M33 alias <-> M7 I-TCM opt */
               ((ABSOLUTE(__core_m7slave_START__) >= 0x30380000) && (ABSOLUTE(__core_m7slave_START__) <= 0x303BFFFF) && (ABSOLUTE(__core_m7slave_START__) - (0x30380000 - 0x00040000) == __vectors_start___core_m7slave))
               , "M7SLAVE execute address differs from address provided in source image");
    } > M7_ITCM_ALIAS AT>BOARD_FLASH 

    .data_RAM6 : ALIGN(4)
    {
        FILL(0xff)
        *(.ramfunc.$RAM6)
        *(.ramfunc.$M7_ITCM_ALIAS)




        *(.data.$RAM6)
        *(.data.$M7_ITCM_ALIAS)
        *(.data.$RAM6.*)
        *(.data.$M7_ITCM_ALIAS.*)




        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM6 = .) ;
        PROVIDE(__end_data_M7_ITCM_ALIAS = .) ;
     } > M7_ITCM_ALIAS AT>BOARD_FLASH

    /* DATA section for BOARD_SDRAM */

    .data_RAM7 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM7 = .) ;
        PROVIDE(__start_data_BOARD_SDRAM = .) ;
        *(.ramfunc.$RAM7)
        *(.ramfunc.$BOARD_SDRAM)






        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM7 = .) ;
        PROVIDE(__end_data_BOARD_SDRAM = .) ;
     } > BOARD_SDRAM AT>BOARD_FLASH

    /* DATA section for BOARD_HYPERRAM */

    .data_RAM8 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM8 = .) ;
        PROVIDE(__start_data_BOARD_HYPERRAM = .) ;
        *(.ramfunc.$RAM8)
        *(.ramfunc.$BOARD_HYPERRAM)






        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM8 = .) ;
        PROVIDE(__end_data_BOARD_HYPERRAM = .) ;
     } > BOARD_HYPERRAM AT>BOARD_FLASH

    /* MAIN DATA SECTION */
    .uninit_RESERVED (NOLOAD) : ALIGN(4)
    {
        _start_uninit_RESERVED = .;
        KEEP(*(.bss.$RESERVED*))
       . = ALIGN(4) ;
        _end_uninit_RESERVED = .;
    } > SRAM_DTC_cm33 AT> SRAM_DTC_cm33

    /* Main DATA section (SRAM_DTC_cm33) */
    .data : ALIGN(4)
	{
		FILL(0xff)
		_data = . ;
		PROVIDE(__start_data_RAM = .) ;
		PROVIDE(__start_data_SRAM_DTC_cm33 = .) ;
		*(vtable)
		*(.ramfunc*)

		KEEP(*(DataQuickAccess))
		*(RamFunction)

       *(.data*)
		. = ALIGN(4) ;
		_edata = . ;
		PROVIDE(__end_data_RAM = .) ;
		PROVIDE(__end_data_SRAM_DTC_cm33 = .) ;
	} > SRAM_DTC_cm33 AT>BOARD_FLASH

    /* BSS section for SRAM_ITC_cm33 */
    .bss_RAM2 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM2 = .) ;
       PROVIDE(__start_bss_SRAM_ITC_cm33 = .) ;
       *(.bss.$RAM2)
       *(.bss.$SRAM_ITC_cm33)
       *(.bss.$RAM2.*)
       *(.bss.$SRAM_ITC_cm33.*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM2 = .) ;
       PROVIDE(__end_bss_SRAM_ITC_cm33 = .) ;
    } > SRAM_ITC_cm33 AT> SRAM_ITC_cm33

    /* BSS section for SRAM_OC1_1H */
    .bss_RAM3 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM3 = .) ;
       PROVIDE(__start_bss_SRAM_OC1_1H = .) ;
       *(.bss.$RAM3)
       *(.bss.$SRAM_OC1_1H)
       *(.bss.$RAM3.*)
       *(.bss.$SRAM_OC1_1H.*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM3 = .) ;
       PROVIDE(__end_bss_SRAM_OC1_1H = .) ;
    } > SRAM_OC1_1H AT> SRAM_OC1_1H

    /* BSS section for NCACHE_REGION */
    .bss_RAM4 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM4 = .) ;
       PROVIDE(__start_bss_NCACHE_REGION = .) ;
       *(NonCacheable)
       *(.bss.$RAM4)
       *(.bss.$NCACHE_REGION)
       *(.bss.$RAM4.*)
       *(.bss.$NCACHE_REGION.*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM4 = .) ;
       PROVIDE(__end_bss_NCACHE_REGION = .) ;
    } > NCACHE_REGION AT> NCACHE_REGION

    /* BSS section for SHMEM_REGION */
    .bss_RAM5 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM5 = .) ;
       PROVIDE(__start_bss_SHMEM_REGION = .) ;
       *(.bss.$RAM5)
       *(.bss.$SHMEM_REGION)
       *(.bss.$RAM5.*)
       *(.bss.$SHMEM_REGION.*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM5 = .) ;
       PROVIDE(__end_bss_SHMEM_REGION = .) ;
    } > SHMEM_REGION AT> SHMEM_REGION

    /* BSS section for M7_ITCM_ALIAS */
    .bss_RAM6 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM6 = .) ;
       PROVIDE(__start_bss_M7_ITCM_ALIAS = .) ;
       *(.bss.$RAM6)
       *(.bss.$M7_ITCM_ALIAS)
       *(.bss.$RAM6.*)
       *(.bss.$M7_ITCM_ALIAS.*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM6 = .) ;
       PROVIDE(__end_bss_M7_ITCM_ALIAS = .) ;
    } > M7_ITCM_ALIAS AT> M7_ITCM_ALIAS

    /* BSS section for BOARD_SDRAM */
    .bss_RAM7 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM7 = .) ;
       PROVIDE(__start_bss_BOARD_SDRAM = .) ;
       *(.bss.$RAM7)
       *(.bss.$BOARD_SDRAM)
       *(.bss.$RAM7.*)
       *(.bss.$BOARD_SDRAM.*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM7 = .) ;
       PROVIDE(__end_bss_BOARD_SDRAM = .) ;
    } > BOARD_SDRAM AT> BOARD_SDRAM

    /* BSS section for BOARD_HYPERRAM */
    .bss_RAM8 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM8 = .) ;
       PROVIDE(__start_bss_BOARD_HYPERRAM = .) ;
       *(.bss.$RAM8)
       *(.bss.$BOARD_HYPERRAM)
       *(.bss.$RAM8.*)
       *(.bss.$BOARD_HYPERRAM.*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM8 = .) ;
       PROVIDE(__end_bss_BOARD_HYPERRAM = .) ;
    } > BOARD_HYPERRAM AT> BOARD_HYPERRAM

    /* MAIN BSS SECTION */
    .bss (NOLOAD) : ALIGN(4)
    {
        _bss = .;
        PROVIDE(__start_bss_RAM = .) ;
        PROVIDE(__start_bss_SRAM_DTC_cm33 = .) ;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4) ;
        _ebss = .;
        PROVIDE(__end_bss_RAM = .) ;
        PROVIDE(__end_bss_SRAM_DTC_cm33 = .) ;
        PROVIDE(end = .);
    } > SRAM_DTC_cm33 AT> SRAM_DTC_cm33

    /* NOINIT section for SRAM_ITC_cm33 */
    .noinit_RAM2 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_noinit_RAM2 = .) ;
       PROVIDE(__start_noinit_SRAM_ITC_cm33 = .) ;
       *(.noinit.$RAM2)
       *(.noinit.$SRAM_ITC_cm33)
       *(.noinit.$RAM2.*)
       *(.noinit.$SRAM_ITC_cm33.*)
       . = ALIGN(4) ;
       PROVIDE(__end_noinit_RAM2 = .) ;
       PROVIDE(__end_noinit_SRAM_ITC_cm33 = .) ;
    } > SRAM_ITC_cm33 AT> SRAM_ITC_cm33

    /* NOINIT section for SRAM_OC1_1H */
    .noinit_RAM3 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_noinit_RAM3 = .) ;
       PROVIDE(__start_noinit_SRAM_OC1_1H = .) ;
       *(.noinit.$RAM3)
       *(.noinit.$SRAM_OC1_1H)
       *(.noinit.$RAM3.*)
       *(.noinit.$SRAM_OC1_1H.*)
       . = ALIGN(4) ;
       PROVIDE(__end_noinit_RAM3 = .) ;
       PROVIDE(__end_noinit_SRAM_OC1_1H = .) ;
    } > SRAM_OC1_1H AT> SRAM_OC1_1H

    /* NOINIT section for NCACHE_REGION */
    .noinit_RAM4 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_noinit_RAM4 = .) ;
       PROVIDE(__start_noinit_NCACHE_REGION = .) ;
       *(.noinit.$RAM4)
       *(.noinit.$NCACHE_REGION)
       *(.noinit.$RAM4.*)
       *(.noinit.$NCACHE_REGION.*)
       . = ALIGN(4) ;
       PROVIDE(__end_noinit_RAM4 = .) ;
       PROVIDE(__end_noinit_NCACHE_REGION = .) ;
    } > NCACHE_REGION AT> NCACHE_REGION

    /* NOINIT section for SHMEM_REGION */
    .noinit_RAM5 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_noinit_RAM5 = .) ;
       PROVIDE(__start_noinit_SHMEM_REGION = .) ;
       *(.noinit.$rpmsg_sh_mem)
       *(.noinit.$RAM5)
       *(.noinit.$SHMEM_REGION)
       *(.noinit.$RAM5.*)
       *(.noinit.$SHMEM_REGION.*)
       . = ALIGN(4) ;
       PROVIDE(__end_noinit_RAM5 = .) ;
       PROVIDE(__end_noinit_SHMEM_REGION = .) ;
    } > SHMEM_REGION AT> SHMEM_REGION

    /* NOINIT section for M7_ITCM_ALIAS */
    .noinit_RAM6 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_noinit_RAM6 = .) ;
       PROVIDE(__start_noinit_M7_ITCM_ALIAS = .) ;
       *(.noinit.$RAM6)
       *(.noinit.$M7_ITCM_ALIAS)
       *(.noinit.$RAM6.*)
       *(.noinit.$M7_ITCM_ALIAS.*)
       . = ALIGN(4) ;
       PROVIDE(__end_noinit_RAM6 = .) ;
       PROVIDE(__end_noinit_M7_ITCM_ALIAS = .) ;
    } > M7_ITCM_ALIAS AT> M7_ITCM_ALIAS

    /* NOINIT section for BOARD_SDRAM */
    .noinit_RAM7 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_noinit_RAM7 = .) ;
       PROVIDE(__start_noinit_BOARD_SDRAM = .) ;
       *(.noinit.$RAM7)
       *(.noinit.$BOARD_SDRAM)
       *(.noinit.$RAM7.*)
       *(.noinit.$BOARD_SDRAM.*)
       . = ALIGN(4) ;
       PROVIDE(__end_noinit_RAM7 = .) ;
       PROVIDE(__end_noinit_BOARD_SDRAM = .) ;
    } > BOARD_SDRAM AT> BOARD_SDRAM

    /* NOINIT section for BOARD_HYPERRAM */
    .noinit_RAM8 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_noinit_RAM8 = .) ;
       PROVIDE(__start_noinit_BOARD_HYPERRAM = .) ;
       *(.noinit.$RAM8)
       *(.noinit.$BOARD_HYPERRAM)
       *(.noinit.$RAM8.*)
       *(.noinit.$BOARD_HYPERRAM.*)
       . = ALIGN(4) ;
       PROVIDE(__end_noinit_RAM8 = .) ;
       PROVIDE(__end_noinit_BOARD_HYPERRAM = .) ;
    } > BOARD_HYPERRAM AT> BOARD_HYPERRAM

    /* DEFAULT NOINIT SECTION */
    .noinit (NOLOAD): ALIGN(4)
    {
        _noinit = .;
        PROVIDE(__start_noinit_RAM = .) ;
        PROVIDE(__start_noinit_SRAM_DTC_cm33 = .) ;
        *(.noinit*)
         . = ALIGN(4) ;
        _end_noinit = .;
       PROVIDE(__end_noinit_RAM = .) ;
       PROVIDE(__end_noinit_SRAM_DTC_cm33 = .) ;        
    } > SRAM_DTC_cm33 AT> SRAM_DTC_cm33

     _StackSize = 0x1000;
     /* Reserve space in memory for Stack */
    .heap2stackfill (NOLOAD) :
    {
        . += _StackSize;
    } > SRAM_DTC_cm33
    /* Locate actual Stack in memory map */
    .stack ORIGIN(SRAM_DTC_cm33) + LENGTH(SRAM_DTC_cm33) - _StackSize - 0 (NOLOAD) :  ALIGN(4)
    {
        _vStackBase = .;
        . = ALIGN(4);
        _vStackTop = . + _StackSize;
    } > SRAM_DTC_cm33

    /* Provide basic symbols giving location and size of main text
     * block, including initial values of RW data sections. Note that
     * these will need extending to give a complete picture with
     * complex images (e.g multiple Flash banks).
     */
    _image_start = LOADADDR(.text);
    _image_end = LOADADDR(.data) + SIZEOF(.data);
    _image_size = _image_end - _image_start;
    /* Provide symbols for MIMXRT1180 parts for boot header generation code
     * to set image to be plain load image or XIP.
     * Config : Plain load image = false
     */
     _image_loadaddr = LOADADDR(.text);
    _boot_loadaddr = ORIGIN(BOARD_FLASH);
    _boot_size = LENGTH(BOARD_FLASH);
}