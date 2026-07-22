#!/bin/sh

pe0_default_func=`mib get PE0_DEFAULT_FUNCTION | grep PE0_DEFAULT_FUNCTION`
pe1_default_func=`mib get PE1_DEFAULT_FUNCTION | grep PE1_DEFAULT_FUNCTION`
pe2_default_func=`mib get PE2_DEFAULT_FUNCTION | grep PE2_DEFAULT_FUNCTION`

# assign WFO image path
wfo_fw_path="/bin/PE_images"
if [ -f /config/wfo_fw_path ] && [ ! -z $(cat /config/wfo_fw_path) ]; then
	wfo_fw_path="$(cat /config/wfo_fw_path)"
fi

if [ "$pe0_default_func" != "PE0_DEFAULT_FUNCTION=-1" ]; then
	if [ "$pe0_default_func" == "PE0_DEFAULT_FUNCTION=0" ] && [ -f "/bin/PE_images/zephyr_07F_pe0_TXAMSDU.bin" ]; then
		echo 0 0 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE0 runs WIFI TX AMSDU function \033[0m"
		/bin/vboot -c 0 -f /bin/PE_images/zephyr_07F_pe0_TXAMSDU.bin -a 0x3000000 -s 0x80000 -d
	elif [ "$pe0_default_func" == "PE0_DEFAULT_FUNCTION=1" ] && [ -f "/bin/PE_images/zephyr_07F_pe0_DEFAULT.bin" ]; then
		echo 1 0 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE0 runs HTTP function \033[0m"
		/bin/vboot -c 0 -f /bin/PE_images/zephyr_07F_pe0_DEFAULT.bin -a 0x3000000 -s 0x80000 -d
	elif [ "$pe0_default_func" == "PE0_DEFAULT_FUNCTION=2" ] && [ -f "/bin/PE_images/zephyr_07F_pe0_DEFAULT.bin" ]; then
		echo 2 0 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE0 runs CRYPTO function \033[0m"
		/bin/vboot -c 0 -f /bin/PE_images/zephyr_07F_pe0_DEFAULT.bin -a 0x3000000 -s 0x80000 -d
		echo 1 > /proc/fc/ctrl/ipsec_en_shortCut
		sleep 1
		echo 1 > /proc/fc/ctrl/ipsec_en_pe_offload
	elif [ "$pe0_default_func" == "PE0_DEFAULT_FUNCTION=3" ] && [ -f "${wfo_fw_path}/zephyr_07F_pe0_WFO.bin" ]; then
		/etc/scripts/rtl_wfo_mon.sh
		echo 3 0 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE0 runs WFO function \033[0m"
		/bin/vboot -c 0 -f ${wfo_fw_path}/zephyr_07F_pe0_WFO.bin -a 0x3000000 -s 0x80000 -d
	elif [ "$pe0_default_func" == "PE0_DEFAULT_FUNCTION=4" ] && [ -f "/bin/PE_images/zephyr_07F_pe0_DEFAULT.bin" ]; then
		echo 4 0 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE0 runs SRv6 function \033[0m"
		/bin/vboot -c 0 -f /bin/PE_images/zephyr_07F_pe0_DEFAULT.bin -a 0x3000000 -s 0x80000 -d
		sleep 1
		echo 1 > /proc/fc/ctrl/pe_srv6_offload
	elif [ "$pe0_default_func" == "PE0_DEFAULT_FUNCTION=5" ] && [ -f "/bin/PE_images/zephyr_07F_pe0_DEFAULT.bin" ]; then
		echo 5 0 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE0 runs SW Queue function \033[0m"
		/bin/vboot -c 0 -f /bin/PE_images/zephyr_07F_pe0_DEFAULT.bin -a 0x3000000 -s 0x80000 -d
	elif [ "$pe0_default_func" == "PE0_DEFAULT_FUNCTION=6" ] && [ -f "/bin/PE_images/zephyr_07F_pe0_DEFAULT.bin" ]; then
		echo 6 0 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE0 runs SW TC Queue 0 function \033[0m"
		/bin/vboot -c 0 -f /bin/PE_images/zephyr_07F_pe0_DEFAULT.bin -a 0x3000000 -s 0x80000 -d
	elif [ "$pe0_default_func" == "PE0_DEFAULT_FUNCTION=7" ] && [ -f "/bin/PE_images/zephyr_07F_pe0_DEFAULT.bin" ]; then
		echo 7 0 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE0 runs SW TC Queue 1 function \033[0m"
		/bin/vboot -c 0 -f /bin/PE_images/zephyr_07F_pe0_DEFAULT.bin -a 0x3000000 -s 0x80000 -d	
	else
		echo -e "\033[1;33;41m[WARNING] $pe0_default_func is unknown !! \033[0m"
	fi
fi

if [ "$pe1_default_func" != "PE1_DEFAULT_FUNCTION=-1" ]; then
	if [ "$pe1_default_func" == "PE1_DEFAULT_FUNCTION=0" ] && [ -f "/bin/PE_images/zephyr_07F_pe1_TXAMSDU.bin" ]; then
		echo 0 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE1 runs WIFI TX AMSDU function \033[0m"
		/bin/vboot -c 1 -f /bin/PE_images/zephyr_07F_pe1_TXAMSDU.bin -a 0x3100000 -s 0x80000 -d
	elif [ "$pe1_default_func" == "PE1_DEFAULT_FUNCTION=1" ] && [ -f "/bin/PE_images/zephyr_07F_pe1_DEFAULT.bin" ]; then
		echo 1 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE1 runs HTTP function \033[0m"
		/bin/vboot -c 1 -f /bin/PE_images/zephyr_07F_pe1_DEFAULT.bin -a 0x3100000 -s 0x80000 -d
	elif [ "$pe1_default_func" == "PE1_DEFAULT_FUNCTION=2" ] && [ -f "/bin/PE_images/zephyr_07F_pe1_DEFAULT.bin" ]; then
		echo 2 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE1 runs CRYPTO function \033[0m"
		/bin/vboot -c 1 -f /bin/PE_images/zephyr_07F_pe1_DEFAULT.bin -a 0x3100000 -s 0x80000 -d
		echo 1 > /proc/fc/ctrl/ipsec_en_shortCut
		sleep 1
		echo 1 > /proc/fc/ctrl/ipsec_en_pe_offload
	elif [ "$pe1_default_func" == "PE1_DEFAULT_FUNCTION=3" ] && [ -f "${wfo_fw_path}/zephyr_07F_pe1_WFO.bin" ]; then
		echo 3 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE1 runs WFO function \033[0m"
		/bin/vboot -c 1 -f ${wfo_fw_path}/zephyr_07F_pe1_WFO.bin -a 0x3100000 -s 0x80000 -d
	elif [ "$pe1_default_func" == "PE1_DEFAULT_FUNCTION=4" ] && [ -f "/bin/PE_images/zephyr_07F_pe1_DEFAULT.bin" ]; then
		echo 4 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE1 runs SRv6 function \033[0m"
		/bin/vboot -c 1 -f /bin/PE_images/zephyr_07F_pe1_DEFAULT.bin -a 0x3100000 -s 0x80000 -d
		sleep 1
		echo 1 > /proc/fc/ctrl/pe_srv6_offload
	elif [ "$pe1_default_func" == "PE1_DEFAULT_FUNCTION=5" ] && [ -f "/bin/PE_images/zephyr_07F_pe1_DEFAULT.bin" ]; then
		echo 5 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE1 runs SW Queue function \033[0m"
		/bin/vboot -c 1 -f /bin/PE_images/zephyr_07F_pe1_DEFAULT.bin -a 0x3100000 -s 0x80000 -d	
	elif [ "$pe1_default_func" == "PE1_DEFAULT_FUNCTION=6" ] && [ -f "/bin/PE_images/zephyr_07F_pe1_DEFAULT.bin" ]; then
		echo 6 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE1 runs SW TC Queue 0 function \033[0m"
		/bin/vboot -c 1 -f /bin/PE_images/zephyr_07F_pe1_DEFAULT.bin -a 0x3100000 -s 0x80000 -d
	elif [ "$pe1_default_func" == "PE1_DEFAULT_FUNCTION=7" ] && [ -f "/bin/PE_images/zephyr_07F_pe1_DEFAULT.bin" ]; then
		echo 7 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE1 runs SW TC Queue 1 function \033[0m"
		/bin/vboot -c 1 -f /bin/PE_images/zephyr_07F_pe1_DEFAULT.bin -a 0x3100000 -s 0x80000 -d	
	else
		echo -e "\033[1;33;41m[WARNING] $pe1_default_func is unknown !! \033[0m"
	fi
fi

if [ "$pe2_default_func" != "PE2_DEFAULT_FUNCTION=-1" ]; then
	if [ "$pe2_default_func" == "PE2_DEFAULT_FUNCTION=0" ] && [ -f "/bin/PE_images/zephyr_07F_pe2_TXAMSDU.bin" ]; then
		echo 0 2 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE2 runs WIFI TX AMSDU function \033[0m"
		/bin/vboot -c 2 -f /bin/PE_images/zephyr_07F_pe2_TXAMSDU.bin -a 0x3200000 -s 0x80000 -d
	elif [ "$pe2_default_func" == "PE2_DEFAULT_FUNCTION=1" ] && [ -f "/bin/PE_images/zephyr_07F_pe2_DEFAULT.bin" ]; then
		echo 1 2 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE2 runs HTTP function \033[0m"
		/bin/vboot -c 2 -f /bin/PE_images/zephyr_07F_pe2_DEFAULT.bin -a 0x3200000 -s 0x80000 -d
	elif [ "$pe2_default_func" == "PE2_DEFAULT_FUNCTION=2" ] && [ -f "/bin/PE_images/zephyr_07F_pe2_DEFAULT.bin" ]; then
		echo 2 2 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE2 runs CRYPTO function \033[0m"
		/bin/vboot -c 2 -f /bin/PE_images/zephyr_07F_pe2_DEFAULT.bin -a 0x3200000 -s 0x80000 -d
		echo 1 > /proc/fc/ctrl/ipsec_en_shortCut
		sleep 1
		echo 1 > /proc/fc/ctrl/ipsec_en_pe_offload
	elif [ "$pe2_default_func" == "PE2_DEFAULT_FUNCTION=3" ] && [ -f "${wfo_fw_path}/zephyr_07F_pe2_WFO.bin" ]; then
		echo 3 2 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE2 runs WFO function \033[0m"
		/bin/vboot -c 2 -f ${wfo_fw_path}/zephyr_07F_pe2_WFO.bin -a 0x3200000 -s 0x80000 -d
	elif [ "$pe2_default_func" == "PE2_DEFAULT_FUNCTION=4" ] && [ -f "/bin/PE_images/zephyr_07F_pe2_DEFAULT.bin" ]; then
		echo 4 2 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE2 runs SRv6 function \033[0m"
		/bin/vboot -c 2 -f /bin/PE_images/zephyr_07F_pe2_DEFAULT.bin -a 0x3200000 -s 0x80000 -d
		sleep 1
		echo 1 > /proc/fc/ctrl/pe_srv6_offload
	elif [ "$pe2_default_func" == "PE2_DEFAULT_FUNCTION=5" ] && [ -f "/bin/PE_images/zephyr_07F_pe2_DEFAULT.bin" ]; then
		echo 5 2 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE2 runs SW Queue function \033[0m"
		/bin/vboot -c 2 -f /bin/PE_images/zephyr_07F_pe2_DEFAULT.bin -a 0x3200000 -s 0x80000 -d
	elif [ "$pe2_default_func" == "PE2_DEFAULT_FUNCTION=6" ] && [ -f "/bin/PE_images/zephyr_07F_pe2_DEFAULT.bin" ]; then
		echo 6 2 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE2 runs SW TC Queue 0 function \033[0m"
		/bin/vboot -c 2 -f /bin/PE_images/zephyr_07F_pe2_DEFAULT.bin -a 0x3200000 -s 0x80000 -d
	elif [ "$pe2_default_func" == "PE2_DEFAULT_FUNCTION=7" ] && [ -f "/bin/PE_images/zephyr_07F_pe2_DEFAULT.bin" ]; then
		echo 7 2 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
		echo -e "\033[1;37;45m[PE] PE2 runs SW TC Queue 1 function \033[0m"
		/bin/vboot -c 2 -f /bin/PE_images/zephyr_07F_pe2_DEFAULT.bin -a 0x3200000 -s 0x80000 -d	
	else
		echo -e "\033[1;33;41m[WARNING] $pe2_default_func is unknown !! \033[0m"
	fi
fi
