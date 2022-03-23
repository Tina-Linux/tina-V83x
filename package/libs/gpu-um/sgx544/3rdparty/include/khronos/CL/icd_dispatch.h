/*
 * Copyright (c) 2009 The Khronos Group Inc.  All rights reserved.
 *
 * NOTICE TO KHRONOS MEMBER:  
 *
 * NVIDIA has assigned the copyright for this object code to Khronos.
 * This object code is subject to Khronos ownership rights under U.S. and
 * international Copyright laws.  
 *
 * Permission is hereby granted, free of charge, to any Khronos Member
 * obtaining a copy of this software and/or associated documentation files
 * (the "Materials"), to use, copy, modify and merge the Materials in object
 * form only and to publish, distribute and/or sell copies of the Materials  
 * solely in object code form as part of conformant OpenCL API implementations, 
 * subject to the following conditions: 
 *
 * Khronos Members shall ensure that their respective ICD implementation, 
 * that is installed over another Khronos Members' ICD implementation, will 
 * continue to support all OpenCL devices (hardware and software) supported 
 * by the replaced ICD implementation. For the purposes of this notice, "ICD"
 * shall mean a library that presents an implementation of the OpenCL API for 
 * the purpose routing API calls to different vendor implementation.
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Materials. 
 *
 * KHRONOS AND NVIDIA MAKE NO REPRESENTATION ABOUT THE SUITABILITY OF THIS 
 * SOURCE CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR 
 * IMPLIED WARRANTY OF ANY KIND.  KHRONOS AND NVIDIA DISCLAIM ALL WARRANTIES 
 * WITH REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL KHRONOS OR NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, 
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING 
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, 
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH   
 * THE USE OR PERFORMANCE OF THIS SOURCE CODE.
 *
 * U.S. Government End Users.   This source code is a "commercial item" as
 * that term is defined at 48 C.F.R. 2.101 (OCT 1995), consisting of
 * "commercial computer software" and "commercial computer software 
 * documentation" as such terms are used in 48 C.F.R. 12.212 (SEPT 1995)
 * and is provided to the U.S. Government only as a commercial end item.
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the 
 * source code with only those rights set forth herein.
 */

#ifndef _ICD_DISPATCH_H_
#define _ICD_DISPATCH_H_


// cl.h
#include <CL/cl.h>

// cl_gl.h and required files

#include <CL/cl_gl.h>
#include <CL/cl_ext.h>

/*
 *
 * function pointer typedefs
 *
 */

// Platform APIs
typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetPlatformIDs)(
                 cl_uint          num_entries,
                 cl_platform_id * platforms,
                 cl_uint *        num_platforms) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetPlatformInfo)(
    cl_platform_id   platform, 
    cl_platform_info param_name,
    size_t           param_value_size, 
    void *           param_value,
    size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

// Device APIs
typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetDeviceIDs)(
    cl_platform_id   platform,
    cl_device_type   device_type, 
    cl_uint          num_entries, 
    cl_device_id *   devices, 
    cl_uint *        num_devices) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetDeviceInfo)(
    cl_device_id    device,
    cl_device_info  param_name, 
    size_t          param_value_size, 
    void *          param_value,
    size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

// Context APIs  
typedef CL_API_ENTRY cl_context (CL_API_CALL *KHRpfn_clCreateContext)(
    const cl_context_properties * properties,
    cl_uint                 num_devices,
    const cl_device_id *    devices,
    void (CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
    void *                  user_data,
    cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_context (CL_API_CALL *KHRpfn_clCreateContextFromType)(
    const cl_context_properties * properties,
    cl_device_type          device_type,
    void (CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
    void *                  user_data,
    cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clRetainContext)(
    cl_context context) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clReleaseContext)(
    cl_context context) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetContextInfo)(
    cl_context         context, 
    cl_context_info    param_name, 
    size_t             param_value_size, 
    void *             param_value, 
    size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

// Command Queue APIs
typedef CL_API_ENTRY cl_command_queue (CL_API_CALL *KHRpfn_clCreateCommandQueue)(
    cl_context                     context, 
    cl_device_id                   device, 
    cl_command_queue_properties    properties,
    cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clRetainCommandQueue)(
    cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clReleaseCommandQueue)(
    cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetCommandQueueInfo)(
    cl_command_queue      command_queue,
    cl_command_queue_info param_name,
    size_t                param_value_size,
    void *                param_value,
    size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clSetCommandQueueProperty)(
    cl_command_queue              command_queue,
    cl_command_queue_properties   properties, 
    cl_bool                        enable,
    cl_command_queue_properties * old_properties) CL_API_SUFFIX__VERSION_1_0;

// Memory Object APIs
typedef CL_API_ENTRY cl_mem (CL_API_CALL *KHRpfn_clCreateBuffer)(
    cl_context   context,
    cl_mem_flags flags,
    size_t       size,
    void *       host_ptr,
    cl_int *     errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_mem (CL_API_CALL *KHRpfn_clCreateImage2D)(
    cl_context              context,
    cl_mem_flags            flags,
    const cl_image_format * image_format,
    size_t                  image_width,
    size_t                  image_height,
    size_t                  image_row_pitch, 
    void *                  host_ptr,
    cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0;
                        
typedef CL_API_ENTRY cl_mem (CL_API_CALL *KHRpfn_clCreateImage3D)(
    cl_context              context,
    cl_mem_flags            flags,
    const cl_image_format * image_format,
    size_t                  image_width, 
    size_t                  image_height,
    size_t                  image_depth, 
    size_t                  image_row_pitch, 
    size_t                  image_slice_pitch, 
    void *                  host_ptr,
    cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0;
                        
typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clRetainMemObject)(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clReleaseMemObject)(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetSupportedImageFormats)(
    cl_context           context,
    cl_mem_flags         flags,
    cl_mem_object_type   image_type,
    cl_uint              num_entries,
    cl_image_format *    image_formats,
    cl_uint *            num_image_formats) CL_API_SUFFIX__VERSION_1_0;
                                    
typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetMemObjectInfo)(
    cl_mem           memobj,
    cl_mem_info      param_name, 
    size_t           param_value_size,
    void *           param_value,
    size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetImageInfo)(
    cl_mem           image,
    cl_image_info    param_name, 
    size_t           param_value_size,
    void *           param_value,
    size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

// Sampler APIs
typedef CL_API_ENTRY cl_sampler (CL_API_CALL *KHRpfn_clCreateSampler)(
    cl_context          context,
    cl_bool             normalized_coords, 
    cl_addressing_mode  addressing_mode, 
    cl_filter_mode      filter_mode,
    cl_int *            errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clRetainSampler)(cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clReleaseSampler)(cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetSamplerInfo)(
    cl_sampler         sampler,
    cl_sampler_info    param_name,
    size_t             param_value_size,
    void *             param_value,
    size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;
                            
// Program Object APIs
typedef CL_API_ENTRY cl_program (CL_API_CALL *KHRpfn_clCreateProgramWithSource)(
    cl_context        context,
    cl_uint           count,
    const char **     strings,
    const size_t *    lengths,
    cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_program (CL_API_CALL *KHRpfn_clCreateProgramWithBinary)(
    cl_context                     context,
    cl_uint                        num_devices,
    const cl_device_id *           device_list,
    const size_t *                 lengths,
    const unsigned char **         binaries,
    cl_int *                       binary_status,
    cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clRetainProgram)(cl_program program) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clReleaseProgram)(cl_program program) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clBuildProgram)(
    cl_program           program,
    cl_uint              num_devices,
    const cl_device_id * device_list,
    const char *         options, 
    void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data),
    void *               user_data) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clUnloadCompiler)(void) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetProgramInfo)(
    cl_program         program,
    cl_program_info    param_name,
    size_t             param_value_size,
    void *             param_value,
    size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetProgramBuildInfo)(
    cl_program            program,
    cl_device_id          device,
    cl_program_build_info param_name,
    size_t                param_value_size,
    void *                param_value,
    size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;
                            
// Kernel Object APIs
typedef CL_API_ENTRY cl_kernel (CL_API_CALL *KHRpfn_clCreateKernel)(
    cl_program      program,
    const char *    kernel_name,
    cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clCreateKernelsInProgram)(
    cl_program     program,
    cl_uint        num_kernels,
    cl_kernel *    kernels,
    cl_uint *      num_kernels_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clRetainKernel)(cl_kernel    kernel) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clReleaseKernel)(cl_kernel   kernel) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clSetKernelArg)(
    cl_kernel    kernel,
    cl_uint      arg_index,
    size_t       arg_size,
    const void * arg_value) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetKernelInfo)(
    cl_kernel       kernel,
    cl_kernel_info  param_name,
    size_t          param_value_size,
    void *          param_value,
    size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetKernelWorkGroupInfo)(
    cl_kernel                  kernel,
    cl_device_id               device,
    cl_kernel_work_group_info  param_name,
    size_t                     param_value_size,
    void *                     param_value,
    size_t *                   param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

// Event Object APIs
typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clWaitForEvents)(
    cl_uint             num_events,
    const cl_event *    event_list) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetEventInfo)(
    cl_event         event,
    cl_event_info    param_name,
    size_t           param_value_size,
    void *           param_value,
    size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;
                            
typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clRetainEvent)(cl_event event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clReleaseEvent)(cl_event event) CL_API_SUFFIX__VERSION_1_0;

// Profiling APIs
typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetEventProfilingInfo)(
    cl_event            event,
    cl_profiling_info   param_name,
    size_t              param_value_size,
    void *              param_value,
    size_t *            param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;
                                
// Flush and Finish APIs
typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clFlush)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clFinish)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

// Enqueued Commands APIs
typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueReadBuffer)(
    cl_command_queue    command_queue,
    cl_mem              buffer,
    cl_bool             blocking_read,
    size_t              offset,
    size_t              cb, 
    void *              ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event) CL_API_SUFFIX__VERSION_1_0;
                            
typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueWriteBuffer)(
    cl_command_queue   command_queue, 
    cl_mem             buffer, 
    cl_bool            blocking_write, 
    size_t             offset, 
    size_t             cb, 
    const void *       ptr, 
    cl_uint            num_events_in_wait_list, 
    const cl_event *   event_wait_list, 
    cl_event *         event) CL_API_SUFFIX__VERSION_1_0;
                            
typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueCopyBuffer)(
    cl_command_queue    command_queue, 
    cl_mem              src_buffer,
    cl_mem              dst_buffer, 
    size_t              src_offset,
    size_t              dst_offset,
    size_t              cb, 
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event) CL_API_SUFFIX__VERSION_1_0;
                            
typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueReadImage)(
    cl_command_queue     command_queue,
    cl_mem               image,
    cl_bool              blocking_read, 
    const size_t *       origin,
    const size_t *       region,
    size_t               row_pitch,
    size_t               slice_pitch, 
    void *               ptr,
    cl_uint              num_events_in_wait_list,
    const cl_event *     event_wait_list,
    cl_event *           event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueWriteImage)(
    cl_command_queue    command_queue,
    cl_mem              image,
    cl_bool             blocking_write, 
    const size_t *      origin,
    const size_t *      region,
    size_t              input_row_pitch,
    size_t              input_slice_pitch, 
    const void *        ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueCopyImage)(
    cl_command_queue     command_queue,
    cl_mem               src_image,
    cl_mem               dst_image, 
    const size_t *       src_origin,
    const size_t *       dst_origin,
    const size_t *       region, 
    cl_uint              num_events_in_wait_list,
    const cl_event *     event_wait_list,
    cl_event *           event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueCopyImageToBuffer)(
    cl_command_queue command_queue,
    cl_mem           src_image,
    cl_mem           dst_buffer, 
    const size_t *   src_origin,
    const size_t *   region, 
    size_t           dst_offset,
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event *       event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueCopyBufferToImage)(
    cl_command_queue command_queue,
    cl_mem           src_buffer,
    cl_mem           dst_image, 
    size_t           src_offset,
    const size_t *   dst_origin,
    const size_t *   region, 
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event *       event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY void * (CL_API_CALL *KHRpfn_clEnqueueMapBuffer)(
    cl_command_queue command_queue,
    cl_mem           buffer,
    cl_bool          blocking_map, 
    cl_map_flags     map_flags,
    size_t           offset,
    size_t           cb,
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event *       event,
    cl_int *         errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY void * (CL_API_CALL *KHRpfn_clEnqueueMapImage)(
    cl_command_queue  command_queue,
    cl_mem            image, 
    cl_bool           blocking_map, 
    cl_map_flags      map_flags, 
    const size_t *    origin,
    const size_t *    region,
    size_t *          image_row_pitch,
    size_t *          image_slice_pitch,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event,
    cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueUnmapMemObject)(
    cl_command_queue command_queue,
    cl_mem           memobj,
    void *           mapped_ptr,
    cl_uint          num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueNDRangeKernel)(
    cl_command_queue command_queue,
    cl_kernel        kernel,
    cl_uint          work_dim,
    const size_t *   global_work_offset,
    const size_t *   global_work_size,
    const size_t *   local_work_size,
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event *       event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueTask)(
    cl_command_queue  command_queue,
    cl_kernel         kernel,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueNativeKernel)(
    cl_command_queue  command_queue,
    void (*user_func)(void *), 
    void *            args,
    size_t            cb_args, 
    cl_uint           num_mem_objects,
    const cl_mem *    mem_list,
    const void **     args_mem_loc,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueMarker)(
    cl_command_queue    command_queue,
    cl_event *          event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueWaitForEvents)(
    cl_command_queue command_queue,
    cl_uint          num_events,
    const cl_event * event_list) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueBarrier)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY void * (CL_API_CALL *KHRpfn_clGetExtensionFunctionAddress)(const char *function_name) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_mem (CL_API_CALL *KHRpfn_clCreateFromGLBuffer)(
    cl_context    context,
    cl_mem_flags  flags,
    cl_GLuint        bufobj,
    int *         errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_mem (CL_API_CALL *KHRpfn_clCreateFromGLTexture2D)(
    cl_context      context,
    cl_mem_flags    flags,
    cl_GLenum       target,
    cl_GLint        miplevel,
    cl_GLuint       texture,
    cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_mem (CL_API_CALL *KHRpfn_clCreateFromGLTexture3D)(
    cl_context      context,
    cl_mem_flags    flags,
    cl_GLenum       target,
    cl_GLint        miplevel,
    cl_GLuint       texture,
    cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_mem (CL_API_CALL *KHRpfn_clCreateFromGLRenderbuffer)(
    cl_context           context,
    cl_mem_flags         flags,
    cl_GLuint            renderbuffer,
    cl_int *             errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetGLObjectInfo)(
    cl_mem               memobj,
    cl_gl_object_type *  gl_object_type,
    cl_GLuint *          gl_object_name) CL_API_SUFFIX__VERSION_1_0;
                  
typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetGLTextureInfo)(
    cl_mem               memobj,
    cl_gl_texture_info   param_name,
    size_t               param_value_size,
    void *               param_value,
    size_t *             param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueAcquireGLObjects)(
    cl_command_queue     command_queue,
    cl_uint              num_objects,
    const cl_mem *       mem_objects,
    cl_uint              num_events_in_wait_list,
    const cl_event *     event_wait_list,
    cl_event *           event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueReleaseGLObjects)(
    cl_command_queue     command_queue,
    cl_uint              num_objects,
    const cl_mem *       mem_objects,
    cl_uint              num_events_in_wait_list,
    const cl_event *     event_wait_list,
    cl_event *           event) CL_API_SUFFIX__VERSION_1_0;

/* cl_khr_gl_sharing */
typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetGLContextInfoKHR)(
    const cl_context_properties *properties,
    cl_gl_context_info param_name,
    size_t param_value_size,
    void *param_value,
    size_t *param_value_size_ret);



typedef void *KHRpfn_clGetDeviceIDsFromD3D10KHR;
typedef void *KHRpfn_clCreateFromD3D10BufferKHR;
typedef void *KHRpfn_clCreateFromD3D10Texture2DKHR;
typedef void *KHRpfn_clCreateFromD3D10Texture3DKHR;
typedef void *KHRpfn_clEnqueueAcquireD3D10ObjectsKHR;
typedef void *KHRpfn_clEnqueueReleaseD3D10ObjectsKHR;


/* OpenCL 1.1 */

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clSetEventCallback)(
    cl_event            /* event */,
    cl_int              /* command_exec_callback_type */,
    void (CL_CALLBACK * /* pfn_notify */)(cl_event, cl_int, void *),
    void *              /* user_data */) CL_API_SUFFIX__VERSION_1_1;

typedef CL_API_ENTRY cl_mem (CL_API_CALL *KHRpfn_clCreateSubBuffer)(
    cl_mem                   /* buffer */,
    cl_mem_flags             /* flags */,
    cl_buffer_create_type    /* buffer_create_type */,
    const void *             /* buffer_create_info */,
    cl_int *                 /* errcode_ret */) CL_API_SUFFIX__VERSION_1_1;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clSetMemObjectDestructorCallback)(
    cl_mem /* memobj */, 
    void (CL_CALLBACK * /*pfn_notify*/)( cl_mem /* memobj */, void* /*user_data*/), 
    void * /*user_data */ ) CL_API_SUFFIX__VERSION_1_1;

typedef CL_API_ENTRY cl_event (CL_API_CALL *KHRpfn_clCreateUserEvent)(
    cl_context    /* context */,
    cl_int *      /* errcode_ret */) CL_API_SUFFIX__VERSION_1_1;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clSetUserEventStatus)(
    cl_event   /* event */,
    cl_int     /* execution_status */) CL_API_SUFFIX__VERSION_1_1;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueReadBufferRect)(
    cl_command_queue    /* command_queue */,
    cl_mem              /* buffer */,
    cl_bool             /* blocking_read */,
    const size_t *      /* buffer_offset */,
    const size_t *      /* host_offset */, 
    const size_t *      /* region */,
    size_t              /* buffer_row_pitch */,
    size_t              /* buffer_slice_pitch */,
    size_t              /* host_row_pitch */,
    size_t              /* host_slice_pitch */,                        
    void *              /* ptr */,
    cl_uint             /* num_events_in_wait_list */,
    const cl_event *    /* event_wait_list */,
    cl_event *          /* event */) CL_API_SUFFIX__VERSION_1_1;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueWriteBufferRect)(
    cl_command_queue    /* command_queue */,
    cl_mem              /* buffer */,
    cl_bool             /* blocking_read */,
    const size_t *      /* buffer_offset */,
    const size_t *      /* host_offset */, 
    const size_t *      /* region */,
    size_t              /* buffer_row_pitch */,
    size_t              /* buffer_slice_pitch */,
    size_t              /* host_row_pitch */,
    size_t              /* host_slice_pitch */,                        
    const void *        /* ptr */,
    cl_uint             /* num_events_in_wait_list */,
    const cl_event *    /* event_wait_list */,
    cl_event *          /* event */) CL_API_SUFFIX__VERSION_1_1;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueCopyBufferRect)(
    cl_command_queue    /* command_queue */, 
    cl_mem              /* src_buffer */,
    cl_mem              /* dst_buffer */, 
    const size_t *      /* src_origin */,
    const size_t *      /* dst_origin */,
    const size_t *      /* region */, 
    size_t              /* src_row_pitch */,
    size_t              /* src_slice_pitch */,
    size_t              /* dst_row_pitch */,
    size_t              /* dst_slice_pitch */,
    cl_uint             /* num_events_in_wait_list */,
    const cl_event *    /* event_wait_list */,
    cl_event *          /* event */) CL_API_SUFFIX__VERSION_1_1;

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clCreateSubDevicesEXT)(
    cl_device_id     in_device,
    const cl_device_partition_property_ext * partition_properties,
    cl_uint          num_entries,
    cl_device_id *   out_devices,
    cl_uint *        num_devices);

typedef CL_API_ENTRY cl_int (CL_API_CALL * KHRpfn_clRetainDeviceEXT)(
    cl_device_id     device) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL * KHRpfn_clReleaseDeviceEXT)(
    cl_device_id     device) CL_API_SUFFIX__VERSION_1_0;

/*
 *
 * vendor dispatch table structure
 *
 * note that the types in the structure KHRicdVendorDispatch mirror the function 
 * names listed in the string table khrIcdVendorDispatchFunctionNames
 *
 */

typedef struct KHRicdVendorDispatchRec KHRicdVendorDispatch;

struct KHRicdVendorDispatchRec
{
    KHRpfn_clGetPlatformIDs               clGetPlatformIDs;
    KHRpfn_clGetPlatformInfo              clGetPlatformInfo;
    KHRpfn_clGetDeviceIDs                 clGetDeviceIDs;
    KHRpfn_clGetDeviceInfo                clGetDeviceInfo;
    KHRpfn_clCreateContext                clCreateContext;
    KHRpfn_clCreateContextFromType        clCreateContextFromType;
    KHRpfn_clRetainContext                clRetainContext;
    KHRpfn_clReleaseContext               clReleaseContext;
    KHRpfn_clGetContextInfo               clGetContextInfo;
    KHRpfn_clCreateCommandQueue           clCreateCommandQueue;
    KHRpfn_clRetainCommandQueue           clRetainCommandQueue;
    KHRpfn_clReleaseCommandQueue          clReleaseCommandQueue;
    KHRpfn_clGetCommandQueueInfo          clGetCommandQueueInfo;
    KHRpfn_clSetCommandQueueProperty      clSetCommandQueueProperty;
    KHRpfn_clCreateBuffer                 clCreateBuffer;
    KHRpfn_clCreateImage2D                clCreateImage2D;
    KHRpfn_clCreateImage3D                clCreateImage3D;
    KHRpfn_clRetainMemObject              clRetainMemObject;
    KHRpfn_clReleaseMemObject             clReleaseMemObject;
    KHRpfn_clGetSupportedImageFormats     clGetSupportedImageFormats;
    KHRpfn_clGetMemObjectInfo             clGetMemObjectInfo;
    KHRpfn_clGetImageInfo                 clGetImageInfo;
    KHRpfn_clCreateSampler                clCreateSampler;
    KHRpfn_clRetainSampler                clRetainSampler;
    KHRpfn_clReleaseSampler               clReleaseSampler;
    KHRpfn_clGetSamplerInfo               clGetSamplerInfo;
    KHRpfn_clCreateProgramWithSource      clCreateProgramWithSource;
    KHRpfn_clCreateProgramWithBinary      clCreateProgramWithBinary;
    KHRpfn_clRetainProgram                clRetainProgram;
    KHRpfn_clReleaseProgram               clReleaseProgram;
    KHRpfn_clBuildProgram                 clBuildProgram;
    KHRpfn_clUnloadCompiler               clUnloadCompiler;
    KHRpfn_clGetProgramInfo               clGetProgramInfo;
    KHRpfn_clGetProgramBuildInfo          clGetProgramBuildInfo;
    KHRpfn_clCreateKernel                 clCreateKernel;
    KHRpfn_clCreateKernelsInProgram       clCreateKernelsInProgram;
    KHRpfn_clRetainKernel                 clRetainKernel;
    KHRpfn_clReleaseKernel                clReleaseKernel;
    KHRpfn_clSetKernelArg                 clSetKernelArg;
    KHRpfn_clGetKernelInfo                clGetKernelInfo;
    KHRpfn_clGetKernelWorkGroupInfo       clGetKernelWorkGroupInfo;
    KHRpfn_clWaitForEvents                clWaitForEvents;
    KHRpfn_clGetEventInfo                 clGetEventInfo;
    KHRpfn_clRetainEvent                  clRetainEvent;
    KHRpfn_clReleaseEvent                 clReleaseEvent;
    KHRpfn_clGetEventProfilingInfo        clGetEventProfilingInfo;
    KHRpfn_clFlush                        clFlush;
    KHRpfn_clFinish                       clFinish;
    KHRpfn_clEnqueueReadBuffer            clEnqueueReadBuffer;
    KHRpfn_clEnqueueWriteBuffer           clEnqueueWriteBuffer;
    KHRpfn_clEnqueueCopyBuffer            clEnqueueCopyBuffer;
    KHRpfn_clEnqueueReadImage             clEnqueueReadImage;
    KHRpfn_clEnqueueWriteImage            clEnqueueWriteImage;
    KHRpfn_clEnqueueCopyImage             clEnqueueCopyImage;
    KHRpfn_clEnqueueCopyImageToBuffer     clEnqueueCopyImageToBuffer;
    KHRpfn_clEnqueueCopyBufferToImage     clEnqueueCopyBufferToImage;
    KHRpfn_clEnqueueMapBuffer             clEnqueueMapBuffer;
    KHRpfn_clEnqueueMapImage              clEnqueueMapImage;
    KHRpfn_clEnqueueUnmapMemObject        clEnqueueUnmapMemObject;
    KHRpfn_clEnqueueNDRangeKernel         clEnqueueNDRangeKernel;
    KHRpfn_clEnqueueTask                  clEnqueueTask;
    KHRpfn_clEnqueueNativeKernel          clEnqueueNativeKernel;
    KHRpfn_clEnqueueMarker                clEnqueueMarker;
    KHRpfn_clEnqueueWaitForEvents         clEnqueueWaitForEvents;
    KHRpfn_clEnqueueBarrier               clEnqueueBarrier;
    KHRpfn_clGetExtensionFunctionAddress  clGetExtensionFunctionAddress;
    KHRpfn_clCreateFromGLBuffer           clCreateFromGLBuffer;
    KHRpfn_clCreateFromGLTexture2D        clCreateFromGLTexture2D;
    KHRpfn_clCreateFromGLTexture3D        clCreateFromGLTexture3D;
    KHRpfn_clCreateFromGLRenderbuffer     clCreateFromGLRenderbuffer;
    KHRpfn_clGetGLObjectInfo              clGetGLObjectInfo;
    KHRpfn_clGetGLTextureInfo             clGetGLTextureInfo;
    KHRpfn_clEnqueueAcquireGLObjects      clEnqueueAcquireGLObjects;
    KHRpfn_clEnqueueReleaseGLObjects      clEnqueueReleaseGLObjects;
    KHRpfn_clGetGLContextInfoKHR          clGetGLContextInfoKHR;

    KHRpfn_clGetDeviceIDsFromD3D10KHR      clGetDeviceIDsFromD3D10KHR;
    KHRpfn_clCreateFromD3D10BufferKHR      clCreateFromD3D10BufferKHR;
    KHRpfn_clCreateFromD3D10Texture2DKHR   clCreateFromD3D10Texture2DKHR;
    KHRpfn_clCreateFromD3D10Texture3DKHR   clCreateFromD3D10Texture3DKHR;
    KHRpfn_clEnqueueAcquireD3D10ObjectsKHR clEnqueueAcquireD3D10ObjectsKHR;
    KHRpfn_clEnqueueReleaseD3D10ObjectsKHR clEnqueueReleaseD3D10ObjectsKHR;

    KHRpfn_clSetEventCallback               clSetEventCallback;
    KHRpfn_clCreateSubBuffer                clCreateSubBuffer;
    KHRpfn_clSetMemObjectDestructorCallback clSetMemObjectDestructorCallback;
    KHRpfn_clCreateUserEvent                clCreateUserEvent;
    KHRpfn_clSetUserEventStatus             clSetUserEventStatus;
    KHRpfn_clEnqueueReadBufferRect          clEnqueueReadBufferRect;
    KHRpfn_clEnqueueWriteBufferRect         clEnqueueWriteBufferRect;
    KHRpfn_clEnqueueCopyBufferRect          clEnqueueCopyBufferRect;

    KHRpfn_clCreateSubDevicesEXT            clCreateSubDevicesEXT;
    KHRpfn_clRetainDeviceEXT                clRetainDeviceEXT;
    KHRpfn_clReleaseDeviceEXT               clReleaseDeviceEXT;
};

#endif // _ICD_DISPATCH_H_

