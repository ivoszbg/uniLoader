ENTRY(_start);

OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
TARGET(binary)

INPUT(KERNEL_PATH)
INPUT(DTB_PATH)

SECTIONS
{
	/* The default TEXT_BASE address is 0x0 if not specified otherwise, so compare it to 0x1 */
        __start_address = (TEXT_BASE > 0x1) ? TEXT_BASE : 0x80008000;
        . = __start_address;
	
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
