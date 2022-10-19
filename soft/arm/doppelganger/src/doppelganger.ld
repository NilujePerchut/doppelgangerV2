MEMORY {
    rom (rx) : ORIGIN = 0x00000000, LENGTH = 1024K
    ram (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

INCLUDE cortex-m-generic.ld

