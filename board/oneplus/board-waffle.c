#include <board.h>
#include <util.h>
#include <drivers/framework.h>
#include <lib/simplefb.h>

static struct video_info waffle_fb = {
	.format = FB_FORMAT_ARGB8888,
	.width = 1440,
	.height = 3168,
	.stride = 4,
	.scale = 2,
	.address = (void *)0xd5100000
};

static const struct device waffle_devices[] = {
	{ "simplefb", &waffle_fb, "fb"},
};

struct board_data board_ops = {
	.name = "oneplus-waffle",
	.ops = {
	},
	.devices = waffle_devices,
	.num_devices = ARRAY_SIZE(waffle_devices),
	.quirks = 0
};
