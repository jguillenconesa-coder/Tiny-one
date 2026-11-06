ENTRY(__start)
SECTIONS {
 .text 0x1000 : { *(.text) }
 .data 0x10000 : { *(.data) *(.bss) }
 .sdram 0x01000000 : { *(.sdram) }
 .handles 0x02010000 : { *(.handles) }
 .results 0x03000000 (NOLOAD) : { *(.results) }
 .inputs 0x06000000 : { *(.inputs) }
 .request 0xffa0bd2c : { *(.request) }
}
