#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include "OmxCodec.h"
#include "cdx_log.h"
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

typedef struct OMX_PARAM_BUFFERADDRESS
{
    OMX_U32 nSize; /**< size of the structure in bytes */
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_PTR* pData;
    OMX_PTR* pDataPhy;;   /**< buffer physic address */
} OMX_PARAM_BUFFERADDRESS;

typedef enum OmxPortType
{
    OMX_PORT_INPUT,
    OMX_PORT_OUTPUT
} OmxPortType;

typedef struct AW_OMX_PLUGIN
{
    void *mLibHandle;
    OMX_ERRORTYPE (*mInit)();
    OMX_ERRORTYPE (*mDeinit)();
    OMX_ERRORTYPE (*mGetHandle)(OMX_HANDLETYPE *, OMX_STRING, OMX_PTR, OMX_CALLBACKTYPE *);
    OMX_ERRORTYPE (*mFreeHandle)(OMX_HANDLETYPE *);
} AW_OMX_PLUGIN;

typedef struct OmxPort
{
    void *core;
    OmxPortType type;

    unsigned int num_buffers;
    unsigned int buffer_size;
    unsigned int port_index;
    OMX_BUFFERHEADERTYPE **buffers;

    pthread_mutex_t mutex;
    int enabled;
    int omx_allocate;   /**< Setup with OMX_AllocateBuffer rather than OMX_UseBuffer */
    AsyncQueue *queue;
} OmxPort;


typedef struct OmxCore
{
    void* object;
    OMX_HANDLETYPE omx_handle;
    OMX_ERRORTYPE omx_error;

    OMX_STATETYPE omx_state;
    pthread_cond_t omx_state_condition;
    pthread_mutex_t omx_state_mutex;

    OmxPort *ports;
    int ports_num;

    OMXSem *done_sem;
    OMXSem *flush_sem;
    OMXSem *port_sem;

#if 0
    OmxCb settings_changed_cb;
#endif

    AW_OMX_PLUGIN *imp;
    int done;
    char library_name[64];
    char component_name[64];
    char component_role[64];
} OmxCore;

static AW_OMX_PLUGIN * st_plugin = NULL;
static int clientnum = 0;

/* Functions. */

AW_OMX_PLUGIN *LoadOmxPlugin(const char* libName);
void  UnLoadOmxPlugin(void *handle);
OMX_ERRORTYPE MakeComponentInstance(AW_OMX_PLUGIN* plugin, char* name,
                                    OMX_CALLBACKTYPE* callbacks,
                                    OMX_PTR appData, OMX_HANDLETYPE* pHandle);

OMX_ERRORTYPE DestroyComponentInstance(AW_OMX_PLUGIN* plugin, OMX_HANDLETYPE handle);

OmxCore *omx_core_new (void *object);
void omx_core_free (OmxCore * core);
void omx_core_init (OmxCore * core, char* component_name, char* component_role);
void omx_core_deinit (OmxCore * core);

void omx_core_prepare (OmxCore * core);
void omx_core_start (OmxCore * core);
void omx_core_pause (OmxCore * core);
void omx_core_stop (OmxCore * core);
void omx_core_unload (OmxCore * core);
void omx_core_set_done (OmxCore * core);
void omx_core_wait_for_done (OmxCore * core);
void omx_core_flush_start (OmxCore * core);
void omx_core_flush_stop (OmxCore * core);
void omx_port_init (OmxCore * core, unsigned int index);


void omx_port_free (OmxPort * port);
void omx_port_setup (OmxPort * port);
void omx_port_push_buffer (OmxPort * port, OMX_BUFFERHEADERTYPE * omx_buffer);
OMX_BUFFERHEADERTYPE *omx_port_request_buffer (OmxPort * port);
void omx_port_release_buffer (OmxPort * port, OMX_BUFFERHEADERTYPE * omx_buffer);
void omx_port_resume (OmxPort * port);
void omx_port_pause (OmxPort * port);
void omx_port_flush (OmxPort * port);
void omx_port_enable (OmxPort * port);
void omx_port_disable (OmxPort * port);
void omx_port_finish (OmxPort * port);

static inline void change_state (OmxCore * core, OMX_STATETYPE state);
static inline void wait_for_state (OmxCore * core, OMX_STATETYPE state);
static void port_allocate_buffers (OmxPort * port);
static void port_start_buffers (OmxPort * port);
static void port_free_buffers (OmxPort * port);

static inline void got_buffer (OmxCore * core, OmxPort * port, OMX_BUFFERHEADERTYPE * omx_buffer);
static inline OmxPort *get_port(OmxCore * core, unsigned int index);

static OMX_ERRORTYPE EventHandler (OMX_HANDLETYPE omx_handle,OMX_PTR app_data,
                                   OMX_EVENTTYPE event, OMX_U32 data_1,
                                   OMX_U32 data_2, OMX_PTR event_data);

static OMX_ERRORTYPE EmptyBufferDone (OMX_HANDLETYPE omx_handle,
                                      OMX_PTR app_data, OMX_BUFFERHEADERTYPE * omx_buffer);

static OMX_ERRORTYPE FillBufferDone (OMX_HANDLETYPE omx_handle,
                                     OMX_PTR app_data, OMX_BUFFERHEADERTYPE * omx_buffer);


//* callback function
static OMX_CALLBACKTYPE callbacks =
{ EventHandler, EmptyBufferDone, FillBufferDone };


static inline const char *omx_state_to_str (OMX_STATETYPE omx_state);
static inline const char *omx_error_to_str (OMX_ERRORTYPE omx_error);


typedef OMX_ERRORTYPE (*InitFunc)();
typedef OMX_ERRORTYPE (*DeinitFunc)();
typedef OMX_ERRORTYPE (*GetHandleFunc)(OMX_HANDLETYPE *, OMX_STRING, OMX_PTR, OMX_CALLBACKTYPE *);
typedef OMX_ERRORTYPE (*FreeHandleFunc)(OMX_HANDLETYPE *);


#define OMX_INIT_PARAM(param) {                                  \
    memset (&(param), 0, sizeof ((param)));                      \
    (param).nSize = sizeof (param);                              \
    (param).nVersion.s.nVersionMajor = 1;                        \
    (param).nVersion.s.nVersionMinor = 1;                        \
}


typedef void (*OmxPortFunc) (OmxPort * port);

static inline void core_for_each_port (OmxCore * core, OmxPortFunc func)
{
    int index;

    for (index = 0; index < core->ports_num; index++)
    {
        OmxPort *port;
        port = get_port (core, index);

        if (port)
            func (port);
    }
}


AW_OMX_PLUGIN *LoadOmxPlugin(const char* libName)
{
    if(clientnum == 0)
    {

        AW_OMX_PLUGIN* omxPlugIn = (AW_OMX_PLUGIN*)malloc(sizeof(AW_OMX_PLUGIN));
        memset(omxPlugIn, 0, sizeof(AW_OMX_PLUGIN));
        logd("LoadOmxPlugin load lib %s",libName);
        omxPlugIn->mLibHandle = dlopen(libName, RTLD_NOW);

        if(omxPlugIn->mLibHandle != NULL)
        {
            omxPlugIn->mInit       = (InitFunc)dlsym(omxPlugIn->mLibHandle, "OMX_Init");
            omxPlugIn->mDeinit     = (DeinitFunc)dlsym(omxPlugIn->mLibHandle, "OMX_Deinit");
            omxPlugIn->mGetHandle  = (GetHandleFunc)dlsym(omxPlugIn->mLibHandle, "OMX_GetHandle");
            omxPlugIn->mFreeHandle = (FreeHandleFunc)dlsym(omxPlugIn->mLibHandle, "OMX_FreeHandle");
            (*omxPlugIn->mInit)();
        }
        else
        {
            logd("open lib %s failed", libName);
            free(omxPlugIn);
            return NULL;
        }

        st_plugin = omxPlugIn;
        clientnum++;
        return omxPlugIn;
    }
    else
    {
        return st_plugin;
    }
}

void  UnLoadOmxPlugin(void *handle)
{
    if(clientnum > 0)
    {
        clientnum--;
        if(clientnum == 0)
        {
            st_plugin = NULL;
            AW_OMX_PLUGIN* omxPlugIn = (AW_OMX_PLUGIN*)handle;
            if(omxPlugIn->mLibHandle != NULL)
            {
                (*omxPlugIn->mDeinit)();
                dlclose(omxPlugIn->mLibHandle);
                omxPlugIn->mLibHandle = NULL;
            }
        }
    }
}


OMX_ERRORTYPE MakeComponentInstance(AW_OMX_PLUGIN* plugin, char* name,
                                    OMX_CALLBACKTYPE* callbacks,
                                    OMX_PTR appData, OMX_HANDLETYPE* pHandle)
{
    if(!plugin->mLibHandle)
    {
        logd("OMX_ErrorUndefined");
        return OMX_ErrorUndefined;
    }

    logd(" ok !");
    return (*plugin->mGetHandle)(pHandle,
    name,
    appData,
    callbacks);
}


OMX_ERRORTYPE DestroyComponentInstance(AW_OMX_PLUGIN* plugin, OMX_HANDLETYPE handle)
{
    if(handle == NULL)
    {
        return OMX_ErrorUndefined;
    }

    return (*plugin->mFreeHandle)(&handle);
}


/*
 * Core
 */


OmxCore * omx_core_new(void *object)
{
    OmxCore *core;

    core = (OmxCore *)malloc(sizeof(OmxCore));

    if(!core)
    {
        loge("omx_core_new failed");
        return NULL;
    }

    memset(core, 0, sizeof(OmxCore));
    core->object = object;

    core->ports_num = 2;
    core->ports = (OmxPort*)malloc(sizeof(OmxPort)*2);

    memset(core->ports, 0, sizeof(OmxPort)*2);

    logi("core->ports: %p", core->ports);

    //* init port
    omx_port_init(core, OMX_PORT_INPUT);
    omx_port_init(core, OMX_PORT_OUTPUT);

    pthread_cond_init(&core->omx_state_condition, NULL);
    pthread_mutex_init(&core->omx_state_mutex, NULL);

    core->done_sem = t_sem_new ();
    core->flush_sem = t_sem_new ();
    core->port_sem = t_sem_new ();

    core->omx_state = OMX_StateInvalid;

    return core;
}


void omx_core_free (OmxCore * core)
{
    t_sem_free (core->port_sem);
    t_sem_free (core->flush_sem);
    t_sem_free (core->done_sem);

    pthread_cond_destroy(&core->omx_state_condition);
    pthread_mutex_destroy(&core->omx_state_mutex);

    omx_port_free (&core->ports[OMX_PORT_INPUT]);
    omx_port_free (&core->ports[OMX_PORT_OUTPUT]);
    free(core->ports);
    free(core);
}


void omx_core_init (OmxCore * core, char* component_name, char* component_role)
{
    if (!core)
        return;

    strcpy ((char *) core->component_name, component_name);
    strcpy ((char *) core->component_role, component_role);

    // TODO may modify this later
    strcpy ((char *) core->library_name, "libOmxCore.so");

    logi("omx_core_init, loading: %s %s",
    (char*)core->component_name,
    (char*)core->component_role);

    core->imp = LoadOmxPlugin(core->library_name);

    core->omx_error = MakeComponentInstance(core->imp,
    (char *)core->component_name,
    &callbacks,
    core,
    &core->omx_handle);

    logi("OMX_GetHandle( %p), core->omx_error: %d", core->omx_handle, core->omx_error);

    if (!core->omx_error)
    {
        core->omx_state = OMX_StateLoaded;

        OMX_PARAM_COMPONENTROLETYPE param;

        logi("setting component role: %s", core->component_role);

        OMX_INIT_PARAM(param);

        strcpy((char *) param.cRole, core->component_role);
        OMX_SetParameter (core->omx_handle, OMX_IndexParamStandardComponentRole, &param);
    }

    //* set omx port info
    omx_port_setup(&(core->ports[OMX_PORT_INPUT]));
    omx_port_setup(&(core->ports[OMX_PORT_OUTPUT]));

    if(!strncmp("OMX.allwinner.video.decoder", (char *)core->component_name, 27))
    {
        OmxPort * outport = &core->ports[OMX_PORT_OUTPUT];
        outport->omx_allocate = 1;
    }
}


void omx_core_deinit (OmxCore * core)
{
    if (!core->imp)
        return;

    if (core->omx_state == OMX_StateLoaded || core->omx_state == OMX_StateInvalid)
    {
        if (core->omx_handle)
        {
            DestroyComponentInstance(core->imp, core->omx_handle);
            logd("OMX_FreeHandle(%p) -> %d",
            core->omx_handle, core->omx_error);
        }
    }
    else
    {
        loge("Incorrect state: %s", omx_state_to_str (core->omx_state));
    }

    UnLoadOmxPlugin(core->imp);
    core->imp = NULL;
}


void omx_core_prepare (OmxCore * core)
{
    change_state (core, OMX_StateIdle);

    /* Allocate buffers. */
    core_for_each_port (core, port_allocate_buffers);


    wait_for_state (core, OMX_StateIdle);
}


void omx_core_start (OmxCore * core)
{
    change_state (core, OMX_StateExecuting);
    wait_for_state (core, OMX_StateExecuting);

    if (core->omx_state == OMX_StateExecuting)
        core_for_each_port (core, port_start_buffers);
}


void omx_core_stop (OmxCore * core)
{
    if (core->omx_state == OMX_StateExecuting ||
    core->omx_state == OMX_StatePause)
    {
        change_state (core, OMX_StateIdle);
        wait_for_state (core, OMX_StateIdle);
    }
}

void omx_core_pause (OmxCore * core)
{
    change_state (core, OMX_StatePause);
    wait_for_state (core, OMX_StatePause);
}

void omx_core_unload (OmxCore * core)
{
    if (core->omx_state == OMX_StateIdle ||
    core->omx_state == OMX_StateWaitForResources ||
    core->omx_state == OMX_StateInvalid)
    {
        if (core->omx_state != OMX_StateInvalid)
            change_state (core, OMX_StateLoaded);

        core_for_each_port (core, port_free_buffers);

        if (core->omx_state != OMX_StateInvalid)
            wait_for_state (core, OMX_StateLoaded);
    }

    core_for_each_port (core, omx_port_free);
    //ptr_array_clear (core->ports);
}


static inline OmxPort *get_port(OmxCore * core, unsigned int index)
{
    return &core->ports[index];
}


void omx_core_set_done (OmxCore * core)
{
    t_sem_up (core->done_sem);
}

void omx_core_wait_for_done (OmxCore * core)
{
    t_sem_down (core->done_sem);
}

void omx_core_flush_start (OmxCore * core)
{
    core_for_each_port (core, omx_port_pause);
}

void omx_core_flush_stop (OmxCore * core)
{
    core_for_each_port (core, omx_port_flush);
    core_for_each_port (core, omx_port_resume);
}

/*
 * Port
 */

void omx_port_init (OmxCore * core, unsigned int index)
{
    OmxPort *port;

    if(index == OMX_PORT_INPUT)
    {
        port = &core->ports[OMX_PORT_INPUT];
    }
    else
    {
        port = &core->ports[OMX_PORT_OUTPUT];
    }

    port->core = core;
    port->port_index = index;
    port->num_buffers = 0;
    port->buffer_size = 0;
    port->buffers = NULL;

    port->enabled = 1;
    port->queue = async_queue_new();

    pthread_mutex_init(&port->mutex, NULL);
}

void omx_port_free (OmxPort * port)
{
    pthread_mutex_destroy(&port->mutex);
    async_queue_free (port->queue);

    free(port->buffers);
}

void omx_port_setup (OmxPort * port)
{
    OmxPortType type;
    OMX_PARAM_PORTDEFINITIONTYPE param;
    OMX_INIT_PARAM (param);
    param.nPortIndex = port->port_index;
    OMX_GetParameter(((OmxCore *)(port->core))->omx_handle, OMX_IndexParamPortDefinition,
    &param);

    switch (param.eDir)
    {
    case OMX_DirInput:
        type = OMX_PORT_INPUT;
        break;
    case OMX_DirOutput:
        type = OMX_PORT_OUTPUT;
        break;
    default:
        return ;
        //break;
    }
    sleep(1);
    logd("omx_port_setup");

    port->type = type;
    /** @todo should it be nBufferCountMin? */
    port->num_buffers = param.nBufferCountActual;
    port->buffer_size = param.nBufferSize;

    logi("type=%d, num_buffers=%d, buffer_size=%d, port_index=%d",
    port->type, port->num_buffers, port->buffer_size, port->port_index);

#if 0
    if(!port->buffers)
        free (port->buffers);
#endif
    logd("omx_port_setup");
    port->buffers =
        (OMX_BUFFERHEADERTYPE **)malloc(sizeof(OMX_BUFFERHEADERTYPE *)*port->num_buffers);
}


static void port_allocate_buffers (OmxPort * port)
{
    unsigned int i;
    unsigned int size;
    OMX_PARAM_PORTDEFINITIONTYPE param;

    param.nPortIndex = OMX_DirInput;
    OMX_GetParameter(((OmxCore *)(port->core))->omx_handle, OMX_IndexParamPortDefinition,
    &param);

    port->buffer_size = param.nBufferSize;
    size = port->buffer_size;

    for (i = 0; i < port->num_buffers; i++)
    {
        if (port->omx_allocate)
        {
            logv("OMX_AllocateBuffer(),i: %d size=%d" , i, size);
            OMX_AllocateBuffer (((OmxCore *)(port->core))->omx_handle, &port->buffers[i],
            port->port_index, NULL, size);
        }
        else
        {
            void* buffer_data = NULL;
            buffer_data = (void*)malloc(size);
            logv("%d: fangning, OMX_UseBuffer(), size=%d, %p",
                 i, size, ((OmxCore *)(port->core))->omx_handle);

            OMX_UseBuffer (((OmxCore *)(port->core))->omx_handle, &port->buffers[i],
            port->port_index, NULL, size, (OMX_U8*)buffer_data);
        }
    }
}


static void port_free_buffers (OmxPort * port)
{
    unsigned int i;

    for (i = 0; i < port->num_buffers; i++)
    {
        OMX_BUFFERHEADERTYPE *omx_buffer;

        omx_buffer = port->buffers[i];

        if (omx_buffer)
        {
            OMX_FreeBuffer (((OmxCore *)(port->core))->omx_handle, port->port_index, omx_buffer);
            port->buffers[i] = NULL;
        }
    }
}


static void port_start_buffers (OmxPort * port)
{
    unsigned int i;

    for (i = 0; i < port->num_buffers; i++)
    {
        OMX_BUFFERHEADERTYPE *omx_buffer;

        omx_buffer = port->buffers[i];

        /* If it's an input port we will need to fill the buffer, so put it in
         * the queue, otherwise send to omx for processing (fill it up). */
        if (port->type == OMX_PORT_INPUT)
            got_buffer((OmxCore *)port->core, port, omx_buffer);
        else
            omx_port_release_buffer (port, omx_buffer);
    }
}

void omx_port_push_buffer (OmxPort * port, OMX_BUFFERHEADERTYPE * omx_buffer)
{
    async_queue_push (port->queue, (void*)omx_buffer);
}


OMX_BUFFERHEADERTYPE *omx_port_request_buffer (OmxPort * port)
{
    return (OMX_BUFFERHEADERTYPE *)(async_queue_pop (port->queue));
    //return (OMX_BUFFERHEADERTYPE *)(async_queue_pop_forced(port->queue));
}

void omx_port_release_buffer (OmxPort * port, OMX_BUFFERHEADERTYPE * omx_buffer)
{
    switch (port->type)
    {
    case OMX_PORT_INPUT:
        OMX_EmptyThisBuffer (((OmxCore *)port->core)->omx_handle, omx_buffer);
        break;
    case OMX_PORT_OUTPUT:
        OMX_FillThisBuffer (((OmxCore *)port->core)->omx_handle, omx_buffer);
        break;
    default:
        break;
    }
}

void omx_port_resume (OmxPort * port)
{
    async_queue_enable (port->queue);
}

void omx_port_pause (OmxPort * port)
{
    async_queue_disable (port->queue);
}

void omx_port_flush (OmxPort * port)
{
    if (port->type == OMX_PORT_OUTPUT)
    {
        OMX_BUFFERHEADERTYPE *omx_buffer;
        while ((omx_buffer = (OMX_BUFFERHEADERTYPE *)async_queue_pop_forced (port->queue)))
        {
            omx_buffer->nFilledLen = 0;
            omx_port_release_buffer (port, omx_buffer);
        }
    }
    else
    {
        OMX_SendCommand (((OmxCore *)port->core)->omx_handle, OMX_CommandFlush, port->port_index,
        NULL);
        t_sem_down (((OmxCore *)port->core)->flush_sem);
    }
}


void omx_port_enable (OmxPort * port)
{
    OmxCore *core = (OmxCore *)port->core;

    OMX_SendCommand (core->omx_handle, OMX_CommandPortEnable, port->port_index,
    NULL);
    port_allocate_buffers (port);
    if (core->omx_state != OMX_StateLoaded)
        port_start_buffers (port);
    omx_port_resume (port);

    t_sem_down (core->port_sem);
}

void g_omx_port_disable (OmxPort * port)
{
    OmxCore *core = (OmxCore *)(port->core);

    OMX_SendCommand (core->omx_handle, OMX_CommandPortDisable, port->port_index,
    NULL);
    omx_port_pause (port);
    omx_port_flush (port);
    port_free_buffers (port);

    t_sem_down (core->port_sem);
}


void omx_port_finish (OmxPort * port)
{
    port->enabled = 0;
    async_queue_disable (port->queue);
}

/*
 * Helper functions.
 */

static inline void change_state (OmxCore * core, OMX_STATETYPE state)
{
    OMX_SendCommand (core->omx_handle, OMX_CommandStateSet, state, NULL);
}

static inline void complete_change_state (OmxCore * core, OMX_STATETYPE state)
{
    pthread_mutex_lock(&core->omx_state_mutex);

    core->omx_state = state;
    pthread_cond_signal (&core->omx_state_condition);

    pthread_mutex_unlock (&core->omx_state_mutex);
}

static inline void wait_for_state (OmxCore * core, OMX_STATETYPE state)
{
    pthread_mutex_lock (&core->omx_state_mutex);

    if (core->omx_state != state)
    {

        pthread_cond_wait(&core->omx_state_condition, &core->omx_state_mutex);
    }

    pthread_mutex_unlock (&core->omx_state_mutex);
}

static inline void got_buffer (OmxCore * core, OmxPort * port, OMX_BUFFERHEADERTYPE * omx_buffer)
{
    if (!omx_buffer)
    {
        return;
    }

    if (port)
    {
        omx_port_push_buffer (port, omx_buffer);

        switch (port->type)
        {
        case OMX_PORT_INPUT:
            //in_port_cb (port, omx_buffer);
            break;
        case OMX_PORT_OUTPUT:
            //out_port_cb (port, omx_buffer);
            break;
        default:
            break;
        }
    }
}



static OMX_ERRORTYPE EventHandler (OMX_HANDLETYPE omx_handle,
OMX_PTR app_data,
OMX_EVENTTYPE event, OMX_U32 data_1, OMX_U32 data_2, OMX_PTR event_data)
{
    OmxCore *core = (OmxCore *) app_data;

    switch (event)
    {
    case OMX_EventCmdComplete:
    {
        OMX_COMMANDTYPE cmd;

        cmd = (OMX_COMMANDTYPE) data_1;

        logv("OMX_EventCmdComplete: %d", cmd);

        switch (cmd)
        {
        case OMX_CommandStateSet:
            complete_change_state (core, (OMX_STATETYPE)data_2);
            break;
        case OMX_CommandFlush:
            t_sem_up (core->flush_sem);
            break;
        case OMX_CommandPortDisable:
        case OMX_CommandPortEnable:
            t_sem_up (core->port_sem);
        default:
            break;
        }
        break;
    }
    case OMX_EventBufferFlag:
    {
        if (data_2 & OMX_BUFFERFLAG_EOS)
        {
            omx_core_set_done (core);
        }

        break;
    }
    case OMX_EventPortSettingsChanged:
    {
        //*to do something
        break;
    }
    case OMX_EventError:
    {
        core->omx_error = (OMX_ERRORTYPE)data_1;
        loge("unrecoverable error: %s (0x%lx)",
        omx_error_to_str ((OMX_ERRORTYPE)data_1), data_1);
        /* component might leave us waiting for buffers, unblock */
        omx_core_flush_start (core);
        /* unlock wait_for_state */
        pthread_mutex_lock (&core->omx_state_mutex);
        pthread_cond_signal (&core->omx_state_condition);
        pthread_mutex_unlock (&core->omx_state_mutex);

        break;
    }
    default:
        break;
    }

    return OMX_ErrorNone;
}


static OMX_ERRORTYPE EmptyBufferDone (OMX_HANDLETYPE omx_handle,
OMX_PTR app_data, OMX_BUFFERHEADERTYPE * omx_buffer)
{
    OmxCore *core = (OmxCore *) app_data;
    OmxPort *port = get_port(core, omx_buffer->nInputPortIndex);
    got_buffer (core, port, omx_buffer);

    return OMX_ErrorNone;
}


static OMX_ERRORTYPE FillBufferDone (OMX_HANDLETYPE omx_handle,
OMX_PTR app_data, OMX_BUFFERHEADERTYPE * omx_buffer)
{
    OmxCore *core = (OmxCore *) app_data;
    OmxPort *port = get_port (core, omx_buffer->nOutputPortIndex);

    got_buffer (core, port, omx_buffer);

    return OMX_ErrorNone;
}


static inline const char* omx_state_to_str (OMX_STATETYPE omx_state)
{
    switch (omx_state)
    {
    case OMX_StateInvalid:
        return "invalid";
    case OMX_StateLoaded:
        return "loaded";
    case OMX_StateIdle:
        return "idle";
    case OMX_StateExecuting:
        return "executing";
    case OMX_StatePause:
        return "pause";
    case OMX_StateWaitForResources:
        return "wait for resources";
    default:
        return "unknown";
    }
}

static inline const char * omx_error_to_str (OMX_ERRORTYPE omx_error)
{
    switch (omx_error)
    {
    case OMX_ErrorNone:
        return "None";

    case OMX_ErrorInsufficientResources:
        return
        "There were insufficient resources to perform the requested operation";

    case OMX_ErrorUndefined:
        return "The cause of the error could not be determined";

    case OMX_ErrorInvalidComponentName:
        return "The component name string was not valid";

    case OMX_ErrorComponentNotFound:
        return "No component with the specified name string was found";

    case OMX_ErrorInvalidComponent:
        return "The component specified did not have an entry point";

    case OMX_ErrorBadParameter:
        return "One or more parameters were not valid";

    case OMX_ErrorNotImplemented:
        return "The requested function is not implemented";

    case OMX_ErrorUnderflow:
        return "The buffer was emptied before the next buffer was ready";

    case OMX_ErrorOverflow:
        return "The buffer was not available when it was needed";

    case OMX_ErrorHardware:
        return "The hardware failed to respond as expected";

    case OMX_ErrorInvalidState:
        return "The component is in invalid state";

    case OMX_ErrorStreamCorrupt:
        return "Stream is found to be corrupt";

    case OMX_ErrorPortsNotCompatible:
        return "Ports being connected are not compatible";

    case OMX_ErrorResourcesLost:
        return "Resources allocated to an idle component have been lost";

    case OMX_ErrorNoMore:
        return "No more indices can be enumerated";

    case OMX_ErrorVersionMismatch:
        return "The component detected a version mismatch";

    case OMX_ErrorNotReady:
        return "The component is not ready to return data at this time";

    case OMX_ErrorTimeout:
        return "There was a timeout that occurred";

    case OMX_ErrorSameState:
        return
        "This error occurs when trying to transition into the state you are already in";

    case OMX_ErrorResourcesPreempted:
        return
        "Resources allocated to an executing or paused component have been preempted";

    case OMX_ErrorPortUnresponsiveDuringAllocation:
        return
        "Waited an unusually long time for the supplier to allocate buffers";

    case OMX_ErrorPortUnresponsiveDuringDeallocation:
        return
        "Waited an unusually long time for the supplier to de-allocate buffers";

    case OMX_ErrorPortUnresponsiveDuringStop:
        return
        "Waited an unusually long time for the non-supplier to return a buffer during stop";

    case OMX_ErrorIncorrectStateTransition:
        return "Attempting a state transition that is not allowed";

    case OMX_ErrorIncorrectStateOperation:
        return
        "Attempting a command that is not allowed during the present state";

    case OMX_ErrorUnsupportedSetting:
        return
        "The values encapsulated in the parameter or config structure are not supported";

    case OMX_ErrorUnsupportedIndex:
        return
        "The parameter or config indicated by the given index is not supported";

    case OMX_ErrorBadPortIndex:
        return "The port index supplied is incorrect";

    case OMX_ErrorPortUnpopulated:
        return
        "The port has lost one or more of its buffers and it thus unpopulated";

    case OMX_ErrorComponentSuspended:
        return "Component suspended due to temporary loss of resources";

    case OMX_ErrorDynamicResourcesUnavailable:
        return
        "Component suspended due to an inability to acquire dynamic resources";

    case OMX_ErrorMbErrorsInFrame:
        return "Frame generated macroblock error";

    case OMX_ErrorFormatNotDetected:
        return "Cannot parse or determine the format of an input stream";

    case OMX_ErrorContentPipeOpenFailed:
        return "The content open operation failed";

    case OMX_ErrorContentPipeCreationFailed:
        return "The content creation operation failed";

    case OMX_ErrorSeperateTablesUsed:
        return "Separate table information is being used";

    case OMX_ErrorTunnelingUnsupported:
        return "Tunneling is unsupported by the component";

    default:
        return "Unknown error";
    }
}


void* OmxCodecCreate(char* component_name, char* component_role)
{
    void *object = NULL; // TODO: do something later

    OmxCore* core = omx_core_new(object);
    if(!core)
        return NULL;

    omx_core_init(core, component_name, component_role);

    return (void*)core;
}

void OmxCodecDestroy(void* omx_codec)
{
    OmxCore* core = (OmxCore*)omx_codec;
    omx_core_deinit(core);
    omx_core_free (core);
}


int OmxCodecConfigure(void* omx_codec, OMX_BOOL isEncoder, OMX_U32 nFrameWidth,
OMX_U32 nFrameHeight,OMX_U32 nBitrate ,OMX_U32 nFrameRate, OMX_COLOR_FORMATTYPE color_format)
{
    OmxCore* core = (OmxCore*)omx_codec;
    if(isEncoder)
    {

        OMX_PARAM_PORTDEFINITIONTYPE param;
        OMX_VIDEO_PARAM_PORTFORMATTYPE format;
        OMX_VIDEO_PORTDEFINITIONTYPE *video_def = &param.format.video;

        format.nPortIndex = OMX_DirInput;
        OMX_GetParameter(core->omx_handle, OMX_IndexParamVideoPortFormat,
        &format);

        format.eCompressionFormat = OMX_VIDEO_CodingUnused;
        format.eColorFormat = color_format;

        OMX_SetParameter(core->omx_handle, OMX_IndexParamVideoPortFormat,
        &format);


        param.nPortIndex = OMX_DirInput;
        OMX_GetParameter(core->omx_handle, OMX_IndexParamPortDefinition,
        &param);

        param.nBufferSize = nFrameWidth*nFrameHeight*3/2; //* TODO: need check this later
        video_def->nFrameWidth = nFrameWidth;
        video_def->nFrameHeight = nFrameHeight;
        video_def->nStride = nFrameWidth;//* TODO: need check this later
        video_def->nBitrate = nBitrate;
        video_def->xFramerate = nFrameRate;

        OMX_SetParameter(core->omx_handle, OMX_IndexParamPortDefinition,
        &param);

        param.nPortIndex = OMX_DirOutput;
        OMX_GetParameter(core->omx_handle, OMX_IndexParamPortDefinition,
        &param);

        param.nBufferSize = nFrameWidth*nFrameHeight*3/2; //* TODO: need check this later
        video_def->nFrameWidth = nFrameWidth;
        video_def->nFrameHeight = nFrameHeight;
        video_def->nStride = nFrameWidth;//* TODO: need check this later
        video_def->nBitrate = nBitrate;
        video_def->xFramerate = nFrameRate;

        OMX_SetParameter(core->omx_handle, OMX_IndexParamPortDefinition,
        &param);

    }
    else
    {
        OMX_VIDEO_PARAM_PORTFORMATTYPE format;
        OMX_PARAM_PORTDEFINITIONTYPE param;
        OMX_VIDEO_PORTDEFINITIONTYPE *video_def = &param.format.video;

        format.nPortIndex = OMX_DirOutput;
        OMX_GetParameter(core->omx_handle, OMX_IndexParamVideoPortFormat,
        &format);

        format.eCompressionFormat = OMX_VIDEO_CodingUnused;
        format.eColorFormat = color_format;

        OMX_SetParameter(core->omx_handle, OMX_IndexParamVideoPortFormat,
        &format);

        param.nPortIndex = OMX_DirInput;
        OMX_GetParameter(core->omx_handle, OMX_IndexParamPortDefinition,
        &param);

        param.nBufferSize = nFrameWidth*nFrameHeight*3/2; //* TODO: need check this later

        video_def->nFrameWidth = nFrameWidth;
        video_def->nFrameHeight = nFrameHeight;
        video_def->nStride = nFrameWidth;//* TODO: need check this later

        OMX_SetParameter(core->omx_handle, OMX_IndexParamPortDefinition,
        &param);

        param.nPortIndex = OMX_DirOutput;
        OMX_GetParameter(core->omx_handle, OMX_IndexParamPortDefinition,
        &param);


        param.nBufferSize = nFrameWidth*nFrameHeight*3/2; //* TODO: need check this later
        video_def->nFrameWidth = nFrameWidth;
        video_def->nFrameHeight = nFrameHeight;
        video_def->nStride = nFrameWidth;//* TODO: need check this later


        OMX_SetParameter(core->omx_handle, OMX_IndexParamPortDefinition,
        &param);
    }

    return 0;
}


void OmxCodecStart(void* omx_codec)
{
    OmxCore* core = (OmxCore*)omx_codec;
    omx_core_prepare(core);
    omx_core_start(core);
}

void OmxCodecStop(void* omx_codec)
{
    OmxCore* core = (OmxCore*)omx_codec;
    omx_core_stop(core);
    omx_core_unload(core);
}


OMX_BUFFERHEADERTYPE* dequeneInputBuffer(void* omx_codec)
{
    OmxCore* core = (OmxCore*)omx_codec;
    OmxPort * port = &core->ports[OMX_PORT_INPUT];
    return omx_port_request_buffer(port);
}

int queneInputBuffer(void* omx_codec, OMX_BUFFERHEADERTYPE* pBuffer)
{
    OmxCore* core = (OmxCore*)omx_codec;
    OmxPort * port = &core->ports[OMX_PORT_INPUT];
    omx_port_release_buffer (port, pBuffer);
    return 0;
}

OMX_BUFFERHEADERTYPE* dequeneOutputBuffer(void* omx_codec)
{

    OmxCore* core = (OmxCore*)omx_codec;
    OmxPort * port = &core->ports[OMX_PORT_OUTPUT];
    return omx_port_request_buffer(port);
}

int queneOutputBuffer(void* omx_codec, OMX_BUFFERHEADERTYPE* pBuffer)
{
    OmxCore* core = (OmxCore*)omx_codec;
    OmxPort * port = &core->ports[OMX_PORT_OUTPUT];
    omx_port_release_buffer (port, pBuffer);
    return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
