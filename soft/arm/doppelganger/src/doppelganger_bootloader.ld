MEMORY {
    rom (rx) : ORIGIN = 0x08010000, LENGTH = 960K
    ram (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

INCLUDE cortex-m-generic.ld

