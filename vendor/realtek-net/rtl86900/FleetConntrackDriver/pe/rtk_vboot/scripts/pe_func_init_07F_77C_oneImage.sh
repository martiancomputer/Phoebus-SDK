#!/bin/sh

rtk_soc_family=`cat /sys/devices/soc0/family`

if [ "$rtk_soc_family" == "Taurus" ]; then
	echo -e "\033[1;37;45m[PE] Load image for Taurus \033[0m"
	/bin/sh /etc/scripts/pe_func_init_77C.sh
elif [ "$rtk_soc_family" == "Elnath" ]; then
	echo -e "\033[1;37;45m[PE] Load image for Elnath \033[0m"
	/bin/sh /etc/scripts/pe_func_init_07F.sh
else
	echo -e "\033[1;37;45m[PE] Fail to load image for unknown rtk_soc_family $rtk_soc_family !! \033[0m"
fi
