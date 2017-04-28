MEMORY
{
  /* Define each memory region */
  MFlash   (rx) : ORIGIN = 0x0, LENGTH = 0xC000 /* 48K bytes */
  RamLoc8 (rwx) : ORIGIN = 0x10000000, LENGTH = 0x2000 /* 8K bytes */
  RamUSB  (rwx) : ORIGIN = 0x20004000, LENGTH = 0x0400 /* 1K bytes */
  RamDisp (rwx) : ORIGIN = 0x20004400, LENGTH = 0x0400 /* 1K bytes */


}
  /* Define a symbol for the top of each memory region */
  __top_MFlash  = 0x0 + 0xC000;
  __top_RamLoc8 = 0x10000000 + 0x2000;
  __top_RamUSB  = 0x20004000 + 0x800;

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
        __data_section_table_end = .;
        __bss_section_table = .;
        LONG(    ADDR(.bss));
        LONG(  SIZEOF(.bss));
        __bss_section_table_end = .;
        __section_table_end = . ;
        /* End of Global Section Table */
        

        *(.after_vectors*)
        
        /* Code Read Protect data */
        . = 0x000002FC ;
        PROVIDE(__CRP_WORD_START__ = .) ;
        KEEP(*(.crp))
        PROVIDE(__CRP_WORD_END__ = .) ;
        ASSERT(!(__CRP_WORD_START__ == __CRP_WORD_END__), "Linker CRP Enabled, but no CRP_WORD provided within application");
        /* End of Code Read Protect */
        
    } >MFlash
    
    .dispBuffer (NOLOAD):
    {
	*(.dispBufer)
	} > RamDisp

    .text : ALIGN(4)    
    {
         *(.text*)
        *(.rodata .rodata.* .constdata .constdata.*)
        . = ALIGN(4);
        
            } > MFlash

    /*
     * for exception handling/unwind - some Newlib functions (in common
     * with C++ and STDC++) use this. 
     */
    .ARM.extab : ALIGN(4)
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > MFlash
    __exidx_start = .;
    
    .ARM.exidx : ALIGN(4)
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > MFlash
    __exidx_end = .;
    
    _etext = .;
        
    
    /* MAIN DATA SECTION */
    
       .uninit_RESERVED : ALIGN(4)
    {
        KEEP(*(.bss.$RESERVED*))
        . = ALIGN(4) ;
        _end_uninit_RESERVED = .;
    } > RamLoc8

        
        /* Main DATA section (RamLoc8) */
        .data : ALIGN(4)
        {
           FILL(0xff)
           _data = . ;
           *(vtable)
           *(.ramfunc*)
           *(.data*)
           . = ALIGN(4) ;
           _edata = . ;
        } > RamLoc8 AT>MFlash
      /* MAIN BSS SECTION */
    .bss : ALIGN(4)
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4) ;
        _ebss = .;
        PROVIDE(end = .);
    } > RamLoc8
        
    
    /* DEFAULT NOINIT SECTION */
    .noinit (NOLOAD): ALIGN(4)
    {
        _noinit = .;
        *(.noinit*) 
         . = ALIGN(4) ;
        _end_noinit = .;
    } > RamLoc8

  .usbeplist 0x02008000 :
  {
        KEEP(*(.usbeplist))
  } > RamUSB

    
       PROVIDE(_pvHeapStart = DEFINED(__user_heap_base) ? __user_heap_base : .);
    PROVIDE(_vStackTop = DEFINED(__user_stack_top) ? __user_stack_top : __top_RamLoc8 - 32);
}

    
