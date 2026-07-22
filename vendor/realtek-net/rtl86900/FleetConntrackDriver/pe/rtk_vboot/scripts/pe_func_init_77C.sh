#!/bin/sh

pe0_default_func=`mib get PE0_DEFAULT_FUNCTION | grep PE0_DEFAULT_FUNCTION`
pe1_default_func=`mib get PE1_DEFAULT_FUNCTION | grep PE1_DEFAULT_FUNCTION`

if [ "$pe0_default_func" != "PE0_DEFAULT_FUNCTION=-1" ]; then
	devmem 0xf4320004 32 0x00000020
	if [ "$pe0_default_func" == "PE0_DEFAULT_FUNCTION=0" ] && [ -f "/bin/PE_images/zephyr_77C_pe0_TXAMSDU.bin" ]; then
		echo 0 0 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE0 runs WIFI TX AMSDU function \033[0m"
		/bin/vboot -c 0 -f /bin/PE_images/zephyr_77C_pe0_TXAMSDU.bin -a 0x3000000 -s 0x80000 -d
	elif [ "$pe0_default_func" == "PE0_DEFAULT_FUNCTION=1" ] && [ -f "/bin/PE_images/zephyr_77C_pe0_DEFAULT.bin" ]; then
		echo 1 0 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE0 runs HTTP function \033[0m"
		/bin/vboot -c 0 -f /bin/PE_images/zephyr_77C_pe0_DEFAULT.bin -a 0x3000000 -s 0x80000 -d
	elif [ "$pe0_default_func" == "PE0_DEFAULT_FUNCTION=2" ] && [ -f "/bin/PE_images/zephyr_77C_pe0_DEFAULT.bin" ]; then
		echo 2 0 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE0 runs CRYPTO function \033[0m"
		/bin/vboot -c 0 -f /bin/PE_images/zephyr_77C_pe0_DEFAULT.bin -a 0x3000000 -s 0x80000 -d
		echo 1 > /proc/fc/ctrl/ipsec_en_shortCut
		sleep 1
		echo 1 > /proc/fc/ctrl/ipsec_en_pe_offload
	elif [ "$pe0_default_func" == "PE0_DEFAULT_FUNCTION=4" ] && [ -f "/bin/PE_images/zephyr_77C_pe0_DEFAULT.bin" ]; then
		echo 4 0 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE0 runs SRv6 function \033[0m"
		/bin/vboot -c 0 -f /bin/PE_images/zephyr_77C_pe0_DEFAULT.bin -a 0x3000000 -s 0x80000 -d
		sleep 1
		echo 1 > /proc/fc/ctrl/pe_srv6_offload
	elif [ "$pe0_default_func" == "PE0_DEFAULT_FUNCTION=5" ] && [ -f "/bin/PE_images/zephyr_77C_pe0_DEFAULT.bin" ]; then
		echo 5 0 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE0 runs SW Queue function \033[0m"
		/bin/vboot -c 0 -f /bin/PE_images/zephyr_77C_pe0_DEFAULT.bin -a 0x3000000 -s 0x80000 -d
	elif [ "$pe0_default_func" == "PE0_DEFAULT_FUNCTION=6" ] && [ -f "/bin/PE_images/zephyr_77C_pe0_DEFAULT.bin" ]; then
		echo 6 0 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE0 runs SW TC Queue 0 function \033[0m"
		/bin/vboot -c 0 -f /bin/PE_images/zephyr_77C_pe0_DEFAULT.bin -a 0x3000000 -s 0x80000 -d
	elif [ "$pe0_default_func" == "PE0_DEFAULT_FUNCTION=7" ] && [ -f "/bin/PE_images/zephyr_77C_pe0_DEFAULT.bin" ]; then
		echo 7 0 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE0 runs SW TC Queue 1 function \033[0m"
		/bin/vboot -c 0 -f /bin/PE_images/zephyr_77C_pe0_DEFAULT.bin -a 0x3000000 -s 0x80000 -d
	else
		echo -e "\033[1;33;41m[WARNING] $pe0_default_func is unknown !! \033[0m"
	fi
fi
	
if [ "$pe1_default_func" != "PE1_DEFAULT_FUNCTION=-1" ]; then
	devmem 0xf4320004 32 0x00000020
	if [ "$pe1_default_func" == "PE1_DEFAULT_FUNCTION=0" ] && [ -f "/bin/PE_images/zephyr_77C_pe1_TXAMSDU.bin" ]; then
		echo 0 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE1 runs WIFI TX AMSDU function \033[0m"
		/bin/vboot -c 1 -f /bin/PE_images/zephyr_77C_pe1_TXAMSDU.bin -a 0x3100000 -s 0x80000 -d
	elif [ "$pe1_default_func" == "PE1_DEFAULT_FUNCTION=1" ] && [ -f "/bin/PE_images/zephyr_77C_pe1_DEFAULT.bin" ]; then
		echo 1 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE1 runs HTTP function \033[0m"
		/bin/vboot -c 1 -f /bin/PE_images/zephyr_77C_pe1_DEFAULT.bin -a 0x3100000 -s 0x80000 -d
	elif [ "$pe1_default_func" == "PE1_DEFAULT_FUNCTION=2" ] && [ -f "/bin/PE_images/zephyr_77C_pe1_DEFAULT.bin" ]; then
		echo 2 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE1 runs CRYPTO function \033[0m"
		/bin/vboot -c 1 -f /bin/PE_images/zephyr_77C_pe1_DEFAULT.bin -a 0x3100000 -s 0x80000 -d
		echo 1 > /proc/fc/ctrl/ipsec_en_shortCut
		sleep 1
		echo 1 > /proc/fc/ctrl/ipsec_en_pe_offload
	elif [ "$pe1_default_func" == "PE1_DEFAULT_FUNCTION=4" ] && [ -f "/bin/PE_images/zephyr_77C_pe1_DEFAULT.bin" ]; then
		echo 4 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE1 runs SRv6 function \033[0m"
		/bin/vboot -c 1 -f /bin/PE_images/zephyr_77C_pe1_DEFAULT.bin -a 0x3100000 -s 0x80000 -d
		sleep 1
		echo 1 > /proc/fc/ctrl/pe_srv6_offload
	elif [ "$pe1_default_func" == "PE1_DEFAULT_FUNCTION=5" ] && [ -f "/bin/PE_images/zephyr_77C_pe1_DEFAULT.bin" ]; then
		echo 5 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE1 runs SW Queue function \033[0m"
		/bin/vboot -c 1 -f /bin/PE_images/zephyr_77C_pe1_DEFAULT.bin -a 0x3100000 -s 0x80000 -d
	elif [ "$pe1_default_func" == "PE1_DEFAULT_FUNCTION=6" ] && [ -f "/bin/PE_images/zephyr_77C_pe1_DEFAULT.bin" ]; then
		echo 6 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE1 runs SW TC Queue 0 function \033[0m"
		/bin/vboot -c 1 -f /bin/PE_images/zephyr_77C_pe1_DEFAULT.bin -a 0x3100000 -s 0x80000 -d
	elif [ "$pe1_default_func" == "PE1_DEFAULT_FUNCTION=7" ] && [ -f "/bin/PE_images/zephyr_77C_pe1_DEFAULT.bin" ]; then
		echo 7 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE1 runs SW TC Queue 1 function \033[0m"
		/bin/vboot -c 1 -f /bin/PE_images/zephyr_77C_pe1_DEFAULT.bin -a 0x3100000 -s 0x80000 -d	
	else
		echo -e "\033[1;33;41m[WARNING] $pe1_default_func is unknown !! \033[0m"
	fi	
fi
