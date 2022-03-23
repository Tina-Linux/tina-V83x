/*
 * =====================================================================================
 *
 *       Filename:  omx_micro.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  08/02/2018 03:24:13 PM
 *       Revision:  none
 *       Compiler:
 *
 *         Author: Gan Qiuye(ganqiuye@allwinnertech.com)
 *        Company:  Allwinnertech
 *
 * =====================================================================================
 */
#ifndef __OMX_MACRO_H__
#define __OMX_MACRO_H__

/*
  * Initializes a data structure using a pointer to the structure.
  * The initialization of OMX structures always sets up the nSize and nVersion fields of the sturcture.
  */
#define OMX_CONF_INIT_STRUCT_PTR(_variable_, _struct_name_)    \
         memset((_variable_), 0x0, sizeof(_struct_name_));    \
         (_variable_)->nSize = sizeof(_struct_name_);        \
         (_variable_)->nVersion.s.nVersionMajor = 0x1;    \
         (_variable_)->nVersion.s.nVersionMinor = 0x1;    \
         (_variable_)->nVersion.s.nRevision = 0x0;        \
         (_variable_)->nVersion.s.nStep = 0x0


/* Checking for version compliance.
  * If the nSize of the OMX structure is not set ,raises bad parameter error.
  * In case of version mismatch, raises a version mismatch error.
  */
#define OMX_CONF_CHK_VERSION(_s_, _name_, _e_)   \
        if((_s_)->nSize != sizeof(_name_)) _e_ = OMX_ErrorBadParameter; \
        if(((_s_)->nVersion.s.nVersionMajor != 0x1) ||  \
           ((_s_)->nVersion.s.nVersionMinor != 0x0) ||  \
           ((_s_)->nVersion.s.nRevision != 0x0) || \
           ((_s_)->nVersion.s.nStep != 0x0)) _e_ = OMX_ErrorVersionMismatch; \
        if(_e_ != OMX_ErrorNone) return _e_;

#define OMX_ALIGN(a, x) (a+(x-1))&(~(x-1))

#define OMX_TOSTRING(s) #s

#define STRINGIFY(x) case x: return #x

#define kInvalidTimeStamp  0xfffffffe

#endif
