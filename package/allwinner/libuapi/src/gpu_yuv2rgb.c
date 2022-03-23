/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2009 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifdef SUNXI_DISPLAY_GPU
#include "gpu_yuv2rgb.h"

int message(const char *format, ...) {
    int ret;
    va_list args;
    va_start(args, format);
    ret = vprintf(format, args);
    fflush(stdout);
    va_end(args);
    return ret;
}

void eglErrorCheck(int line) {
    EGLint err = eglGetError();

    switch (err) {
    case EGL_SUCCESS:
        message("line %d: eglGetError() = 0x%x: success\n", line, err);
        break;
    case EGL_NOT_INITIALIZED:
        message("line %d: eglGetError() = 0x%x: not initialized\n", line, err);
        break;
    case EGL_BAD_ACCESS:
        message("line %d: eglGetError() = 0x%x: bad access\n", line, err);
        break;
    case EGL_BAD_ALLOC:
        message("line %d: eglGetError() = 0x%x: bad alloc\n", line, err);
        break;
    case EGL_BAD_ATTRIBUTE:
        message("line %d: eglGetError() = 0x%x: bad attribute\n", line, err);
        break;
    case EGL_BAD_CONTEXT:
        message("line %d: eglGetError() = 0x%x: bad context\n", line, err);
        break;
    case EGL_BAD_CONFIG:
        message("line %d: eglGetError() = 0x%x: bad config\n", line, err);
        break;
    case EGL_BAD_CURRENT_SURFACE:
        message("line %d: eglGetError() = 0x%x: bad current surface\n", line,
                err);
        break;
    case EGL_BAD_DISPLAY:
        message("line %d: eglGetError() = 0x%x: bad display\n", line, err);
        break;
    case EGL_BAD_SURFACE:
        message("line %d: eglGetError() = 0x%x: bad surface\n", line, err);
        break;
    case EGL_BAD_MATCH:
        message("line %d: eglGetError() = 0x%x: bad match\n", line, err);
        break;
    case EGL_BAD_PARAMETER:
        message("line %d: eglGetError() = 0x%x: bad parameter\n", line, err);
        break;
    case EGL_BAD_NATIVE_PIXMAP:
        message("line %d: eglGetError() = 0x%x: bad native pixmap\n", line,
                err);
        break;
    case EGL_BAD_NATIVE_WINDOW:
        message("line %d: eglGetError() = 0x%x: bad native window\n", line,
                err);
        break;
    default:
        message("line %d: eglGetError() = 0x%x\n", line, err);
    }
}

void checkGlError(const char* op) {
    GLint error;
    for (error = glGetError(); error; error = glGetError()) {
        printf("error::after %s() glError (0x%x)\n", op, error);
    }
}

static void set_defaults(mmark_context* ctx) {
    /* default options */
    int fd;
    struct fb_var_screeninfo vinfo;
    fd = open("/dev/fb0", O_RDWR);
    if (fd < 0) {
        printf("Failed to open /dev/fb0\n");
        return;
    }
    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
        printf("Failed to get fb_var_screeninfo\n");
        return;
    }
    ctx->window_width = vinfo.xres;
    ctx->window_height = vinfo.yres;
    ctx->depth = 32;
    ctx->fsaa = 4;
    close(fd);
}
EGLConfig config;
static void init_egl(mmark_context* ctx) {
    EGLint attrib_list[64];
    int attribIndex = 0;
    EGLint max_num_config;
    EGLint num_configs;
    EGLConfig *configs = NULL;
    int i;
    int cfg_found = 0;
    //EGLConfig config;
    const EGLint window_attribute_list[] = { EGL_NONE };
    const EGLint context_attributes[] = { EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE };
    GLuint red = 0;
    GLuint green = 0;
    GLuint blue = 0;
    GLuint alpha = 0;
    GLuint fsaa = 0;
    EGLint major;
    EGLint minor;
    if (ctx->depth == 32) {
        red = 8;
        green = 8;
        blue = 8;
        alpha = 8;
    } else if (ctx->depth == 16) {
        red = 5;
        green = 6;
        blue = 5;
        alpha = 0;
    }
    fsaa = ctx->fsaa;
    attrib_list[attribIndex++] = EGL_RED_SIZE;
    attrib_list[attribIndex++] = red;
    attrib_list[attribIndex++] = EGL_GREEN_SIZE;
    attrib_list[attribIndex++] = green;
    attrib_list[attribIndex++] = EGL_BLUE_SIZE;
    attrib_list[attribIndex++] = blue;
    attrib_list[attribIndex++] = EGL_ALPHA_SIZE;
    attrib_list[attribIndex++] = alpha;
    attrib_list[attribIndex++] = EGL_SAMPLES;
    attrib_list[attribIndex++] = fsaa;

    attrib_list[attribIndex++] = EGL_NONE;

    dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (dpy == EGL_NO_DISPLAY) {
        fprintf(stderr, "line %d: no display\n", __LINE__);
        exit(1);
    }
    if (!eglInitialize(dpy, &major, &minor)) {
        eglErrorCheck(__LINE__);
        message("initializing EGL... 2\n");
        exit(1);
    }
    message("\nEGL INFO\nVersion: %i.%i (%s)\nVendor: %s\n", major, minor,
            eglQueryString(dpy, EGL_VERSION), eglQueryString(dpy, EGL_VENDOR));

    if (!eglGetConfigs(dpy, NULL, 0, &max_num_config)) {
        eglErrorCheck(__LINE__);
        exit(1);
    }
    configs = (EGLConfig *) malloc(sizeof(EGLConfig) * max_num_config);
    if (NULL == configs) {
        fprintf(stderr, "line %d: Out of memory\n", __LINE__);
        exit(1);
    }
    if (eglGetConfigs(dpy, configs, max_num_config, &max_num_config)
            == EGL_FALSE || eglGetError() != EGL_SUCCESS) {
        exit(1);
    }
    if (!eglChooseConfig(dpy, attrib_list, configs, max_num_config,
            &num_configs)) {
        eglErrorCheck(__LINE__);
        exit(1);
    }

    if (num_configs == 0) {
        fprintf(stderr, "line %d: no matching config found\n", __LINE__);
        exit(1);
    }
    for (i = 0; i < num_configs; i++) {
        EGLint value;
        /*Use this to explicitly check that the EGL config has the expected color depths */
        eglGetConfigAttrib(dpy, configs[i], EGL_RED_SIZE, &value);
        if (red != value)
            continue;
        message("red OK: %d \n", value);
        eglGetConfigAttrib(dpy, configs[i], EGL_GREEN_SIZE, &value);
        if (green != value)
            continue;
        message("green OK: %d \n", value);
        eglGetConfigAttrib(dpy, configs[i], EGL_BLUE_SIZE, &value);
        if (blue != value)
            continue;
        message("blue OK: %d \n", value);
        eglGetConfigAttrib(dpy, configs[i], EGL_ALPHA_SIZE, &value);
        if (alpha != value)
            continue;
        message("alpha OK: %d \n", value);
        eglGetConfigAttrib(dpy, configs[i], EGL_SAMPLES, &value);
        if (fsaa != value)
            continue;
        message("fsaa OK: %d \n", value);
        config = configs[i];
        cfg_found = 1;
        break;
    }
    if (!cfg_found) {
        fprintf(stderr, "line %d: no matching config found\n", __LINE__);
        exit(1);
    }
    if (configs) {
        free(configs);
    }
    /* Mali-specific call: */
    Ewin.width = ctx->window_width;
    Ewin.height = ctx->window_height;
    context = eglCreateContext(dpy, config, EGL_NO_CONTEXT, context_attributes);
    if (context == EGL_NO_CONTEXT) {
        eglErrorCheck(__LINE__);
        exit(1);
    }

    surface = eglCreateWindowSurface(dpy, config, &Ewin, window_attribute_list);
    /* Works with most things: */
    if (surface == EGL_NO_SURFACE) {
        switch (eglGetError()) {
        case EGL_BAD_MATCH:
            printf("EGL_BAD_MATCH\n");
            break;
        case EGL_BAD_CONFIG:
            printf("EGL_BAD_CONFIG\n");
            break;
        case EGL_BAD_NATIVE_WINDOW:
            printf("EGL_BAD_NATIVE_WINDOW\n");
            break;
        case EGL_BAD_ALLOC:
            printf("EGL_BAD_ALLOC\n");
            break;
            eglErrorCheck(__LINE__);
        }
        exit(1);
    }
    if (!eglMakeCurrent(dpy, surface, surface, context)) {
        eglErrorCheck(__LINE__);
        exit(1);
    }
    eglSwapInterval(dpy, 3);
}
static GLfloat vVertices[] = { -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, -1.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 0.0f, };
static GLfloat vTexVertices[] =
        { 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, };

static GLfloat vTexVertices_ionbuf[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, };

static const char *vertex_shader_source = "attribute vec4 aPosition;  \n"
        "attribute vec4 aColor;     \n"
        "attribute vec2 aTexCoord;  \n"
        "varying vec4 vColor;       \n"
        "varying vec2 vTexCoord;    \n"
        "void main()            \n"
        "{              \n"
        "   vColor = aColor;    \n"
        "   vTexCoord = aTexCoord;  \n"
        "   gl_Position = aPosition;\n"
        "}              \n";
static const char *fragment_shader_source_RGBA =
        "#extension GL_OES_EGL_image_external : require     \n"
                "precision mediump float;               \n"
                "uniform samplerExternalOES uTexSampler;        \n"
                "varying vec4 vColor;                   \n"
                "varying vec2 vTexCoord;                \n"
                "void main()                        \n"
                "{                          \n"
                "   vec4 temp = texture2D(uTexSampler, vTexCoord)   \n"
                "           +vColor;            \n"
                "   gl_FragColor = vec4(temp.xyz, 1.0);     \n"
                "}                          \n";

int gl_createshadow(int flag) {
    GLint ret;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    if (!vertex_shader) {
        fprintf(stderr,
                "Error: glCreateShader(GL_VERTEX_SHADER) " "failed: 0x%08X\n",
                glGetError());
        return -1;
    }
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &ret);
    if (!ret) {
        char *log;
        fprintf(stderr, "Error: vertex shader compilation failed!\n");
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &ret);
        if (ret > 1) {
            log = malloc(ret);
            glGetShaderInfoLog(vertex_shader, ret, NULL, log);
            fprintf(stderr, "%s", log);
        }
        return -1;
    }

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!fragment_shader) {
        fprintf(stderr,
                "Error: glCreateShader(GL_FRAGMENT_SHADER) " "failed: 0x%08X\n",
                glGetError());
        return -1;
    }
    glShaderSource(fragment_shader, 1, &fragment_shader_source_RGBA, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &ret);
    if (!ret) {
        char *log;
        fprintf(stderr, "Error: fragment shader compilation failed!\n");
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &ret);
        if (ret > 1) {
            log = malloc(ret);
            glGetShaderInfoLog(fragment_shader, ret, NULL, log);
            fprintf(stderr, "%s", log);
        }
        return -1;
    }
    return 0;
}

int gl_createprogram(int flag) {
    GLint ret;
    program = glCreateProgram();
    if (!program) {
        fprintf(stderr, "Error: failed to create program!\n");
        return -1;
    }
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    glBindAttribLocation(program, 0, "aPosition");
    glBindAttribLocation(program, 1, "aColor");
    glBindAttribLocation(program, 2, "aTexCoord");

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &ret);
    if (!ret) {
        char *log;
        fprintf(stderr, "Error: program linking failed!\n");
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &ret);
        if (ret > 1) {
            log = malloc(ret);
            glGetProgramInfoLog(program, ret, NULL, log);
            fprintf(stderr, "%s\n", log);
        }
        return -1;
    }
    glUseProgram(program);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
    glEnableVertexAttribArray(0);
    if (flag == GPU_IONBUF) {
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, vTexVertices_ionbuf);
        glEnableVertexAttribArray(2);
    } else if (flag == GPU_FRAMEBUFFER) {
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, vTexVertices);
        glEnableVertexAttribArray(2);
    }
    if (eglSwapInterval(dpy, 0) != EGL_TRUE) {
        printf("eglSwapInterval failed.\n");
    }
    return 0;
}

void gl_setsrcKHRattr(int dma_fd, Srcresource * src_resource, EGLint *attribs0) {
    int atti = 0;
    switch (src_resource->srcformat) {
    case FORMAT_YU12:
    case FORMAT_YUV420: {
        attribs0[atti++] = EGL_WIDTH;
        attribs0[atti++] = src_resource->cropwidth;
        attribs0[atti++] = EGL_HEIGHT;
        attribs0[atti++] = src_resource->cropheight;
        attribs0[atti++] = EGL_LINUX_DRM_FOURCC_EXT;
        attribs0[atti++] = DRM_FORMAT_YUV420;
        attribs0[atti++] = EGL_DMA_BUF_PLANE0_FD_EXT;
        attribs0[atti++] = dma_fd;
        attribs0[atti++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
        attribs0[atti++] = 0;
        attribs0[atti++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
        attribs0[atti++] = src_resource->srcwidth;

        attribs0[atti++] = EGL_DMA_BUF_PLANE1_FD_EXT;
        attribs0[atti++] = dma_fd;
        attribs0[atti++] = EGL_DMA_BUF_PLANE1_OFFSET_EXT;
        attribs0[atti++] = src_resource->srcwidth * src_resource->srcheight;
        attribs0[atti++] = EGL_DMA_BUF_PLANE1_PITCH_EXT;
        attribs0[atti++] = src_resource->srcwidth;

        attribs0[atti++] = EGL_DMA_BUF_PLANE2_FD_EXT;
        attribs0[atti++] = dma_fd;
        attribs0[atti++] = EGL_DMA_BUF_PLANE2_OFFSET_EXT;
        attribs0[atti++] = src_resource->srcwidth * src_resource->srcheight
                + src_resource->srcwidth * src_resource->srcheight * 0.25;
        attribs0[atti++] = EGL_DMA_BUF_PLANE2_PITCH_EXT;
        attribs0[atti++] = src_resource->srcwidth;

        attribs0[atti++] = EGL_YUV_COLOR_SPACE_HINT_EXT;
        attribs0[atti++] = EGL_ITU_REC709_EXT;
        attribs0[atti++] = EGL_SAMPLE_RANGE_HINT_EXT;
        attribs0[atti++] = EGL_YUV_FULL_RANGE_EXT;

        attribs0[atti++] = EGL_NONE;
        break;
    }
    case FORMAT_NV12: {
        attribs0[atti++] = EGL_WIDTH;
        attribs0[atti++] = src_resource->cropwidth;
        attribs0[atti++] = EGL_HEIGHT;
        attribs0[atti++] = src_resource->cropheight;
        attribs0[atti++] = EGL_LINUX_DRM_FOURCC_EXT;
        attribs0[atti++] = DRM_FORMAT_NV12;
        attribs0[atti++] = EGL_DMA_BUF_PLANE0_FD_EXT;
        attribs0[atti++] = dma_fd;
        attribs0[atti++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
        attribs0[atti++] = 0;
        attribs0[atti++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
        attribs0[atti++] = src_resource->srcwidth;

        attribs0[atti++] = EGL_DMA_BUF_PLANE1_FD_EXT;
        attribs0[atti++] = dma_fd;
        attribs0[atti++] = EGL_DMA_BUF_PLANE1_OFFSET_EXT;
        attribs0[atti++] = src_resource->srcwidth * src_resource->srcheight;
        attribs0[atti++] = EGL_DMA_BUF_PLANE1_PITCH_EXT;
        attribs0[atti++] = src_resource->srcwidth;

        attribs0[atti++] = EGL_YUV_COLOR_SPACE_HINT_EXT;
        attribs0[atti++] = EGL_ITU_REC709_EXT;
        attribs0[atti++] = EGL_SAMPLE_RANGE_HINT_EXT;
        attribs0[atti++] = EGL_YUV_FULL_RANGE_EXT;

        attribs0[atti++] = EGL_NONE;
        break;
    }
    case FORMAT_NV21: {
        attribs0[atti++] = EGL_WIDTH;
        attribs0[atti++] = src_resource->cropwidth;
        attribs0[atti++] = EGL_HEIGHT;
        attribs0[atti++] = src_resource->cropheight;
        attribs0[atti++] = EGL_LINUX_DRM_FOURCC_EXT;
        attribs0[atti++] = DRM_FORMAT_NV21;
        attribs0[atti++] = EGL_DMA_BUF_PLANE0_FD_EXT;
        attribs0[atti++] = dma_fd;
        attribs0[atti++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
        attribs0[atti++] = 0;
        attribs0[atti++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
        attribs0[atti++] = src_resource->srcwidth;

        attribs0[atti++] = EGL_DMA_BUF_PLANE1_FD_EXT;
        attribs0[atti++] = dma_fd;
        attribs0[atti++] = EGL_DMA_BUF_PLANE1_OFFSET_EXT;
        attribs0[atti++] = src_resource->srcwidth * src_resource->srcheight;
        attribs0[atti++] = EGL_DMA_BUF_PLANE1_PITCH_EXT;
        attribs0[atti++] = src_resource->srcwidth;

        attribs0[atti++] = EGL_YUV_COLOR_SPACE_HINT_EXT;
        attribs0[atti++] = EGL_ITU_REC709_EXT;
        attribs0[atti++] = EGL_SAMPLE_RANGE_HINT_EXT;
        attribs0[atti++] = EGL_YUV_FULL_RANGE_EXT;

        attribs0[atti++] = EGL_NONE;
        break;
    }
    case FORMAT_YV12: {
        attribs0[atti++] = EGL_WIDTH;
        attribs0[atti++] = src_resource->cropwidth;
        attribs0[atti++] = EGL_HEIGHT;
        attribs0[atti++] = src_resource->cropheight;
        attribs0[atti++] = EGL_LINUX_DRM_FOURCC_EXT;
        attribs0[atti++] = DRM_FORMAT_YUV420;
        attribs0[atti++] = EGL_DMA_BUF_PLANE0_FD_EXT;
        attribs0[atti++] = dma_fd;
        attribs0[atti++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
        attribs0[atti++] = 0;
        attribs0[atti++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
        attribs0[atti++] = src_resource->srcwidth;

        attribs0[atti++] = EGL_DMA_BUF_PLANE1_FD_EXT;
        attribs0[atti++] = dma_fd;
        attribs0[atti++] = EGL_DMA_BUF_PLANE1_OFFSET_EXT;
        attribs0[atti++] = src_resource->srcwidth * src_resource->srcheight
                + src_resource->srcwidth * src_resource->srcheight * 0.25;
        attribs0[atti++] = EGL_DMA_BUF_PLANE1_PITCH_EXT;
        attribs0[atti++] = src_resource->srcwidth;

        attribs0[atti++] = EGL_DMA_BUF_PLANE2_FD_EXT;
        attribs0[atti++] = dma_fd;
        attribs0[atti++] = EGL_DMA_BUF_PLANE2_OFFSET_EXT;
        attribs0[atti++] = src_resource->srcwidth * src_resource->srcheight;
        attribs0[atti++] = EGL_DMA_BUF_PLANE2_PITCH_EXT;
        attribs0[atti++] = src_resource->srcwidth;

        attribs0[atti++] = EGL_YUV_COLOR_SPACE_HINT_EXT;
        attribs0[atti++] = EGL_ITU_REC709_EXT;
        attribs0[atti++] = EGL_SAMPLE_RANGE_HINT_EXT;
        attribs0[atti++] = EGL_YUV_FULL_RANGE_EXT;

        attribs0[atti++] = EGL_NONE;
        break;
    }
    case FORMAT_YUYV: {
        attribs0[atti++] = EGL_WIDTH;
        attribs0[atti++] = src_resource->cropwidth;
        attribs0[atti++] = EGL_HEIGHT;
        attribs0[atti++] = src_resource->cropheight;
        attribs0[atti++] = EGL_LINUX_DRM_FOURCC_EXT;
        attribs0[atti++] = DRM_FORMAT_YUYV;
        attribs0[atti++] = EGL_DMA_BUF_PLANE0_FD_EXT;
        attribs0[atti++] = dma_fd;
        attribs0[atti++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
        attribs0[atti++] = 0;
        attribs0[atti++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
        attribs0[atti++] = src_resource->srcwidth;
        attribs0[atti++] = EGL_YUV_COLOR_SPACE_HINT_EXT;
        attribs0[atti++] = EGL_ITU_REC709_EXT;
        attribs0[atti++] = EGL_SAMPLE_RANGE_HINT_EXT;
        attribs0[atti++] = EGL_YUV_FULL_RANGE_EXT;

        attribs0[atti++] = EGL_NONE;
        break;
    }

    }
}

void gl_setdstKHRattr(int output_fd, Dstresource * dst_resource,
        EGLint *attribs1) {
    int att1i = 0;
    switch (dst_resource->dstformat) {
    case FORMAT_RGBA8888:
    case FORMAT_ARGB8888: {
        attribs1[att1i++] = EGL_WIDTH;
        attribs1[att1i++] = dst_resource->dstwidth;
        attribs1[att1i++] = EGL_HEIGHT;
        attribs1[att1i++] = dst_resource->dstheight;
        attribs1[att1i++] = EGL_LINUX_DRM_FOURCC_EXT;
        attribs1[att1i++] = DRM_FORMAT_RGBA8888;
        attribs1[att1i++] = EGL_DMA_BUF_PLANE0_FD_EXT;
        attribs1[att1i++] = output_fd;
        attribs1[att1i++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
        attribs1[att1i++] = 0;
        attribs1[att1i++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
        attribs1[att1i++] = dst_resource->dstwidth * 4;

        attribs1[att1i++] = EGL_NONE;
        break;
    }
    case FORMAT_RGB565: {
        attribs1[att1i++] = EGL_WIDTH;
        attribs1[att1i++] = dst_resource->dstwidth;
        attribs1[att1i++] = EGL_HEIGHT;
        attribs1[att1i++] = dst_resource->dstheight;
        attribs1[att1i++] = EGL_LINUX_DRM_FOURCC_EXT;
        attribs1[att1i++] = DRM_FORMAT_RGB565;
        attribs1[att1i++] = EGL_DMA_BUF_PLANE0_FD_EXT;
        attribs1[att1i++] = output_fd;
        attribs1[att1i++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
        attribs1[att1i++] = 0;
        attribs1[att1i++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
        attribs1[att1i++] = dst_resource->dstwidth * 2;

        attribs1[att1i++] = EGL_NONE;
        break;
    }
    }
}

int gl_createSRCKHR(int dma_fd, Srcresource * src_resource) {
    EGLint attribs0[30];
    GLuint yuvTex0;

    if (dma_fd < 0) {
        printf("dma_fb < 0\n");
        return -1;
    }
    gl_setsrcKHRattr(dma_fd, src_resource, attribs0);
    input_img = eglCreateImageKHR(dpy, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, 0,
            attribs0);
    if (input_img == EGL_NO_IMAGE_KHR) {
        printf("Error:0000 failed: 0x%08X\n", eglGetError());
        return -1;
    }

    //glGenTextures(1, &yuvTex0);
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_EXTERNAL_OES, yuvTex0);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES,
            (GLeglImageOES) input_img);
    if (glGetError()) {
        printf("Error:44 failed: 0x%08X\n", glGetError());
        return -1;
    }
    return 0;
}

EGLImageKHR gl_createDSTKHR(int output_fd, Dstresource * dst_resource) {
    EGLint attribs1[30];
    gl_setdstKHRattr(output_fd, dst_resource, attribs1);
    output_img = eglCreateImageKHR(dpy, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT,
            0, attribs1);
    if (output_img == EGL_NO_IMAGE_KHR) {
        printf("output_img Error:0000 failed: 0x%08X\n", eglGetError());
        return NULL;
    }

    return output_img;
}
int gl_bindFBO(EGLImageKHR swap_img) {
    checkGlError("glBindRenderbuffer");
    glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, swap_img);
    checkGlError("glEGLImageTargetRenderbufferStorageOES");
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_RENDERBUFFER, outputRbo);
    checkGlError("glFramebufferRenderbuffer");
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("failed to create framebuffer\n");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    return 0;
}

void Redraw(Dstresource *dst_resource) {
#ifndef SUNXI_DISPLAY_GPU_BUF
    glViewport(dst_resource->dstx,
            dst_resource->screenheight - dst_resource->dstheight
                    - dst_resource->dsty, dst_resource->dstwidth,
            dst_resource->dstheight);
#else
    glViewport(dst_resource->dstx, dst_resource->dsty, dst_resource->dstwidth,
            dst_resource->dstheight);
#endif
    //glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#ifndef SUNXI_DISPLAY_GPU_BUF
    eglSwapBuffers(dpy, surface);
#endif
    glFlush();
}

static int BuildOpenglES(Dstresource * dst_resource) {
    Redraw(dst_resource);
    return 0;
}

void DestorySrcKHR() {
    if (input_img) {
        eglDestroyImageKHR(dpy, input_img);
    }
}

void DestoryDstKHR(EGLImageKHR swap_img) {
    if (swap_img) {
        eglDestroyImageKHR(dpy, swap_img);
    }
}

void InitGLResource(int flag) {
    mmark_context context;
    set_defaults(&context);
    init_egl(&context);
    gl_createshadow(flag);
    gl_createprogram(flag);

    if (flag == GPU_IONBUF) {
        glGenRenderbuffers(1, &outputRbo);
        glBindRenderbuffer(GL_RENDERBUFFER, outputRbo);
        glGenFramebuffers(1, &outputFbo);
        glBindFramebuffer(GL_FRAMEBUFFER, outputFbo);
    }
}

void DestoryGLResource(int flag) {
    if (vertex_shader) {
        glDetachShader(program, vertex_shader);
        glDeleteShader(vertex_shader);
    }
    if (fragment_shader) {
        glDetachShader(program, fragment_shader);
        glDeleteShader(fragment_shader);
    }
    if (program) {
        //glDeleteShader(program);
        glDeleteProgram(program);
    }
    eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (context) {
        eglDestroyContext(dpy, context);
    }
    if (surface) {
        eglDestroySurface(dpy, surface);
    }
    if (flag == GPU_IONBUF) {
        if (outputRbo) {
            glDeleteRenderbuffers(GL_RENDERBUFFER, &outputRbo);
        }
        if (outputFbo) {
            glDeleteFramebuffers(GL_FRAMEBUFFER, &outputFbo);
        }
    }

    eglReleaseThread();

}
int ShaderYUVtoRGB_toionbuf(Dstresource * dst_resource) {
    if (BuildOpenglES(dst_resource) != 0) {
        return -1;
    }
    return 0;
}
int ShaderYUVtoRGB_tofb(int dma_fd, int output_fd, Srcresource * src_resource,
        Dstresource * dst_resource) {
    gl_createSRCKHR(dma_fd, src_resource);
    if (BuildOpenglES(dst_resource) != 0) {
        return -1;
    }
    eglSwapBuffers(dpy, surface);
    return 0;
}
#endif
