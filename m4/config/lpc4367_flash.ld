/*
 * This file was originally generated
 * By LPCXpresso v7.8.0 [Build 426] [2015-05-28]  on Tue Jun 09 22:22:14 BST 2015
 * by the automated build tools.  I have pulled it out and turned it into a
 * standalone linker file which we can edit as needed.

 GROUP(
 libcr_c.a
 libcr_eabihelpers.a
 )
 */

MEMORY
{
  /* Define each memory region */
  Flash_00 (rx) : ORIGIN = 0x1a000000, LENGTH = 0x80000 /* 512K bytes */
  Flash_01 (rx) : ORIGIN = 0x1b000000, LENGTH = 0x80000 /* 512K bytes */
  RamLoc32 (rwx) : ORIGIN = 0x10000000, LENGTH = 0x8000 /* 32K bytes */
  RamLoc40 (rwx) : ORIGIN = 0x10080000, LENGTH = 0xA000 /* 40K bytes */
  RamAHB32 (rwx) : ORIGIN = 0x20000000, LENGTH = 0x8000 /* 32K bytes */
  RamAHB16 (rwx) : ORIGIN = 0x20008000, LENGTH = 0x4000 /* 16K bytes */
  RamAHB_ETB16 (rwx) : ORIGIN = 0x2000c000, LENGTH = 0x4000 /* 16K bytes */
  RamM0Sub16 (rwx) : ORIGIN = 0x18000000, LENGTH = 0x4000 /* 16K bytes */
  RamM0Sub2 (rwx) : ORIGIN = 0x18004000, LENGTH = 0x800 /* 2K bytes */


}
  /* Define a symbol for the top of each memory region */
  __top_Flash_00 = 0x1a000000 + 0x80000;
  __top_Flash_01 = 0x1b000000 + 0x80000;
  __top_RamLoc32 = 0x10000000 + 0x8000;
  __top_RamLoc40 = 0x10080000 + 0xA000;
  __top_RamAHB32 = 0x20000000 + 0x8000;
  __top_RamAHB16 = 0x20008000 + 0x4000;
  __top_RamAHB_ETB16 = 0x2000c000 + 0x4000;
  __top_RamM0Sub16 = 0x18000000 + 0x4000;
  __top_RamM0Sub2 = 0x18004000 + 0x800;

ENTRY(ResetISR)

SECTIONS
{

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
        LONG(LOADADDR(.data_RAM6));
        LONG(    ADDR(.data_RAM6));
        LONG(  SIZEOF(.data_RAM6));
        LONG(LOADADDR(.data_RAM7));
        LONG(    ADDR(.data_RAM7));
        LONG(  SIZEOF(.data_RAM7));
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
        __bss_section_table_end = .;
        __section_table_end = . ;
        /* End of Global Section Table */
        

        *(.after_vectors*)
        
        /* Code Read Protect data 
        . = 0x000002FC ;
        PROVIDE(__CRP_WORD_START__ = .) ;
        KEEP(*(.crp))
        PROVIDE(__CRP_WORD_END__ = .) ;
        ASSERT(!(__CRP_WORD_START__ == __CRP_WORD_END__), "Linker CRP Enabled, but no CRP_WORD provided within application");
	End of Code Read Protect */
        
    } >Flash_00
    
    .text : ALIGN(4)    
    {
         *(.text*)
        *(.rodata .rodata.* .constdata .constdata.*)
        . = ALIGN(4);
        
    } > Flash_00

    /*
     * for exception handling/unwind - some Newlib functions (in common
     * with C++ and STDC++) use this. 
     */
    .ARM.extab : ALIGN(4)
    {
    	*(.ARM.extab* .gnu.linkonce.armextab.*)
    } > Flash_00
    __exidx_start = .;
    
    .ARM.exidx : ALIGN(4)
    {
    	*(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > Flash_00
    __exidx_end = .;
    
    _etext = .;
        
    
    /* DATA section for RamLoc40 */
    .data_RAM2 : ALIGN(4)
    {
       FILL(0xff)
       PROVIDE(__start_data_RAM2 = .) ;
       *(.ramfunc.$RAM2)
       *(.ramfunc.$RamLoc72)
    	*(.data.$RAM2*)
    	*(.data.$RamLoc72*)
       . = ALIGN(4) ;
       PROVIDE(__end_data_RAM2 = .) ;
    } > RamLoc40 AT>Flash_00
    
    /* DATA section for RamAHB32 */
    .data_RAM3 : ALIGN(4)
    {
       FILL(0xff)
       PROVIDE(__start_data_RAM3 = .) ;
       *(.ramfunc.$RAM3)
       *(.ramfunc.$RamAHB32)
    	*(.data.$RAM3*)
    	*(.data.$RamAHB32*)
       . = ALIGN(4) ;
       PROVIDE(__end_data_RAM3 = .) ;
    } > RamAHB32 AT>Flash_00
    
    /* DATA section for RamAHB16 */
    .data_RAM4 : ALIGN(4)
    {
       FILL(0xff)
       PROVIDE(__start_data_RAM4 = .) ;
       *(.ramfunc.$RAM4)
       *(.ramfunc.$RamAHB16)
    	*(.data.$RAM4*)
    	*(.data.$RamAHB16*)
       . = ALIGN(4) ;
       PROVIDE(__end_data_RAM4 = .) ;
    } > RamAHB16 AT>Flash_00
    
    /* DATA section for RamAHB_ETB16 */
    .data_RAM5 : ALIGN(4)
    {
       FILL(0xff)
       PROVIDE(__start_data_RAM5 = .) ;
       *(.ramfunc.$RAM5)
       *(.ramfunc.$RamAHB_ETB16)
    	*(.data.$RAM5*)
    	*(.data.$RamAHB_ETB16*)
       . = ALIGN(4) ;
       PROVIDE(__end_data_RAM5 = .) ;
    } > RamAHB_ETB16 AT>Flash_00
    
    /* DATA section for RamM0Sub16 */
    .data_RAM6 : ALIGN(4)
    {
       FILL(0xff)
       PROVIDE(__start_data_RAM6 = .) ;
       *(.ramfunc.$RAM6)
       *(.ramfunc.$RamM0Sub16)
    	*(.data.$RAM6*)
    	*(.data.$RamM0Sub16*)
       . = ALIGN(4) ;
       PROVIDE(__end_data_RAM6 = .) ;
    } > RamM0Sub16 AT>Flash_00
    
    /* DATA section for RamM0Sub2 */
    .data_RAM7 : ALIGN(4)
    {
       FILL(0xff)
       PROVIDE(__start_data_RAM7 = .) ;
       *(.ramfunc.$RAM7)
       *(.ramfunc.$RamM0Sub2)
    	*(.data.$RAM7*)
    	*(.data.$RamM0Sub2*)
       . = ALIGN(4) ;
       PROVIDE(__end_data_RAM7 = .) ;
    } > RamM0Sub2 AT>Flash_00
    
    /* MAIN DATA SECTION */
    

    .uninit_RESERVED : ALIGN(4)
    {
        KEEP(*(.bss.$RESERVED*))
        . = ALIGN(4) ;
        _end_uninit_RESERVED = .;
    } > RamLoc32

	
	/* Main DATA section (RamLoc32) */
	.data : ALIGN(4)
	{
	   FILL(0xff)
	   _data = . ;
	   *(vtable)
	   *(.ramfunc*)
	   *(.data*)
	   . = ALIGN(4) ;
	   _edata = . ;
	} > RamLoc32 AT>Flash_00

    /* BSS section for RamLoc40 */
    .bss_RAM2 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM2 = .) ;
    	*(.bss.$RAM2*)
    	*(.bss.$RamLoc72*)
       . = ALIGN(4) ;
       PROVIDE(__end_bss_RAM2 = .) ;
    } > RamLoc40
    /* BSS section for RamAHB32 */
    .bss_RAM3 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM3 = .) ;
    	*(.bss.$RAM3*)
    	*(.bss.$RamAHB32*)
       . = ALIGN(4) ;
       PROVIDE(__end_bss_RAM3 = .) ;
    } > RamAHB32
    /* BSS section for RamAHB16 */
    .bss_RAM4 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM4 = .) ;
    	*(.bss.$RAM4*)
    	*(.bss.$RamAHB16*)
       . = ALIGN(4) ;
       PROVIDE(__end_bss_RAM4 = .) ;
    } > RamAHB16
    /* BSS section for RamAHB_ETB16 */
    .bss_RAM5 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM5 = .) ;
    	*(.bss.$RAM5*)
    	*(.bss.$RamAHB_ETB16*)
       . = ALIGN(4) ;
       PROVIDE(__end_bss_RAM5 = .) ;
    } > RamAHB_ETB16
    /* BSS section for RamM0Sub16 */
    .bss_RAM6 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM6 = .) ;
    	*(.bss.$RAM6*)
    	*(.bss.$RamM0Sub16*)
       . = ALIGN(4) ;
       PROVIDE(__end_bss_RAM6 = .) ;
    } > RamM0Sub16
    /* BSS section for RamM0Sub2 */
    .bss_RAM7 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM7 = .) ;
    	*(.bss.$RAM7*)
    	*(.bss.$RamM0Sub2*)
       . = ALIGN(4) ;
       PROVIDE(__end_bss_RAM7 = .) ;
    } > RamM0Sub2

    /* MAIN BSS SECTION */
    .bss : ALIGN(4)
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4) ;
        _ebss = .;
        PROVIDE(end = .);
    } > RamLoc32
        
    /* NOINIT section for RamLoc72 */
    .noinit_RAM2 (NOLOAD) : ALIGN(4)
    {
    	*(.noinit.$RAM2*)
    	*(.noinit.$RamLoc40*)
       . = ALIGN(4) ;
    } > RamLoc40
    /* NOINIT section for RamAHB32 */
    .noinit_RAM3 (NOLOAD) : ALIGN(4)
    {
    	*(.noinit.$RAM3*)
    	*(.noinit.$RamAHB32*)
       . = ALIGN(4) ;
    } > RamAHB32 
    /* NOINIT section for RamAHB16 */
    .noinit_RAM4 (NOLOAD) : ALIGN(4)
    {
    	*(.noinit.$RAM4*)
    	*(.noinit.$RamAHB16*)
       . = ALIGN(4) ;
    } > RamAHB16 
    /* NOINIT section for RamAHB_ETB16 */
    .noinit_RAM5 (NOLOAD) : ALIGN(4)
    {
    	*(.noinit.$RAM5*)
    	*(.noinit.$RamAHB_ETB16*)
       . = ALIGN(4) ;
    } > RamAHB_ETB16 
    /* NOINIT section for RamM0Sub16 */
    .noinit_RAM6 (NOLOAD) : ALIGN(4)
    {
    	*(.noinit.$RAM6*)
    	*(.noinit.$RamM0Sub16*)
       . = ALIGN(4) ;
    } > RamM0Sub16 
    /* NOINIT section for RamM0Sub2 */
    .noinit_RAM7 (NOLOAD) : ALIGN(4)
    {
    	*(.noinit.$RAM7*)
    	*(.noinit.$RamM0Sub2*)
       . = ALIGN(4) ;
    } > RamM0Sub2 
    
    /* DEFAULT NOINIT SECTION */
    .noinit (NOLOAD): ALIGN(4)
    {
        _noinit = .;
        *(.noinit*) 
         . = ALIGN(4) ;
        _end_noinit = .;
    } > RamLoc32
    
    PROVIDE(_pvHeapStart = DEFINED(__user_heap_base) ? __user_heap_base : .);
    PROVIDE(_vStackTop = DEFINED(__user_stack_top) ? __user_stack_top : __top_RamLoc40 - 0);
}
