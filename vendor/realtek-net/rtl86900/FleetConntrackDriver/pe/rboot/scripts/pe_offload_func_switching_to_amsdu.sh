#!/bin/sh

diag rt_pe http_test download stop
echo 0 > /proc/fc/ctrl/pe_offload_wifi_amsdu_func_switching
sleep 1
echo 0 1 > /proc/fc/ctrl/pe_func_on_pe_cpu_num
echo -e "\033[1;37;45m[PE] PE1 runs WIFI TX AMSDU function \033[0m"
rboot -z -2 /bin/zephyr_77C_TXAMSDU.bin
sleep 1
echo 1 > /proc/fc/ctrl/pe_offload_wifi_amsdu_func_switching
