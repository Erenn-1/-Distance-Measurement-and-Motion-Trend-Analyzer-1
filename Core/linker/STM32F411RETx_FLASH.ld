/* STM32F411RETx — 512 KB Flash, 128 KB SRAM */
ENTRY(Reset_Handler)

_estack = 0x20020000;   /* Top of SRAM */

MEMORY {
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
    RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS {
    /* Interrupt vector table and code */
    .text : {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } >FLASH

    /* Initialised data: stored in Flash, copied to SRAM at reset */
    _sidata = LOADADDR(.data);
    .data : {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } >RAM AT> FLASH

    /* Zero-initialised data */
    .bss : {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } >RAM

    /* Stack and heap go at the top of SRAM (_estack) */
}
