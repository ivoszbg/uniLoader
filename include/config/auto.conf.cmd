deps_config := \
	board/Kconfig \
	soc/Kconfig \
	lib/Kconfig \
	Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
