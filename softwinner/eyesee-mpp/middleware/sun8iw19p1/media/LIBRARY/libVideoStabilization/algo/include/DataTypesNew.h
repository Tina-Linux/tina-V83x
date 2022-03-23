#ifndef DATA_TYPES_NEW_H
#define DATA_TYPES_NEW_H

#include "Datatypes.h"

typedef enum COMPRESS_TYPE{

    COMP_EQUAL_STEP,

	COMP_NEW

}COMP_TYPE;

typedef enum COLOR_CONVERT_CODE
{
    YVU420_to_YUV420  = (PLANE_YVU420 << 4)  + PLANE_YUV420,
	YUV420p_to_YUV420 = (PLANE_YUV420p << 4) + PLANE_YUV420,
	YVU422_to_YUV422  = (PLANE_YVU422 << 4)  + PLANE_YUV422,
	YVU422p_to_YUV422 = (PLANE_YUV422p << 4) + PLANE_YUV422,
	
	YUV420_to_RGB     = (PLANE_YUV420 << 4)  + PLANE_RGB,
	YUV422_to_RGB     = (PLANE_YUV422 << 4)  + PLANE_RGB,
	YUV420p_to_RGB    = (PLANE_YUV420p << 4) + PLANE_RGB,
	YUV422p_to_RGB    = (PLANE_YUV422p << 4) + PLANE_RGB,

	RGB_to_YUV420     = (PLANE_RGB << 4)  + PLANE_YUV420,
	RGB_to_YUV422     = (PLANE_RGB << 4)  + PLANE_YUV422,
	RGB_to_YUV420p    = (PLANE_RGB << 4)  + PLANE_YUV420p,
	RGB_to_YUV422p    = (PLANE_RGB << 4)  + PLANE_YUV422p
}COLOR_CVT_CODE;
 
 
typedef union _ROIP_params{

	int64_t llval;

	struct
	{
		int32_t lowval;
		int32_t highval;
	}dwval;

	struct
	{
		uint32_t bottom      : 12 ;
		uint32_t res0        :  4 ;
		uint32_t top         : 12 ;
		uint32_t res1        :  4 ;
		uint32_t right       : 12 ;
		uint32_t res2        :  4 ;
		uint32_t left        : 12 ;
		uint32_t res3        :  3 ;
		uint32_t split_info  :  1 ;
	}bits;
}ROIP_params;

typedef struct BlockInfoEQSampleHeader
{
	ROIP_params roip_params[4];
    uint32_t lut_comp_data[COMP_PIXES * COMP_PIXES];   //
}BIEQSHeader;  

//! new defined 
typedef struct MapTableInfoHeader
{
	int32_t cmp_type;              // compress type
	int32_t blk_nums;
	int8_t  b_mipmap;              // use mip_map
    void   *BIInfo_datastream;     // data stream
}MAPTIHeader;

typedef struct MaskTableInfoHeader
{
	int32_t cmp_type;              // compress type
	int32_t blk_nums;              //
	void   *MaskInfo_datastream;   // data stream
}MaskTIHeader;

typedef struct BlendInfoHeader 
{
	MAPTIHeader MTIH_datastream[2];   //
	MaskTIHeader mask_blocks[2];      // 
	int8_t *ref_maps;                 // 00, 01, 02, data source of the datastream
}BIHeader;

//! lut block info header
typedef struct LUTBlockInfoHeader
{
	uint32_t *lutdata;
    int32_t lut_width;
	int32_t lut_height;
	int32_t lut_stride;    
}LUTBIHeader;

//! 
typedef struct ImageROIHeader
{
    uint8_t *imagedata;
    int32_t roi_width;
	int32_t roi_height;
	int32_t ori_step1;
	int32_t ori_step0;
}IMGROIHeader;

//!
typedef union LUTPixelData
{
    uint32_t dwval;
	
	struct{
        uint16_t x;
		uint16_t y;
	}pixelsData0;

	struct{
		uint16_t fixed_part_x :  4;
		uint16_t int_part_x    : 12;
		uint16_t fixed_part_y :  4;
		uint16_t int_part_y    : 12;
	}pixelsData1;
}LUTPdata;


//!
typedef struct UserImageHeader
{
    int32_t width;
	int32_t height;
	//int32_t channel;

    uint8_t *pdata[3];
	int32_t stride[3];       //
	int32_t pixels_step[3];

	IMAGE_FORMAT img_format;
}IMGHeader;


#endif