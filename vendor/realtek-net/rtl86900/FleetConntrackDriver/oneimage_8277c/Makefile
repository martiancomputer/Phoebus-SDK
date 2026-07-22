include $(APOLLODIR)/Compiler_Flag

#$(info fc oneimege flags = $(EXTRA_CFLAGS))

## COMPILE FLAGS ##
EXTRA_CFLAGS += $(FC_CFLAGS)


## ADDITIONAL CONTROL ##

CORE_MODULE_NAME := fc_77c
MGR_MODULE_NAME := fc_mgr_77c

#$(info fc show $(RTK_FC_OBJ))


#####################################
### FC CORE MODULE - build source ###
#####################################

RTK_FC_ASIC_OBJ += $(FC_COREFOLDER)/asicDrv/rtk_8277c_asicDriver.o

#LINUXINCLUDE += -include $(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0-77c/include/generated/ca_ne_autoconf.h
EXTRA_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0-77c/include
EXTRA_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0-77c/aal-77c/include
EXTRA_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0-77c/ni-drv-77c/
EXTRA_CFLAGS += -DCONFIG_ARCH_CORTINA_TAURUS -DCONFIG_RTL8277C_SERIES -DCONFIG_FC_RTL8277C_SERIES -DONE_IMAGE_77C
#EXTRA_CFLAGS:=$(filter-out -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/aal-gen2/include,$(EXTRA_CFLAGS))
# filter out original included path
EXTRA_CFLAGS:=$(filter-out -I$(APOLLODIR)/ca-ne/ni-drv-gen2,$(EXTRA_CFLAGS))
EXTRA_CFLAGS:=$(filter-out -I$(APOLLODIR)/ca-ne/ni-drv-77c,$(EXTRA_CFLAGS))
EXTRA_CFLAGS:=$(filter-out -I$(APOLLODIR)/ca-ne/ni-drv-07f,$(EXTRA_CFLAGS))
EXTRA_CFLAGS:=$(filter-out -I$(APOLLODIR)/ca-ne/aal-gen2/include,$(EXTRA_CFLAGS))
EXTRA_CFLAGS:=$(filter-out -I$(APOLLODIR)/ca-ne/aal-77c/include,$(EXTRA_CFLAGS))
EXTRA_CFLAGS:=$(filter-out -I$(APOLLODIR)/ca-ne/aal-07f/include,$(EXTRA_CFLAGS))
# filter out 77B flags
EXTRA_CFLAGS:=$(filter-out -DCONFIG_ARCH_CORTINA_VENUS,$(EXTRA_CFLAGS))
EXTRA_CFLAGS:=$(filter-out -DCONFIG_CA8277B_SERIES,$(EXTRA_CFLAGS))
EXTRA_CFLAGS:=$(filter-out -DCONFIG_FC_CA8277B_SERIES,$(EXTRA_CFLAGS))
EXTRA_CFLAGS:=$(filter-out -DONE_IMAGE_VENUS,$(EXTRA_CFLAGS))
## filter out 07F flags
EXTRA_CFLAGS:=$(filter-out -DCONFIG_ARCH_REALTEK_9607F,$(EXTRA_CFLAGS))
EXTRA_CFLAGS:=$(filter-out -DCONFIG_RTL9607F_SERIES,$(EXTRA_CFLAGS))
EXTRA_CFLAGS:=$(filter-out -DCONFIG_FC_RTL9607F_SERIES,$(EXTRA_CFLAGS))
EXTRA_CFLAGS:=$(filter-out -DONE_IMAGE_07F,$(EXTRA_CFLAGS))

#####################################
### FC MANAGER MODULE             ###
#####################################

obj-$(CONFIG_RTK_L34_FC_KERNEL_MODULE) += $(MGR_MODULE_NAME).o
$(MGR_MODULE_NAME)-objs := $(RTK_FC_MGR_OBJ) $(RTK_FC_OBJ) $(RTK_FC_ASIC_OBJ)
                                

