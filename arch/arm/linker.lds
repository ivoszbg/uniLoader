ENTRY(_start);
OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
TARGET(binary)
INPUT(blob/Image)
INPUT(blob/dtb)
SECTIONS
{
        . = 0x80208000;
        .boot : {
                arch/arm/Start.o
        }
        .dtb ALIGN(0x1000) : {
                dtb = .;
                blob/dtb
        }
        .kernel ALIGN(0x1000) : {
                kernel = .;
                blob/Image
        }
        kernel_size = SIZEOF(.kernel);
}
