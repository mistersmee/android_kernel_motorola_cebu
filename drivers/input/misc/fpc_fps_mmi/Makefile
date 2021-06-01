ifneq ($(filter m y,$(CONFIG_INPUT_MISC_FPC1020_SAVE_TO_CLASS_DEVICE)),)
	ccflags-y += -DCONFIG_INPUT_MISC_FPC1020_SAVE_TO_CLASS_DEVICE
endif

obj-$(CONFIG_INPUT_FPC1020) += fpc1020_mmi.o
fpc1020_mmi-y += fpc1020_tee.o


