#!/bin/sh

wifi_scan()
{
    echo "====== start wifi_scan_results_test ======"
    wifi_scan_results_test
    if [ $? -eq 0 ];then
	echo "====== wifi_scan_results_test successed!!! ======"
	exit 0
    else
	echo "====== wifi_scan_results_test failed!!! ======"
	exit 1
    fi
}

wifi_scan
