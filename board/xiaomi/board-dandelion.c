#include <board.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>
#include <lib/debug.h>

#define UART_BASE 0x11002000
#define UART_LSR_BASE	0x14
#define UART_LSR_DR 	0x01 	/* Data ready */
#define UART_LSR_THRE 	0x20 	/* TX holding register empty */

void uart_putc(char ch)
{
	while (!(readl((void *)(UART_BASE + UART_LSR_BASE)) & UART_LSR_THRE))
		;

	writel(ch, (void *)UART_BASE);
}

void uart_puts(const char *s)
{
	while (*s != '\0') {
		uart_putc(*s);
		s++;
	}
}

static struct video_info dandelion_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 736,
	.height = 1600,
	.stride = 4,
	.address = (void *)0x7ec50000 // !!WARNING!! This address is for R firmware vendor only. If you have a Q vendor, the address would be 0x7ef50000.
};

int dandelion_drv(void)
{
	REGISTER_DRIVER("simplefb", simplefb_probe, &dandelion_fb);
	return 0;
}

struct board_data board_ops = {
	.name = "xiaomi-dandelion",
	.ops = {
		.drivers_init = dandelion_drv,
	},
	.quirks = 0
};
