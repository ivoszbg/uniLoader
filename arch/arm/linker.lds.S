ENTRY(_start);

OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
TARGET(binary)

INPUT(KERNEL_PATH)
INPUT(DTB_PATH)

SECTIONS
{
        . = 0x80008000;
	
        .boot : {
                arch/arm/Start.o
        }

        .dtb ALIGN(0x1000) : {
                dtb = .;
                DTB_PATH
        }

        .kernel ALIGN(0x1000) : {
                kernel = .;
                KERNEL_PATH
        }

        kernel_size = SIZEOF(.kernel);
}
