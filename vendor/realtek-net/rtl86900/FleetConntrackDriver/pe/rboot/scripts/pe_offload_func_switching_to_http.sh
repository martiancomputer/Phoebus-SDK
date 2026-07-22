#!/bin/sh

echo 0 > /proc/fc/ctrl/pe_offload_wifi_amsdu_func_switching
echo 1 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
echo -e "\033[1;37;45m[PE] PE1 runs HTTP function \033[0m"
rboot -z -2 /bin/zephyr_77C.bin
sleep 1
