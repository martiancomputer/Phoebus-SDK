include $(APOLLODIR)/Compiler_Flag

MODULE_NAME  = rtk_switchdev

ifeq ($(CONFIG_ARCH_REALTEK_TAURUS), y)
TARGET_GEN = 77c
CA_NE_DRVFOLDER = ca-ne
CA_NE_DRVDIR = $(APOLLODIR)/$(CA_NE_DRVFOLDER)
else ifeq ($(CONFIG_ARCH_REALTEK_9607F), y)
TARGET_GEN = 07f
CA_NE_DRVFOLDER = ca-ne
CA_NE_DRVDIR = $(APOLLODIR)/$(CA_NE_DRVFOLDER)
else ifeq ($(CONFIG_ARCH_REALTEK_MERCURY), y)
TARGET_GEN = mercury
CA_NE_DRVFOLDER = ca-ne
CA_NE_DRVDIR = $(APOLLODIR)/$(CA_NE_DRVFOLDER)
else
TARGET_GEN = gen2
CA_NE_DRVFOLDER = ca-ne
CA_NE_DRVDIR = $(APOLLODIR)/$(CA_NE_DRVFOLDER)
endif

RTKSW_DRVFOLDER = SwitchdevDriver
RTKSW_HW_DRVDIR = $(APOLLODIR)/$(RTKSW_DRVFOLDER)/platform/$(TARGET_GEN)/include

EXTRA_CFLAGS += -Wall \
		-Werror \
		-I$(CA_NE_DRVDIR) \
		-I$(APOLLODIR)/sdk/system/include/common \
		-I$(APOLLODIR)/sdk/include/rtk/rt \
 		-I$(RTKSW_HW_DRVDIR) \

obj-$(CONFIG_RTK_SWITCHDEV)		:= $(MODULE_NAME).o

$(MODULE_NAME)-objs := rtk_switchdev_main.o \
					   rtk_switchdev_ethtool.o \
					   rtk_switchdev_hw.o \
					   platform/$(TARGET_GEN)/rtk_switchdev_gmac.o \

obj-m		+= $(MODULE_NAME)_test.o

