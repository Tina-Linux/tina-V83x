/*************************************************************************/ /*!
@File
@Title          GL ES 1.x extensions.
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef __drv_glext_h__
#define __drv_glext_h__

/* We want all the prototypes */
#define GL_GLEXT_PROTOTYPES

/* Normally GL extensions with GL_GLEXT_PROTOTYPES will get default
 * visibility with GL_API, but we want to override that. The old
 * SGX headers used GL_API_EXT to achieve this.
 */
#if defined(__linux__) || defined(__QNXNTO__)
#define GL_API_EXT __attribute__((visibility("hidden")))
#else
#define GL_API_EXT
#endif

/* NOTE: If you want to override an extension, you could put it here,
 * before we include the Khronos header, so the khronos header doesn't
 * define it first.
 */

#if defined _MSC_VER
#pragma push_macro("GL_API")
#endif
#undef GL_API
#define GL_API GL_API_EXT

#define GL_APIENTRYP GL_APIENTRY*

/* Android's glext.h is outdated. Workaround for GL_EXT_multi_draw_arrays. */
#ifndef GL_EXT_multi_draw_arrays
#define GL_EXT_multi_draw_arrays 1
#ifdef GL_GLEXT_PROTOTYPES
GL_API void GL_APIENTRY glMultiDrawArraysEXT (GLenum, const GLint *, const GLsizei *, GLsizei);
GL_API void GL_APIENTRY glMultiDrawElementsEXT (GLenum, const GLsizei *, GLenum, const GLvoid* *, GLsizei);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (GL_APIENTRYP PFNGLMULTIDRAWARRAYSEXTPROC) (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount);
typedef void (GL_APIENTRYP PFNGLMULTIDRAWELEMENTSEXTPROC) (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount);
#endif

/* Include the Khronos header first */
#include <GLES/glext.h>

/* NOTE: Extensions following might be overridden by the platform's
 * version of glext.h.
 */

/* GL_IMG_texture_format_BGRA8888 */
#ifndef GL_IMG_texture_format_BGRA8888
#define GL_IMG_texture_format_BGRA8888 1
#endif

/* GL_IMG_texture_env_enhanced_fixed_function */
#ifndef GL_IMG_texture_env_enhanced_fixed_function
#define GL_IMG_texture_env_enhanced_fixed_function 1
#endif

/* GL_IMG_vertex_program */
#ifndef GL_IMG_vertex_program
#define GL_IMG_vertex_program 1
#ifdef GL_GLEXT_PROTOTYPES
GL_API void GL_APIENTRY glVertexAttrib4fARB(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GL_API void GL_APIENTRY glProgramEnvParameter4fARB(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GL_API void GL_APIENTRY glProgramEnvParameter4fvARB(GLenum target, GLuint index, const GLfloat *params);
GL_API void GL_APIENTRY glProgramLocalParameter4fARB(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GL_API void GL_APIENTRY glProgramLocalParameter4fvARB(GLenum target, GLuint index, const GLfloat *params);
GL_API void GL_APIENTRY glVertexAttrib4xIMG(GLuint index, GLfixed x, GLfixed y, GLfixed z, GLfixed w);
GL_API void GL_APIENTRY glProgramLocalParameter4xIMG(GLenum target, GLuint index, GLfixed x, GLfixed y, GLfixed z, GLfixed w);
GL_API void GL_APIENTRY glProgramLocalParameter4xvIMG(GLenum target, GLuint index, const GLfixed *params);
GL_API void GL_APIENTRY glProgramEnvParameter4xIMG(GLenum target, GLuint index, GLfixed x, GLfixed y, GLfixed z, GLfixed w);
GL_API void GL_APIENTRY glProgramEnvParameter4xvIMG(GLenum target, GLuint index, const GLfixed *params);
GL_API void GL_APIENTRY glVertexAttribPointerARB(GLuint index, GLsizei size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
GL_API void GL_APIENTRY glEnableVertexAttribArrayARB(GLuint index);
GL_API void GL_APIENTRY glDisableVertexAttribArrayARB(GLuint index);
GL_API void GL_APIENTRY glProgramStringARB(GLenum target, GLenum format, GLsizei len, const void *string);
GL_API void GL_APIENTRY glBindProgramARB(GLenum target, GLuint program);
GL_API void GL_APIENTRY glDeleteProgramsARB(GLsizei n, const GLuint *programs);
GL_API void GL_APIENTRY glGenProgramsARB(GLsizei n, GLuint *programs);
#endif
#endif

/* Workaround for what seems to be a khronos header issue */
#define GL_ALPHA8_OES				0x803C
#define GL_LUMINANCE8_OES			0x8040
#define GL_LUMINANCE8_ALPHA8_OES	0x8045
#define GL_LUMINANCE4_ALPHA4_OES	0x8043

#undef GL_API
#define GL_API KHRONOS_APICALL
#if defined _MSC_VER
#pragma pop_macro("GL_API")
#endif

#endif /* __drv_glext_h__ */
