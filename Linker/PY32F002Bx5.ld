/******************************************************************************
 * @file     PY32F002Bx5.ld
 * @brief    GNU linker script for PY32F002Bx5 (Cortex-M0+, 24KB Flash, 3KB RAM)
 * @note     Clean rewrite — clear MEMORY/SECTIONS, ASSERT on overflow.
 ******************************************************************************/

/*----------------------------------------------------------------------------
  Memory Configuration
 *----------------------------------------------------------------------------*/
__ROM_BASE   = 0x08000000;
__ROM_SIZE   = 0x00006000;    /* 24 KB Flash */
__RAM_BASE   = 0x20000000;
__RAM_SIZE   = 0x00000C00;    /*  3 KB SRAM  */
__STACK_SIZE = 0x00000200;    /* 512 bytes   */
__HEAP_SIZE  = 0x00000000;    /* No heap     */

MEMORY
{
  FLASH (rx)  : ORIGIN = __ROM_BASE, LENGTH = __ROM_SIZE
  RAM   (rwx) : ORIGIN = __RAM_BASE, LENGTH = __RAM_SIZE
}

ENTRY(Reset_Handler)

/*----------------------------------------------------------------------------
  Sections
 *----------------------------------------------------------------------------*/
SECTIONS
{
  /* ---- Code + read-only data in FLASH ---- */
  .text :
  {
    KEEP(*(.vectors))
    *(.text*)

    KEEP(*(.init))
    KEEP(*(.fini))

    /* C++ constructors / destructors */
    *crtbegin.o(.ctors)
    *crtbegin?.o(.ctors)
    *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
    *(SORT(.ctors.*))
    *(.ctors)

    *crtbegin.o(.dtors)
    *crtbegin?.o(.dtors)
    *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
    *(SORT(.dtors.*))
    *(.dtors)

    *(.rodata*)
    KEEP(*(.eh_frame*))
  } > FLASH

  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > FLASH

  __exidx_start = .;
  .ARM.exidx :
  {
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
  } > FLASH
  __exidx_end = .;

  /* ---- Data initialization tables in FLASH ---- */
  .copy.table :
  {
    . = ALIGN(4);
    __copy_table_start__ = .;
    LONG (__etext)
    LONG (__data_start__)
    LONG ((__data_end__ - __data_start__) / 4)
    __copy_table_end__ = .;
  } > FLASH

  .zero.table :
  {
    . = ALIGN(4);
    __zero_table_start__ = .;
    LONG (__bss_start__)
    LONG ((__bss_end__ - __bss_start__) / 4)
    __zero_table_end__ = .;
  } > FLASH

  /* __etext = end of FLASH-resident data, must be 4-byte aligned */
  __etext = ALIGN(4);

  /* ---- Initialized data in RAM, LMA in FLASH ---- */
  .data : AT (__etext)
  {
    __data_start__ = .;
    *(vtable)
    *(.data)
    *(.data.*)

    . = ALIGN(4);
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP(*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);

    . = ALIGN(4);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array))
    PROVIDE_HIDDEN (__init_array_end = .);

    . = ALIGN(4);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array))
    PROVIDE_HIDDEN (__fini_array_end = .);

    KEEP(*(.jcr*))
    . = ALIGN(4);
    __data_end__ = .;
  } > RAM

  /* ---- Zero-initialized data in RAM ---- */
  .bss :
  {
    . = ALIGN(4);
    __bss_start__ = .;
    *(.bss)
    *(.bss.*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
  } > RAM AT > RAM

  /* ---- Heap (zero size) ---- */
  .heap (COPY) :
  {
    . = ALIGN(8);
    __end__ = .;
    PROVIDE(end = .);
    . = . + __HEAP_SIZE;
    . = ALIGN(8);
    __HeapLimit = .;
  } > RAM

  /* ---- Stack at top of RAM ---- */
  .stack (ORIGIN(RAM) + LENGTH(RAM) - __STACK_SIZE) (COPY) :
  {
    . = ALIGN(8);
    __StackLimit = .;
    . = . + __STACK_SIZE;
    . = ALIGN(8);
    __StackTop = .;
  } > RAM
  PROVIDE(__stack = __StackTop);

  /* ---- Safety check ---- */
  ASSERT(__StackLimit >= __HeapLimit, "RAM overflow: stack overlaps heap")
}
