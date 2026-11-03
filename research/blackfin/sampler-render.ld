ENTRY(__start)
SECTIONS {
 .text 0x1000 : { *(.text) }
 .data 0x10000 : { *(.data) *(.bss) }
 .sdram 0x01000000 : { *(.sdram) }
 .pcm 0x04000000 : { *(.pcm) }
 .handles 0x02010000 : { *(.handles) }
 .results 0x03000000 (NOLOAD) : { *(.results) }
}
