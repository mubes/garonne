MEMORY
{
  /* Define each memory region */
  SPIFI_4MB (rx) : ORIGIN = 0x14000000, LENGTH = 0x400000 /* 4M bytes */
  
  /* Where the text (program) code goes */
  RamLoc128 (rwx) : ORIGIN = 0x10000000, LENGTH = 0x20000 /* 128K bytes */

  /* Where the data goes */
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
/* RUN_FROM_RAM */
        LONG(LOADADDR(.text_ram));
        LONG(    ADDR(.text_ram)) ;
        LONG(  SIZEOF(.text_ram));
/* END RUN_FROM_RAM */
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
        __bss_section_table_end = .;
        __section_table_end = . ;
        /* End of Global Section Table */
        

        *(.after_vectors*)

 /* RUN_FROM_RAM */    
        *startup_lpc43xx.o (.text* .rodata .rodata.* .constdata .constdata.*)
        *fault_handler.o (.text* .rodata .rodata.* .constdata .constdata.*)
        *config.o (.text* .rodata .rodata.* .constdata .constdata.*)
        *llb_init.o (.text* .rodata .rodata.* .constdata .constdata.*)
        *ui.o (.text* .rodata .rodata.* .constdata .constdata.*)
        *(.text.main)         
	*(.text.PRE_SystemInit) 
/* END RUN_FROM_RAM */ 
        
    } >SPIFI_4MB
    
 /* RUN_FROM_RAM */    
    .text_ram : ALIGN(4)
/* END RUN_FROM_RAM */    
     {
        *(.text*)
        *(.rodata .rodata.* .constdata .constdata.*)
        . = ALIGN(4);
/* RUN_FROM_RAM */          
    } > RamLoc128 AT>SPIFI_4MB
/* END RUN_FROM_RAM */

    /*
     * for exception handling/unwind - some Newlib functions (in common
     * with C++ and STDC++) use this. 
     */
    .ARM.extab : ALIGN(4)
    {
    	*(.ARM.extab* .gnu.linkonce.armextab.*)
    
/* RUN_FROM_RAM */  
    } > RamLoc128 AT>SPIFI_4MB
/* END RUN_FROM_RAM */
    
    __exidx_start = .;
    
    .ARM.exidx : ALIGN(4)
    {
    	*(.ARM.exidx* .gnu.linkonce.armexidx.*)
/* RUN_FROM_RAM */  
    } > RamLoc128 AT>SPIFI_4MB
/* END RUN_FROM_RAM */

    __exidx_end = .;
    
    _etext = .;
        
    /* MAIN DATA SECTION */
	/* Main DATA section (RamLoc72) */
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
	} > RamLoc72 AT>SPIFI_4MB
    
    /* DATA section for RamLoc72 */
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
    } > RamLoc72 AT>SPIFI_4MB
    
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
    } > RamAHB32 AT>SPIFI_4MB
    
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
    } > RamAHB16 AT>SPIFI_4MB
    
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
    } > RamAHB_ETB16 AT>SPIFI_4MB

    .uninit_RESERVED : ALIGN(4)
    {
        KEEP(*(.bss.$RESERVED*))
        . = ALIGN(4) ;
        _end_uninit_RESERVED = .;
    } > RamLoc72

    /* BSS section for RamLoc72 */
    .bss_RAM2 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM2 = .) ;
    	*(.bss.$RAM2*)
    	*(.bss.$RamLoc72*)
       . = ALIGN(4) ;
       PROVIDE(__end_bss_RAM2 = .) ;
    } > RamLoc72
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

    /* MAIN BSS SECTION */
    .bss : ALIGN(4)
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4) ;
        _ebss = .;
        PROVIDE(end = .);
    } > RamLoc72
        
    /* NOINIT section for RamLoc72 */
    .noinit_RAM2 (NOLOAD) : ALIGN(4)
    {
    	*(.noinit.$RAM2*)
    	*(.noinit.$RamLoc72*)
       . = ALIGN(4) ;
    } > RamLoc72 
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
    
    /* DEFAULT NOINIT SECTION */
    .noinit (NOLOAD): ALIGN(4)
    {
        _noinit = .;
        *(.noinit*) 
         . = ALIGN(4) ;
        _end_noinit = .;
    } > RamAHB32
    
    PROVIDE(_pvHeapStart = DEFINED(__user_heap_base) ? __user_heap_base : .);
    PROVIDE(_vStackTop = DEFINED(__user_stack_top) ? __user_stack_top : __top_RamLoc128);
}
