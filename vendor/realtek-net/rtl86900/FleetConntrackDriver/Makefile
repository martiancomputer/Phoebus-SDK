include $(APOLLODIR)/Compiler_Flag

## SUB MODULE NAME ##
SUB_MODULE = $(CONFIG_RTK_L34_MODULE_STRING)

## MODULE NAME ##
CORE_MODULE_NAME = fc_$(patsubst "%",%,$(SUB_MODULE))
MGR_MODULE_NAME = fc_mgr
ASSISTANT_MODULE_NAME = fc_assistant
FCTESTING_MODULE_NAME = fc_testing

## MODULE PATH ##
FC_DRVFOLDER = FleetConntrackDriver
FC_COREFOLDER = core
FC_COREFOLDER_OBJ = release_$(patsubst "%",%,$(SUB_MODULE))_obj
FC_SRCFOLDER = src
FC_ASICDRVFOLDER = asicDrv
FC_TESTINGFOLDER = fc_testing
FC_ONEIMAGE_77C = oneimage_8277c
FC_DRVDIR = $(APOLLODIR)/$(FC_DRVFOLDER)
FC_COREDIR = $(APOLLODIR)/$(FC_DRVFOLDER)/$(FC_COREFOLDER)
FC_COREDIR_OBJ = $(APOLLODIR)/$(FC_DRVFOLDER)/$(FC_COREFOLDER_OBJ)
FC_SRCDIR = $(APOLLODIR)/$(FC_DRVFOLDER)/$(FC_SRCFOLDER)
FC_TESTING = $(APOLLODIR)/$(FC_DRVFOLDER)/$(FC_TESTINGFOLDER)
FC_VER_FILE = rtk_fc_ver.h

FC_BUILD_SRC = $(shell test -e $(FC_COREDIR) && echo -n y)
srctree=$(KDIR)

export FC_DRVFOLDER FC_COREFOLDER FC_SRCFOLDER FC_ONEIMAGE_77C FC_DRVDIR FC_COREDIR FC_SRCDIR
#$(info fc cc flags = $(EXTRA_CFLAGS))

## COMPILE FLAGS ##
FC_CFLAGS += -Wall \
		-Werror \
		-O2 \
		$(PON_CFLAGS) \
		-I$(FC_COREDIR) \
		-I$(FC_DRVDIR) \
		-I$(FC_DRVDIR)/include \
		-I$(FC_COREDIR)/$(FC_ASICDRVFOLDER) \
		-I$(APOLLODIR)/nicDriver \
		-I$(APOLLODIR)/sdk/include \
		-I$(APOLLODIR)/sdk/system/include \
		-I$(APOLLODIR)/sdk/src/app/diag_rtk/include \
		-I$(KDIR)/net/bridge \
		-I$(KDIR)/fs/proc \
		-I$(KDIR)/include/soc/cortina \
				
ifeq ($(CONFIG_DEFAULTS_KERNEL_2_6),y)
	FC_CFLAGS += -I$(srctree)/drivers/net
else
	FC_CFLAGS += -I$(srctree)/drivers/net/ethernet/realtek
endif

FC_CFLAGS += -I$(FC_DRVDIR)/rt/include

FC_CFLAGS += -DCONFIG_SDK_KERNEL_LINUX

ifeq ($(CONFIG_FC_RTL9607C_RTL9603CVD_HYBRID), y)
FC_CFLAGS += -DCONFIG_RG_RTL9607C_SERIES
endif
ifeq ($(CONFIG_FC_RTL9607C_SERIES), y)
FC_CFLAGS += -DCONFIG_RG_RTL9607C_SERIES
endif
ifeq ($(CONFIG_FC_RTL9603CVD_SERIES), y)
FC_CFLAGS += -DCONFIG_RG_RTL9603CVD_SERIES
endif


ifeq ($(CONFIG_WFO_OFFLOAD_2G), y)
FC_CFLAGS += -I$(KDIR)/drivers/net/wireless/realtek/g6_wifi_driver/
FC_CFLAGS += -I$(KDIR)/drivers/net/wireless/realtek/wfo_virt/
endif

ifeq ($(CONFIG_RTK_L34_G3_PLATFORM), y)

ifneq ($(CONFIG_RTK_FC_HYBRID_MODULE), y)

FC_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/include
#LINUXINCLUDE += -include $(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/include/generated/ca_ne_autoconf.h

ifeq ($(CONFIG_ARCH_REALTEK_TAURUS), y)
FC_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/aal-77c/include
endif
ifeq ($(CONFIG_ARCH_REALTEK_9607F), y)
FC_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/aal-07f/include
endif
ifeq ($(CONFIG_ARCH_CORTINA_VENUS), y)
FC_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/aal-gen2/include
endif
endif #CONFIG_RTK_FC_HYBRID_MODULE

ifeq ($(CONFIG_ARCH_CORTINA_G3HGU), y)
FC_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/aal/include
endif

ifdef CONFIG_MLO_INTF_NUM
ifneq ($(CONFIG_MLO_INTF_NUM), 0)
FC_CFLAGS += -DCONFIG_RTK_FC_WIFI7_SUPPORT
endif
endif

endif

## VERSION CONTROL ##
ifeq ($(FC_BUILD_SRC),y)

GIT_VER_CTRL = $(shell git --git-dir=$(FC_DRVDIR)/.git --work-tree=$(FC_DRVDIR) remote get-url origin | grep -q fleetConntrack && echo -n y || echo -n n)

ifeq ($(GIT_VER_CTRL),y)
FC_RELEASE_VER := $(shell bash $(FC_COREDIR)/getGitTagNumber.sh $(FC_DRVDIR))
FC_PATCH_VER := $(shell bash $(FC_COREDIR)/getGitPatchNumber.sh $(FC_DRVDIR))
FC_CUR_SHA1 := $(shell git --git-dir=$(FC_DRVDIR)/.git --work-tree=$(FC_DRVDIR) rev-parse HEAD | cut -c1-8)
FC_CUR_SHA1_DATE_RAW := $(shell git --git-dir=$(FC_DRVDIR)/.git --work-tree=$(FC_DRVDIR) log -n1 --format="%at")
FC_CUR_SHA1_DATE := $(shell date -d @$(FC_CUR_SHA1_DATE_RAW) +%Y-%m-%d-%H%M)
FC_CUR_SHA1_AUTHOR := $(shell git --git-dir=$(FC_DRVDIR)/.git --work-tree=$(FC_DRVDIR) log -n1 --format="%an")
FC_VERSION := $(FC_RELEASE_VER).$(FC_PATCH_VER)
FC_COMPILE_BY := $(shell whoami | sed 's/\\/\\\\/')
FC_COMPILE_DATE := $(shell date "+%Y-%m-%d-%H%M")

# update version file
$(shell echo \#define FC_GIT_VERSION "\""$(FC_VERSION)"\"" > $(FC_DRVDIR)/$(FC_VER_FILE))
$(shell echo \#define FC_GIT_SHA1 "\""$(FC_CUR_SHA1)"\"" >> $(FC_DRVDIR)/$(FC_VER_FILE))
$(shell echo \#define FC_GIT_SHA1_DATE "\""$(FC_CUR_SHA1_DATE)"\"" >> $(FC_DRVDIR)/$(FC_VER_FILE))
$(shell echo \#define FC_GIT_AUTHOR "\""$(FC_CUR_SHA1_AUTHOR)"\"" >> $(FC_DRVDIR)/$(FC_VER_FILE))
$(shell echo \#define FC_COMPILE_BY "\""$(FC_COMPILE_BY)"\"" >> $(FC_DRVDIR)/$(FC_VER_FILE))
$(shell echo \#define FC_COMPILE_DATE "\""$(FC_COMPILE_DATE)"\"" >> $(FC_DRVDIR)/$(FC_VER_FILE))

endif #GIT_VER_CTRL

#$(info fc debug Makefile = $(GIT_VER_CTRL) $(FC_CUR_SHA1))
FC_VER_CONTROL = $(shell test -e $(FC_DRVDIR)/$(FC_VER_FILE) && echo -n y)
ifeq ($(FC_VER_CONTROL),y)
FC_CFLAGS += -DFC_VER_CONTROL=$(FC_VER_CONTROL)
endif

FC_CORE_BUILD_FOLDER = $(FC_COREFOLDER)
FC_ASICDRV_BUILD_FOLDER = $(FC_ASICDRVFOLDER)

else #!FC_BUILD_SRC

FC_CORE_BUILD_FOLDER = $(FC_COREFOLDER_OBJ)
FC_ASICDRV_BUILD_FOLDER =

endif #FC_BUILD_SRC

export FC_CFLAGS FC_VERSION
EXTRA_CFLAGS += $(FC_CFLAGS)

#$(info fc cc flags = $(FC_CFLAGS))

## ADDITIONAL CONTROL for ONE IMAGE ##
include $(FC_DRVDIR)/Makefile.oneimg
######################################


## CORE MODULE FILES ##
RTK_FC_OBJ = $(FC_CORE_BUILD_FOLDER)/rtk_fc_driver.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_debug.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_internal.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_external.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_multicast.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_proc.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_mappingAPI.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_callback.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_dualHeader.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_acl.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_mapper_api.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_vxlan.o \
		$(FC_CORE_BUILD_FOLDER)/rtk_fc_ipsec.o \

export RTK_FC_OBJ

ifeq ($(CONFIG_RTK_FC_HYBRID_MODULE), y)	

else
	
ifeq ($(CONFIG_RTL9607C_SERIES), y)
RTK_FC_ASIC_OBJ += $(FC_CORE_BUILD_FOLDER)/$(FC_ASICDRV_BUILD_FOLDER)/rtk_rg_apolloPro_asicDriver.o \
                $(FC_CORE_BUILD_FOLDER)/$(FC_ASICDRV_BUILD_FOLDER)/rtk_rg_apolloPro_internal.o
else ifeq ($(CONFIG_RTL9603CVD_SERIES), y)
RTK_FC_ASIC_OBJ += $(FC_CORE_BUILD_FOLDER)/$(FC_ASICDRV_BUILD_FOLDER)/rtk_rg_rtl9603cvd_asicDriver.o \
                $(FC_CORE_BUILD_FOLDER)/$(FC_ASICDRV_BUILD_FOLDER)/rtk_rg_apolloPro_internal.o
else ifeq ($(CONFIG_FC_RTL8277C_SERIES), y)
RTK_FC_ASIC_OBJ += $(FC_CORE_BUILD_FOLDER)/$(FC_ASICDRV_BUILD_FOLDER)/rtk_8277c_asicDriver.o
else ifeq ($(CONFIG_FC_RTL9607F_SERIES), y)
RTK_FC_ASIC_OBJ += $(FC_CORE_BUILD_FOLDER)/$(FC_ASICDRV_BUILD_FOLDER)/rtk_9607f_asicDriver.o
else ifeq ($(CONFIG_FC_RTL8277C_RTL9607F_HYBRID), y)
RTK_FC_ASIC_OBJ += $(FC_CORE_BUILD_FOLDER)/$(FC_ASICDRV_BUILD_FOLDER)/rtk_9607f_asicDriver.o
else ifeq ($(CONFIG_RTK_L34_G3_PLATFORM), y)
RTK_FC_ASIC_OBJ += $(FC_CORE_BUILD_FOLDER)/$(FC_ASICDRV_BUILD_FOLDER)/rtk_rg_g3_internal.o \
		$(FC_CORE_BUILD_FOLDER)/$(FC_ASICDRV_BUILD_FOLDER)/rtk_rg_ca8277_asicDriver.o				
endif

endif #CONFIG_RTK_FC_HYBRID_MODULE

## MGR MODULE FILES ##
RTK_FC_MGR_OBJ = src/rtk_fc_mgr.o \
		src/rtk_fc_mgrTRx.o \
		src/rtk_fc_helper.o \
		src/rtk_fc_helper_wlan.o \
		src/rtk_fc_helper_ps.o \
		src/rtk_fc_helper_rt_api.o \
		src/rtk_fc_helper_common.o \
		src/rtk_fc_helper_multicast.o

export RTK_FC_MGR_OBJ


#####################################
## FC ASSISTANT MODULE            ###
#####################################
ASSISTANT_MODULE_MODE=y

RTK_FC_ASSISTANT_OBJ = src/rtk_fc_assistant.o
obj-$(ASSISTANT_MODULE_MODE) += $(ASSISTANT_MODULE_NAME).o
$(ASSISTANT_MODULE_NAME)-objs := $(RTK_FC_ASSISTANT_OBJ)

#####################################
### FC MANAGER MODULE             ###
#####################################

$(shell touch $(FC_COREDIR)/rtk_fc_driver.c)

obj-$(CONFIG_RTK_L34_FC_KERNEL_MODULE) += $(MGR_MODULE_NAME).o 
$(MGR_MODULE_NAME)-objs := $(RTK_FC_MGR_OBJ) $(RTK_FC_OBJ) $(RTK_FC_ASIC_OBJ)
                                
####################################
### FC DUAL MODULE SUPPORT       ###
####################################
ifeq ($(CONFIG_FC_CA8277B_RTL8277C_HYBRID), y)
obj-$(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)+= $(FC_ONEIMAGE_77C)/
endif
ifeq ($(CONFIG_FC_RTL8277C_RTL9607F_HYBRID), y)
obj-$(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)+= $(FC_ONEIMAGE_77C)/
endif

ifeq ($(CONFIG_RTK_FC_TESTING),y)
include  $(FC_TESTING)/Makefile
endif
