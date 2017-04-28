MEMORY
{
  /* THIS IS THE M0 MEMORY MAP - BEWARE, IT IS DIFFERENT TO THE M4 */
  /* SPECIFICALLY, IT IS COMPLETELY RAM BASED AND EVERYTHING GOES INTO */
  /* THE 16K AT RamAHB16.  THE COPY FROM SPIFI TO RAM IS PERFORMED BY */
  /* THE M4 ON BOOT AND IS OPAQUE TO THE M0s */

  /* Define each memory region */
  SPIFI_4MB (rx) : ORIGIN = 0x14000000, LENGTH = 0x400000 /* 4M bytes */
  
  /* Where the text (program) code goes for the M4 */
  RamLoc128 (rwx) : ORIGIN = 0x10000000, LENGTH = 0x20000 /* 128K bytes */

  /* Where the data goes for the M4 */
  RamLoc72 (rwx) : ORIGIN = 0x10080000, LENGTH = 0x12000 /* 72K bytes */

  RamAHB32 (rwx) : ORIGIN = 0x20000000, LENGTH = 0x8000 /* 32K bytes */

  /* Reserve for M0 APP */
  RamAHB16 (rwx) : ORIGIN = 0x20008000, LENGTH = 0x4000 /* 16K bytes */
  RamAHB_ETB16 (rwx) : ORIGIN = 0x2000c000, LENGTH = 0x4000 /* 16K bytes */


}
  /* Define a symbol for the top of each memory region */
  __top_SPIFI_4MB = 0x14000000 + 0x400000;
  __top_RamLoc128 = 0x10000000 + 0x20000;
  __top_RamLoc72 = 0x10080000 + 0x12000;
  __top_RamAHB32 = 0x20000000 + 0x8000;
  __top_RamAHB16 = 0x20008000 + 0x4000;
  __top_RamAHB_ETB16 = 0x2000c000 + 0x4000;

ENTRY(ResetISR)

SECTIONS
{
    /* MAIN TEXT SECTION */    
    .text : ALIGN(4)
    {
        FILL(0xff)
        KEEP(*(.isr_vector))

        /* Global Section Table */
        . = ALIGN(4) ;
        __section_table_start = .;
        __data_section_table = .;
        LONG(LOADADDR(.data));
        LONG(    ADDR(.data));
        LONG(  SIZEOF(.data));
        __data_section_table_end = .;
        __bss_section_table = .;
        LONG(    ADDR(.bss));
        LONG(  SIZEOF(.bss));
        __bss_section_table_end = .;
        __section_table_end = . ;
        /* End of Global Section Table */
        

        *(.after_vectors*)
        *(.text*)
        *(.rodata .rodata.* .constdata .constdata.*)
    } > RamAHB16

    /*
     * for exception handling/unwind - some Newlib functions (in common
     * with C++ and STDC++) use this. 
     */
    .ARM.extab : ALIGN(4)
    {
    	*(.ARM.extab* .gnu.linkonce.armextab.*)
    
    } > RamAHB16
    
    __exidx_start = .;
    
    .ARM.exidx : ALIGN(4)
    {
    	*(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > RamAHB16

    __exidx_end = .;
    
    _etext = .;
        
    /* MAIN DATA SECTION */
	/* Main DATA section */
	.data : ALIGN(512)
	{
	   FILL(0xff)
	   _data = . ;
	   *(.nvectors)
	   *(vtable)
	   *(.ramfunc*)
	   *(.data*)
	   . = ALIGN(4) ; 
	   _edata = . ;
	} > RamAHB16
    
    .uninit_RESERVED : ALIGN(4)
    {
        KEEP(*(.bss.$RESERVED*))
        . = ALIGN(4) ;
        _end_uninit_RESERVED = .;
    } > RamAHB16

    /* MAIN BSS SECTION */
    .bss : ALIGN(4)
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4) ;
        _ebss = .;
        PROVIDE(end = .);
    } > RamAHB16
        
    /* DEFAULT NOINIT SECTION */
    .noinit (NOLOAD): ALIGN(4)
    {
        _noinit = .;
        *(.noinit*) 
         . = ALIGN(4) ;
        _end_noinit = .;
    } > RamAHB16
    PROVIDE(_pvHeapStart = DEFINED(__user_heap_base) ? __user_heap_base : .);
    PROVIDE(_vStackTop = DEFINED(__user_stack_top) ? __user_stack_top : __top_RamAHB16);
}
