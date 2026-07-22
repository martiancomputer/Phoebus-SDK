#!/bin/sh

pe0_default_func=`mib get PE0_DEFAULT_FUNCTION | grep PE0_DEFAULT_FUNCTION`
pe1_default_func=`mib get PE1_DEFAULT_FUNCTION | grep PE1_DEFAULT_FUNCTION`

if [ "$pe0_default_func" != "PE0_DEFAULT_FUNCTION=-1" ] && [ "$pe1_default_func" != "PE1_DEFAULT_FUNCTION=-1" ]; then
	echo -e "\033[1;33;41m[WARNING][PE] Can not run pe function both on PE0 and PE1. Please check mib PE0_DEFAULT_FUNCTION and PE1_DEFAULT_FUNCTION \033[0m"
elif [ "$pe0_default_func" != "PE0_DEFAULT_FUNCTION=-1" ]; then
	#devmem 0xf4320004 32 0x00000020
	if [ "$pe0_default_func" == "PE0_DEFAULT_FUNCTION=0" ] && [ -f "/bin/PE_images/zephyr_77B_pe0_TXAMSDU.bin" ]; then
		echo 0 0 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE0 runs WIFI TX AMSDU function \033[0m"
		/bin/vboot -c 0 -f /bin/PE_images/zephyr_77B_pe0_TXAMSDU.bin -a 0x3000000 -s 0x100000 -d
	elif [ "$pe0_default_func" == "PE0_DEFAULT_FUNCTION=1" ] && [ -f "/bin/PE_images/zephyr_77B_pe0_HTTP.bin" ]; then	
		echo 1 0 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE0 runs HTTP function \033[0m"
		/bin/vboot -c 0 -f /bin/PE_images/zephyr_77B_pe0_HTTP.bin -a 0x3000000 -s 0x100000 -d
	elif [ "$pe0_default_func" == "PE0_DEFAULT_FUNCTION=2" ] && [ -f "/bin/PE_images/zephyr_77B_pe0_CRYPTO.bin" ]; then
		echo 2 0 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE0 runs CRYPTO function \033[0m"
		/bin/vboot -c 0 -f /bin/PE_images/zephyr_77B_pe0_CRYPTO.bin -a 0x3000000 -s 0x100000 -d
		echo 1 > /proc/fc/ctrl/ipsec_en_shortCut
		sleep 1
		echo 1 > /proc/fc/ctrl/ipsec_en_pe_offload
	elif [ "$pe0_default_func" == "PE0_DEFAULT_FUNCTION=3" ] && [ -f "/bin/PE_images/zephyr_77B_pe0_WFO.bin" ]; then
		/etc/scripts/rtl_wfo_mon.sh
		echo 3 0 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE0 runs WFO function \033[0m"
		/bin/vboot -c 0 -f /bin/PE_images/zephyr_77B_pe0_WFO.bin -a 0x3000000 -s 0x100000 -d
	else
		echo -e "\033[1;37;45m[PE] PE0_DEFAULT_FUNCTION is unknown !! \033[0m"
	fi	
elif [ "$pe1_default_func" != "PE1_DEFAULT_FUNCTION=-1" ]; then
	#devmem 0xf4320004 32 0x00000020
	if [ "$pe1_default_func" == "PE1_DEFAULT_FUNCTION=0" ] && [ -f "/bin/PE_images/zephyr_77B_pe1_TXAMSDU.bin" ]; then
		echo 0 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE1 runs WIFI TX AMSDU function \033[0m"
		/bin/vboot -c 1 -f /bin/PE_images/zephyr_77B_pe1_TXAMSDU.bin -a 0x3100000 -s 0x100000 -d
	elif [ "$pe1_default_func" == "PE1_DEFAULT_FUNCTION=1" ] && [ -f "/bin/PE_images/zephyr_77B_pe1_HTTP.bin" ]; then
		echo 1 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE1 runs HTTP function \033[0m"
		/bin/vboot -c 1 -f /bin/PE_images/zephyr_77B_pe1_HTTP.bin -a 0x3100000 -s 0x100000 -d
	elif [ "$pe1_default_func" == "PE1_DEFAULT_FUNCTION=2" ] && [ -f "/bin/PE_images/zephyr_77B_pe1_CRYPTO.bin" ]; then
		echo 2 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE1 runs CRYPTO function \033[0m"
		/bin/vboot -c 1 -f /bin/PE_images/zephyr_77B_pe1_CRYPTO.bin -a 0x3100000 -s 0x100000 -d
		echo 1 > /proc/fc/ctrl/ipsec_en_shortCut
		sleep 1
		echo 1 > /proc/fc/ctrl/ipsec_en_pe_offload
	elif [ "$pe1_default_func" == "PE1_DEFAULT_FUNCTION=3" ] && [ -f "/bin/PE_images/zephyr_77B_pe1_WFO.bin" ]; then
		echo 3 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE1 runs WFO function \033[0m"
		/bin/vboot -c 1 -f /bin/PE_images/zephyr_77B_pe1_WFO.bin -a 0x3100000 -s 0x100000 -d
	else
		echo -e "\033[1;37;45m[PE] PE1_DEFAULT_FUNCTION is unknown !! \033[0m"
	fi	
else
	echo -e "\033[1;33;41m[WARNING][PE] There is no any pe function setting. Please check mib PE0_DEFAULT_FUNCTION and PE1_DEFAULT_FUNCTION \033[0m"
fi
