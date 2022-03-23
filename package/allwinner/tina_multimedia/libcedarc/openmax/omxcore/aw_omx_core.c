/*
* Copyright (c) 2008-2018 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : Aw_omx_core.c
* Description :
* History :
*   Author  : AL3
*   Date    : 2013/05/05
*   Comment : complete the design code
*/

/*============================================================================
                            O p e n M A X   w r a p p e r s
                             O p e n  M A X   C o r e

  This module contains the implementation of the OpenMAX core.

*//*========================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <pthread.h>

#include "log.h"
#include "aw_omx_core.h"
#include "aw_omx_component.h"

#include "OMX_Core.h"
extern OmxCoreType core[];

extern const unsigned int NUM_OF_CORE;

static pthread_mutex_t g_mutex_core_info = PTHREAD_MUTEX_INITIALIZER;

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
static size_t strlcpy_local(char *dst, const char *src, size_t size)
{
    register char *pDst = dst;
    register const char *pSrc = src;
    register size_t nCpySize = size;

    /* Copy as many bytes as will fit */
    if (nCpySize != 0 && --nCpySize != 0) {
        do {
            if ((*pDst++ = *pSrc++) == 0)
                break;
        } while (--nCpySize != 0);
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (nCpySize == 0) {
        if (size != 0)
            *pDst = '\0';       /* NUL-terminate dst */
        while (*pSrc++)
            ;
    }

    return(pSrc - src - 1);    /* count does not include NUL */
}

static int getComponentIndex(char* comp_name)
{
    unsigned int i = 0;
    int rc = -1;
    pthread_mutex_lock(&g_mutex_core_info);

    for(i=0; i<NUM_OF_CORE; i++)
    {
        if(!strcmp(comp_name, core[i].name))
        {
            rc = i;
            break;
        }
    }
    pthread_mutex_unlock(&g_mutex_core_info);

    return rc;
}

static CreateOmxComponent loadComponentLib(int idx)
{
    CreateOmxComponent fn_ptr = NULL;

    pthread_mutex_lock(&g_mutex_core_info);

    if(core[idx].so_lib_handle == NULL)
    {
        core[idx].so_lib_handle = dlopen(core[idx].so_lib_name, RTLD_NOW);
    }

    if(core[idx].so_lib_handle)
    {
        if(core[idx].fn_ptr == NULL)
        {
            core[idx].fn_ptr = (CreateOmxComponent)dlsym(core[idx].so_lib_handle,
                            "AwOmxComponentCreate");
            if(core[idx].fn_ptr == NULL)
            {
                loge("Error: Library %s incompatible as OMX component loader - %s\n",
                      core[idx].so_lib_name, dlerror());
            }
        }
    }
    else
    {
        loge("Error: Couldn't load %s: %s\n",core[idx].so_lib_name,dlerror());
    }

    fn_ptr = core[idx].fn_ptr;

    pthread_mutex_unlock(&g_mutex_core_info);

    return fn_ptr;
}

static int setComponentHandle(char *cmp_name, void *handle)
{
    unsigned i=0,j=0;
    int rc = -1;

    pthread_mutex_lock(&g_mutex_core_info);

    for(i=0; i< NUM_OF_CORE; i++)
    {
        if(!strcmp(cmp_name, core[i].name))
        {
            for(j=0; j< OMX_COMP_MAX_INST; j++)
            {
                if(NULL == core[i].inst[j])
                {
                    rc = j;
                    OMX_LOGV("free handle slot exists %d\n", rc);

                    core[i].inst[j] = handle;
                    pthread_mutex_unlock(&g_mutex_core_info);
                    return rc;
                }
            }

            break;
        }
    }

    pthread_mutex_unlock(&g_mutex_core_info);

    return rc;
}

/* ======================================================================
FUNCTION
  checkLibUnload

DESCRIPTION
  Check if any component instance is using the library

PARAMETERS
  index: Component Index in core array.

RETURN VALUE
  1: Library Unused and can be unloaded.
  0:  Library used and shouldnt be unloaded.
========================================================================== */
static int checkLibUnload(int index)
{
    unsigned i=0;
    int rc = 1;

    pthread_mutex_lock(&g_mutex_core_info);

    for(i=0; i< OMX_COMP_MAX_INST; i++)
    {
        if(core[index].inst[i])
        {
            rc = 0;
            OMX_LOGV("Library Used\n");
            break;
        }
    }

    if(rc == 1)
    {
        logd(" Unloading the dynamic library for %s\n", core[index].name);
        int err = dlclose(core[index].so_lib_handle);
        if(err)
        {
            OMX_LOGV("Error %d in dlclose of lib %s\n", err,core[index].name);
        }

        core[index].so_lib_handle = NULL;
        core[index].fn_ptr = NULL;
    }

    pthread_mutex_unlock(&g_mutex_core_info);

    return rc;
}

/* ======================================================================
FUNCTION
  isCompHandleExists

DESCRIPTION
  Check if the component handle already exists or not.

PARAMETERS
  None

RETURN VALUE
  index pointer if the handle exists
  negative value otherwise
========================================================================== */
static int isCompHandleExists(OMX_HANDLETYPE inst)
{
    unsigned i=0,j=0;
    int rc = -1;

    if(NULL == inst)
        return rc;

    pthread_mutex_lock(&g_mutex_core_info);

    for(i=0; i< NUM_OF_CORE; i++)
    {
        for(j=0; j< OMX_COMP_MAX_INST; j++)
        {
            if(inst == core[i].inst[j])
            {
                rc = i;

                pthread_mutex_unlock(&g_mutex_core_info);
                return rc;
            }
        }
    }

    pthread_mutex_unlock(&g_mutex_core_info);

    return rc;
}

/* ======================================================================
FUNCTION
  clearCompHandle

DESCRIPTION
  Clears the component handle from the component table.

PARAMETERS
  None

RETURN VALUE
  None.
========================================================================== */
static void clearCompHandle(OMX_HANDLETYPE inst)
{
    unsigned i = 0, j = 0;

    if(NULL == inst)
        return;

    pthread_mutex_lock(&g_mutex_core_info);

    for(i=0; i< NUM_OF_CORE; i++)
    {
        for(j=0; j< OMX_COMP_MAX_INST; j++)
        {
            if(inst == core[i].inst[j])
            {
                core[i].inst[j] = NULL;

                pthread_mutex_unlock(&g_mutex_core_info);
                return;
            }
        }
    }

    pthread_mutex_unlock(&g_mutex_core_info);

    return;
}


/* ======================================================================
FUNCTION
  OMX_Init

DESCRIPTION
  This is the first function called by the application.
  There is nothing to do here since components shall be loaded
  whenever the get handle method is called.

PARAMETERS
  None

RETURN VALUE
  None.
========================================================================== */
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_Init()
{
    CEDARC_UNUSE(OMX_Init);
    OMX_LOGV("OMXCORE API - OMX_Init\n");

    /* Nothing to do here ; shared objects shall be loaded at the get handle method */
    return OMX_ErrorNone;
}

/* ======================================================================
FUNCTION
  OMX_DeInit

DESCRIPTION
  DeInitialize all the the relevant OMX components.

PARAMETERS
  None

RETURN VALUE
  Error None.
========================================================================== */
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_Deinit()
{
    CEDARC_UNUSE(OMX_Deinit);
#if 0

    int err;
    unsigned i=0,j=0;
    OMX_ERRORTYPE eRet;

    /* Free the dangling handles here if any */
    for(i=0; i< NUM_OF_CORE; i++)
    {
        for(j=0; j< OMX_COMP_MAX_INST; j++)
        {
            if(core[i].inst[j])
            {
                OMX_LOGV("OMX DeInit: Freeing handle for %s\n", core[i].name);

                /* Release the component and unload dynmaic library */
                eRet = OMX_FreeHandle(core[i].inst[j]);
                if(eRet != OMX_ErrorNone)
                    return eRet;
            }
        }
    }
#endif

    return OMX_ErrorNone;
}


/* ======================================================================
FUNCTION
  OMX_GetHandle

DESCRIPTION
  Constructs requested component. Relevant library is loaded if needed.

PARAMETERS
  None

RETURN VALUE
  Error None  if everything goes fine.
========================================================================== */
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_GetHandle(OMX_OUT OMX_HANDLETYPE* handle,
                                                   OMX_IN OMX_STRING componentName,
                                                   OMX_IN OMX_PTR appData,
                                                   OMX_IN OMX_CALLBACKTYPE* callBacks)
{
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    CreateOmxComponent fn_create_ptr = NULL;
    int comp_index = -1;

    if(handle)
    {
        comp_index = getComponentIndex(componentName);

        if(comp_index >= 0)
        {
            OMX_LOGV("getting fn pointer\n");

            fn_create_ptr = loadComponentLib(comp_index);
            if(fn_create_ptr)
            {
                void* pThis = (*fn_create_ptr)();
                if(pThis)
                {
                    eRet = AwOmxComponentInit(pThis, componentName);
                    if(eRet != OMX_ErrorNone)
                    {
                        logd("Component not created succesfully\n");
                        return eRet;
                    }

                    AwOmxComponentSetCallbacks(pThis, callBacks, appData);

                    int hnd_index = setComponentHandle(componentName, pThis);
                    if(hnd_index >= 0)
                    {
                        *handle = (OMX_HANDLETYPE)pThis;
                    }
                    else
                    {
                        *handle = NULL;
                        AwOmxComponentDeinit(pThis);
                        checkLibUnload(comp_index);
                        return OMX_ErrorInsufficientResources;
                    }
                }
                else
                {
                    eRet = OMX_ErrorInsufficientResources;
                    logd("Component Creation failed\n");
                }

            }
            else
            {
                eRet = OMX_ErrorNotImplemented;
                OMX_LOGV("library couldnt return create instance fn\n");
            }
        }
        else
        {
            eRet = OMX_ErrorNotImplemented;
            OMX_LOGV("ERROR: Already another instance active  ;rejecting\n");
        }

    }
    else
    {
        eRet =  OMX_ErrorBadParameter;
        OMX_LOGV("\n OMX_GetHandle: NULL handle\n");
    }

    return eRet;
}

/* ======================================================================
FUNCTION
  OMX_FreeHandle

DESCRIPTION
  Destructs the component handles.

PARAMETERS
  None

RETURN VALUE
  Error None.
========================================================================== */
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_FreeHandle(OMX_IN OMX_HANDLETYPE hComp)
{
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    int i = 0;

    // 0. Check that we have an active instance
    i = isCompHandleExists(hComp);
    if(i >= 0)
    {
        eRet = AwOmxComponentDeinit(hComp);
        if(eRet == OMX_ErrorNone)
        {
            clearCompHandle(hComp);
            checkLibUnload(i);
        }
        else
        {
            loge(" OMX_FreeHandle failed on %x\n",(unsigned) hComp);
            return eRet;
        }
    }
    else
    {
        logw("OMXCORE Warning: Free Handle called with no active instances\n");
    }

    return OMX_ErrorNone;
}

/* ======================================================================
FUNCTION
  OMX_SetupTunnel

DESCRIPTION
  Not Implemented.

PARAMETERS
  None

RETURN VALUE
  None.
========================================================================== */
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_SetupTunnel(OMX_IN OMX_HANDLETYPE outputComponent,
                                                   OMX_IN OMX_U32 outputPort,
                                                   OMX_IN OMX_HANDLETYPE inputComponent,
                                                   OMX_IN OMX_U32 inputPort)
{
    CEDARC_UNUSE(OMX_SetupTunnel);
    /* Not supported right now */
    OMX_LOGV("OMXCORE API: OMX_SetupTunnel Not implemented\n");

    CEDARC_UNUSE(outputComponent);
    CEDARC_UNUSE(inputComponent);
    CEDARC_UNUSE(outputPort);
    CEDARC_UNUSE(inputPort);
    return OMX_ErrorNotImplemented;
}

/* ======================================================================
FUNCTION
  OMX_GetContentPipe

DESCRIPTION
  Not Implemented.

PARAMETERS
  None

RETURN VALUE
  None.
========================================================================== */
OMX_API OMX_ERRORTYPE OMX_GetContentPipe(OMX_OUT OMX_HANDLETYPE* pipe, OMX_IN OMX_STRING uri)
{
    CEDARC_UNUSE(OMX_GetContentPipe);
    /* Not supported right now */
    OMX_LOGV("OMXCORE API: OMX_GetContentPipe Not implemented \n");

    CEDARC_UNUSE(pipe);
    CEDARC_UNUSE(uri);
    return OMX_ErrorNotImplemented;
}

/* ======================================================================
FUNCTION
  OMX_GetComponentNameEnum

DESCRIPTION
  Returns the component name associated with the index.

PARAMETERS
  None

RETURN VALUE
  None.
========================================================================== */
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_ComponentNameEnum(OMX_OUT OMX_STRING componentName,
                                                         OMX_IN OMX_U32 nameLen,
                                                         OMX_IN OMX_U32 index)
{
    CEDARC_UNUSE(OMX_ComponentNameEnum);

    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    OMX_LOGV("OMXCORE API - OMX_ComponentNameEnum %x %d %d\n",
         (unsigned) componentName, (unsigned)nameLen, (unsigned)index);
    if(index < NUM_OF_CORE)
    {
        strlcpy_local(componentName, core[index].name, nameLen);
    }
    else
    {
        eRet = OMX_ErrorNoMore;
    }

    return eRet;
}

/* ======================================================================
FUNCTION
  OMX_GetComponentsOfRole

DESCRIPTION
  Returns the component name which can fulfill the roles passed in the
  argument.

PARAMETERS
  None

RETURN VALUE
  None.
========================================================================== */
OMX_API OMX_ERRORTYPE OMX_GetComponentsOfRole(OMX_IN OMX_STRING role,
                                               OMX_INOUT OMX_U32* numComps,
                                               OMX_INOUT OMX_U8** compNames)
{
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    unsigned i,j;

    if(compNames == NULL)
    {
        if(numComps == NULL)
        {
            eRet = OMX_ErrorBadParameter;
        }
        else
        {
            *numComps = 0;
            for (i=0; i<NUM_OF_CORE;i++)
            {
                for(j=0; j<OMX_CORE_MAX_CMP_ROLES && core[i].roles[j] ; j++)
                {
                    if(!strcmp(role,core[i].roles[j]))
                    {
                        (*numComps)++;
                    }
                }
            }
        }

        return eRet;
    }

    if(numComps)
    {
        unsigned namecount = *numComps;

        if(namecount == 0)
        {
            return OMX_ErrorBadParameter;
        }

        *numComps = 0;

        for (i=0; i<NUM_OF_CORE;i++)
        {
            for(j=0; j<OMX_CORE_MAX_CMP_ROLES && core[i].roles[j] ; j++)
            {
                if(!strcmp(role, core[i].roles[j]))
                {
                    strlcpy_local((char *)compNames[*numComps], core[i].name,
                                   OMX_MAX_STRINGNAME_SIZE);
                    (*numComps)++;
                    break;
                }
            }

            if (*numComps == namecount)
            {
                break;
            }
        }
    }
    else
    {
        eRet = OMX_ErrorBadParameter;
    }

    printf(" Leaving OMX_GetComponentsOfRole\n");
    return eRet;
}

/* ======================================================================
FUNCTION
  OMX_GetRolesOfComponent

DESCRIPTION
  Returns the primary role of the components supported.

PARAMETERS
  None

RETURN VALUE
  None.
========================================================================== */
OMX_API OMX_ERRORTYPE OMX_GetRolesOfComponent(OMX_IN OMX_STRING compName,
                                              OMX_INOUT OMX_U32* numRoles,
                                              OMX_OUT OMX_U8** roles)
{
    /* Not supported right now */
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    unsigned i,j;
    OMX_LOGV("GetRolesOfComponent %s\n",compName);

    if (roles == NULL)
    {
        if (numRoles == NULL)
        {
            eRet = OMX_ErrorBadParameter;
        }
        else
        {
            *numRoles = 0;
            for(i=0; i< NUM_OF_CORE; i++)
            {
                if(!strcmp(compName,core[i].name))
                {
                    for(j=0; (j<OMX_CORE_MAX_CMP_ROLES) && core[i].roles[j];j++)
                    {
                        (*numRoles)++;
                    }

                    break;
                }
            }
        }

        return eRet;
    }

    if(numRoles)
    {
        if (*numRoles == 0)
        {
            return OMX_ErrorBadParameter;
        }

        unsigned numofroles = *numRoles;
        *numRoles = 0;

        for(i=0; i< NUM_OF_CORE; i++)
        {
            if(!strcmp(compName,core[i].name))
            {
                for(j=0; (j<OMX_CORE_MAX_CMP_ROLES) && core[i].roles[j];j++)
                {
                    if(roles && roles[*numRoles])
                    {
                        strlcpy_local((char *)roles[*numRoles],core[i].roles[j],
                                       OMX_MAX_STRINGNAME_SIZE);
                    }

                    (*numRoles)++;
                    if (numofroles == *numRoles)
                    {
                        break;
                    }
                }

                break;
            }
        }
    }
    else
    {
        OMX_LOGV("ERROR: Both Roles and numRoles Invalid\n");
        eRet = OMX_ErrorBadParameter;
    }

    return eRet;
}
