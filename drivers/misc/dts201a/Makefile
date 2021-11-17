# Each configuration option enables a list of files.
FEATURE_USE_CCI := false

ifeq ($(FEATURE_USE_CCI), true)
HAVE_TECHPACK_CAMERA = $(shell test -d $(TOP)/$(KERNEL_SOURCE_PATH)/techpack/camera && echo 1)

ifeq ($(HAVE_TECHPACK_CAMERA),1)
        KERNEL_CAMERA_DRIVER_PATH ?= $(TOP)/$(KERNEL_SOURCE_PATH)/techpack/camera/drivers
        EXTRA_CFLAGS += -DCONFIG_SPECTRA_CAMERA=1
        EXTRA_CFLAGS += -I$(KERNEL_CAMERA_DRIVER_PATH)/../include
        EXTRA_CFLAGS += -I$(KERNEL_CAMERA_DRIVER_PATH)/../include/uapi
else
        KERNEL_CAMERA_DRIVER_PATH ?= $(TOP)/$(KERNEL_SOURCE_PATH)/drivers/media/platform/msm/camera
endif

EXTRA_CFLAGS += -DCAMERA_CCI
EXTRA_CFLAGS += -I$(KERNEL_CAMERA_DRIVER_PATH)/cam_utils
EXTRA_CFLAGS += -I$(KERNEL_CAMERA_DRIVER_PATH)/cam_cpas/include
EXTRA_CFLAGS += -I$(KERNEL_CAMERA_DRIVER_PATH)/cam_sensor_module/cam_sensor_io
EXTRA_CFLAGS += -I$(KERNEL_CAMERA_DRIVER_PATH)/cam_sensor_module/cam_sensor_utils
EXTRA_CFLAGS += -I$(KERNEL_CAMERA_DRIVER_PATH)/cam_sensor_module/cam_cci
EXTRA_CFLAGS += -I$(KERNEL_CAMERA_DRIVER_PATH)/cam_req_mgr
EXTRA_CFLAGS += -I$(KERNEL_CAMERA_DRIVER_PATH)/cam_smmu
endif

EXTRA_CFLAGS += -Wall
dts201a_mmi-y := dts201a.o dts201a-cci.o
obj-$(CONFIG_DTS201A_MMI) += dts201a_mmi.o


