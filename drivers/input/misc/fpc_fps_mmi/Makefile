# add -Wall to try to catch everything we can.
EXTRA_CFLAGS += -Wall

ifneq ($(filter m y,$(CONFIG_INPUT_MISC_FPC1020_SAVE_TO_CLASS_DEVICE)),)
	EXTRA_CFLAGS += -DCONFIG_INPUT_MISC_FPC1020_SAVE_TO_CLASS_DEVICE
endif

fpc1020_mmi-y += fpc1020_tee.o

obj-$(CONFIG_INPUT_FPC1020) += fpc1020_mmi.o

