#
# Makefile for the Realtek NIC skb pool mechanism.
#

include $(APOLLODIR)/Compiler_Flag

ccflags-y += -O2

ifeq ($(CONFIG_LUNA_G3_SERIES), y)
ccflags-y += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/include

ifeq ($(CONFIG_ARCH_REALTEK_9607F), y)
ccflags-y += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/aal-07f/include
endif

ifeq ($(CONFIG_ARCH_REALTEK_TAURUS), y)
ccflags-y += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/aal-77c/include
endif

ifeq ($(CONFIG_ARCH_CORTINA_VENUS), y)
ccflags-y += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/aal-gen2/include
endif

ifeq ($(CONFIG_CORTINA_BOARD_G3HGU_ENG), y)
ccflags-y += -I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/aal/include
endif

endif

obj-$(CONFIG_RTL_ETH_RECYCLED_SKB) += re_recycleskb.o
