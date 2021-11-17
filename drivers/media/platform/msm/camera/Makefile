HAVE_TECHPACK_CAMERA = $(shell test -d $(ANDROID_BUILD_TOP)/$(KERNEL_SOURCE_PATH)/techpack/camera && echo 1)

ifeq ($(HAVE_TECHPACK_CAMERA),1)
        KERNEL_CAMERA_DRIVER_PATH ?= $(ANDROID_BUILD_TOP)/$(KERNEL_SOURCE_PATH)/techpack/camera/drivers
        EXTRA_CFLAGS += -DCONFIG_SPECTRA_CAMERA=1
        EXTRA_CFLAGS += -I$(KERNEL_CAMERA_DRIVER_PATH)/../include
        EXTRA_CFLAGS += -I$(KERNEL_CAMERA_DRIVER_PATH)/../include/uapi
        EXTRA_CFLAGS += -I$(KERNEL_CAMERA_DRIVER_PATH)/../include/uapi/camera
else
        KERNEL_CAMERA_DRIVER_PATH ?= $(ANDROID_BUILD_TOP)/$(KERNEL_SOURCE_PATH)/drivers/media/platform/msm/camera
endif

EXTRA_CFLAGS += -Wall
EXTRA_CFLAGS += -I$(KERNEL_CAMERA_DRIVER_PATH)/cam_utils
EXTRA_CFLAGS += -I$(KERNEL_CAMERA_DRIVER_PATH)/cam_cpas/include
EXTRA_CFLAGS += -I$(KERNEL_CAMERA_DRIVER_PATH)/cam_sensor_module/cam_sensor_io
EXTRA_CFLAGS += -I$(KERNEL_CAMERA_DRIVER_PATH)/cam_sensor_module/cam_sensor_utils
EXTRA_CFLAGS += -I$(KERNEL_CAMERA_DRIVER_PATH)/cam_sensor_module/cam_cci
EXTRA_CFLAGS += -I$(KERNEL_CAMERA_DRIVER_PATH)/cam_req_mgr
EXTRA_CFLAGS += -I$(KERNEL_CAMERA_DRIVER_PATH)/cam_smmu

obj-$(CONFIG_CCI_INTF)	+= cci_intf.o
