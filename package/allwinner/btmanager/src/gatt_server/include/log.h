/*
 *  Copyright (C) 2019 Realtek Semiconductor Corporation.
 *
 *  Author: Alex Lu <alex_lu@realsil.com.cn>
 */

#ifndef __BTA_LOG__
#define __BTA_LOG__

#ifdef DEBUG
#define pr_debug(fmt, ...)	printf("%s(): "fmt"\n", __func__, ##__VA_ARGS__)
#else
#define pr_debug(fmt, ...)
#endif
#define pr_info(fmt, ...)	printf("%s(): "fmt"\n", __func__, ##__VA_ARGS__)
#define pr_warning(fmt, ...)	printf("%s() WARN: "fmt"\n", __func__, ##__VA_ARGS__)
#define pr_error(fmt, ...)	printf("%s() ERROR: "fmt"\n", __func__, ##__VA_ARGS__)

#endif
