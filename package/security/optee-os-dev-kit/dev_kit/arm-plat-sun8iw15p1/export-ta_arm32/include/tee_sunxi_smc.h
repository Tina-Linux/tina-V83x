

/** allwinner sunxi SMC hardware module */
#ifndef __SUNXI_SMC_H__
#define __SUNXI_SMC_H__


#define   SUNXI_DRM_PROCTECTION_SETUP_ID              0
#define   SUNXI_DRM_PROCTECTION_SHUTDOWN_ID           1
#define   SUNXI_DRM_PROCTECTION_SETUP_VE_ID           2
#define   SUNXI_DRM_PROCTECTION_SHUTDOWN_VE_ID        3
#define   SUNXI_DRM_AREA_START                        4
#define   SUNXI_DRM_AREA_SIZE                         5


int sunxi_drm_protection_setup(void);
int sunxi_drm_protection_shutdown(void);

int sunxi_drm_protection_setup_ve(void);
int sunxi_drm_protection_shutdown_ve(void);

uint32_t sunxi_drm_area_start(void);
uint32_t sunxi_drm_area_size(void);

#endif /* __HWSMC_H__ */
