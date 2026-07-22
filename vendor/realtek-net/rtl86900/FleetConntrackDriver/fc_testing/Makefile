####################################
### FC TESTING MODULE            ###
####################################
# basic source files for fc testing

EXTRA_CFLAGS += -D_LITTLE_ENDIAN

EXTRA_CFLAGS  += -I$(FC_TESTING)/pktgen \
				-I$(FC_TESTING)/testcase \
				-I$(FC_TESTING)/utility \
				-I$(FC_TESTING) \
				-I$(KDIR) \
				
RTK_FC_TESTING_OBJ = fc_testing/pktgen/crc32.o \
					fc_testing/pktgen/pktForm.o \
					fc_testing/pktgen/pktGen.o \
					fc_testing/pktgen/protoForm.o \
					fc_testing/pktgen/utility.o \
					fc_testing/utility/dump.o \
					fc_testing/utility/rtl_glue.o \
					fc_testing/utility/rtl_util.o \
					fc_testing/utility/rtl_protocol.o \
					fc_testing/main_test.o \
					fc_testing/l34testmodule.o \
					

RTK_FC_TESTING_OBJ += fc_testing/testcase/testcase_dual_dslite.o \
						fc_testing/testcase/testcase_wfo.o \


export RTK_FC_TESTING_OBJ	

obj-$(CONFIG_RTK_L34_FC_KERNEL_MODULE) += $(FCTESTING_MODULE_NAME).o
$(FCTESTING_MODULE_NAME)-objs := $(RTK_FC_TESTING_OBJ)

