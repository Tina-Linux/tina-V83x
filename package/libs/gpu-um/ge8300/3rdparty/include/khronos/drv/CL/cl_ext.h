/*************************************************************************/ /*!
@File           cl_ext.h
@Title          CL extensions.
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef __drv_clext_h__
#define __drv_clext_h__

/* NOTE: If you want to override an extension, you could put it here,
 * before we include the Khronos header, so the khronos header doesn't
 * define it first.
 */

/* Include the Khronos header first */
#include <CL/cl_ext.h>

/* NOTE: Extensions following might be overridden by the platform's
 * version of cl_ext.h.
 */

/* Mipmap extension for OpenCL 1.2 */

#ifndef cl_sampler_properties
typedef cl_bitfield cl_sampler_properties;
#endif

#ifndef CL_FILTER_NONE
#define CL_FILTER_NONE CL_FILTER_LINEAR+1
#endif

#ifndef CL_SAMPLER_MIP_FILTER_MODE
#define CL_SAMPLER_MIP_FILTER_MODE 0x1155
#endif

#ifndef CL_SAMPLER_LOD_MIN
#define CL_SAMPLER_LOD_MIN 0x1156
#endif

#ifndef CL_SAMPLER_LOD_MAX
#define CL_SAMPLER_LOD_MAX 0x1157
#endif

#ifndef clCreateSamplerWithProperties
extern CL_API_ENTRY cl_sampler CL_API_CALL
clCreateSamplerWithProperties(cl_context                     /* context */,
                              const cl_sampler_properties *  /* normalized_coords */,
                              cl_int *                       /* errcode_ret */) CL_API_SUFFIX__VERSION_1_2;
#endif

/* Trusted Execute Environment Extension */
#define CL_MEM_PROTECTED_CONTENT_IMG                (1U << 29)

#ifdef CL_VERSION_2_0
/*********************************
* cl_img_gen_mipmap extension
*********************************/
#define cl_img_gen_mipmap 1

#define	CL_MIPMAP_FILTER_ANY_IMG           0x0
#define	CL_MIPMAP_FILTER_BOX_IMG           0x1

#define CL_COMMAND_GENERATE_MIPMAP 0x40D5

typedef cl_uint             cl_mipmap_filter_mode;
	
extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueGenerateMipmapIMG(cl_command_queue			command_queue,
						   cl_mem					src_image,
						   cl_mem					dst_image,
						   cl_mipmap_filter_mode	mipmap_filter_mode,
						   const size_t *			array_region,
						   const size_t				src_mip_level,
						   const size_t *			mip_region,
						   cl_uint					num_events_in_wait_list,
						   const cl_event *			event_wait_list,
						   cl_event *				event) CL_EXT_SUFFIX__VERSION_2_0;

typedef	CL_API_ENTRY cl_int
     ( CL_API_CALL * clEnqueueGenerateMipmapIMG_fn)(cl_command_queue		command_queue,
						cl_mem					src_image,
						cl_mem					dst_image,
						cl_mipmap_filter_mode	mipmap_filter_mode,
						const size_t *			array_region,
						const size_t			src_mip_level,
						const size_t *			mip_region,
						cl_uint					num_events_in_wait_list,
						const cl_event *		event_wait_list,
						cl_event *				event) CL_EXT_SUFFIX__VERSION_2_0;


#endif /* CL_VERSION_2_0 */

#endif /* __drv_clext_h__ */
