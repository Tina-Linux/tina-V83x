/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2009 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#define GLES 
#define TRI_SCALE  2

#ifdef WINCE
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>

#ifdef GLES
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#define USE_FIXEDP 1
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>

#ifdef GLES
#if USE_FIXEDP
   #define COMPONENTS 3
   #define STRIDE     0
   typedef GLfixed elem_t;
   #define ELEM_TYPE GL_FIXED
#else /* USE_FIXEDP */
   #define COMPONENTS 3
   #define STRIDE 0
   typedef GLbyte elem_t;
   #define ELEM_TYPE GL_BYTE
#endif /* USE_FIXEDP */
   NativeWindowType win;
#else /* GLES */
   #define COMPONENTS 3
   #define STRIDE     0
    typedef GLushort elem_t;
   #define ELEM_TYPE GL_SHORT
#endif /* GLES */

enum
{
   VBO_INDEX,
   VBO_VERTEX,
   VBO_NORMAL,
   VBO_MAX
} vbo_slots;

typedef unsigned short index_t;
typedef struct
{
   elem_t *elems;
   elem_t *normals;
   index_t *indices;
   unsigned int n_elems;
   unsigned int n_indices;
   GLuint vbo_names[VBO_MAX];
   int n_frames;
   unsigned int box_width;
   unsigned int box_height;
   float        tri_scale;
   unsigned int n_draws_horizontal;
   unsigned int n_draws_vertical;
   unsigned int window_height;
   unsigned int window_width;
   unsigned int depth;
   unsigned int fsaa;
   double frame_start_time;
   int total_frames;
   int quit_on_frame;
   int infinite_loop;
   unsigned int fpsevery;
   unsigned int n_warmup;
   int use_quads;
   int use_vbo;
   int swaponly;
   int fshader;
   int vshader;
} mmark_context;

int message( const char *format, ... )
{
   int ret;
   va_list args;

   va_start( args, format );
#ifdef WINCE
   {
      char buf[1024];

      ret = vsprintf( buf, format, args );
      RETAILMSG( 1, ( L"%S", buf ) );
   }
#else
   ret = vprintf( format, args );
   fflush( stdout );
#endif
   va_end(args);
   return ret;
}

#ifdef GLES

void eglErrorCheck(int line) {
   EGLint err = eglGetError();
   
   switch (err) {
      case EGL_SUCCESS:             message("line %d: eglGetError() = 0x%x: success\n", line, err);             break;
      case EGL_NOT_INITIALIZED:     message("line %d: eglGetError() = 0x%x: not initialized\n", line, err);     break;
      case EGL_BAD_ACCESS:          message("line %d: eglGetError() = 0x%x: bad access\n", line, err);          break;
      case EGL_BAD_ALLOC:           message("line %d: eglGetError() = 0x%x: bad alloc\n", line, err);           break;
      case EGL_BAD_ATTRIBUTE:       message("line %d: eglGetError() = 0x%x: bad attribute\n", line, err);       break;
      case EGL_BAD_CONTEXT:         message("line %d: eglGetError() = 0x%x: bad context\n", line, err);         break;
      case EGL_BAD_CONFIG:          message("line %d: eglGetError() = 0x%x: bad config\n", line, err);          break;
      case EGL_BAD_CURRENT_SURFACE: message("line %d: eglGetError() = 0x%x: bad current surface\n", line, err); break;
      case EGL_BAD_DISPLAY:         message("line %d: eglGetError() = 0x%x: bad display\n", line, err);         break;
      case EGL_BAD_SURFACE:         message("line %d: eglGetError() = 0x%x: bad surface\n", line, err);         break;
      case EGL_BAD_MATCH:           message("line %d: eglGetError() = 0x%x: bad match\n", line, err);           break;
      case EGL_BAD_PARAMETER:       message("line %d: eglGetError() = 0x%x: bad parameter\n", line, err);       break;
      case EGL_BAD_NATIVE_PIXMAP:   message("line %d: eglGetError() = 0x%x: bad native pixmap\n", line, err);   break;
      case EGL_BAD_NATIVE_WINDOW:   message("line %d: eglGetError() = 0x%x: bad native window\n", line, err);   break;
      default:                      message("line %d: eglGetError() = 0x%x\n", line, err);
   }
}

void glErrorCheck(int line) {
   GLenum err = glGetError();
   switch (err) {
      case GL_NO_ERROR:                                                                                                        break;
      case GL_INVALID_ENUM:                  message("line %d: glError() = 0x%x: invalid enum\n", line, err);                  break;
      case GL_INVALID_VALUE:                 message("line %d: glError() = 0x%x: invalid value\n", line, err);                 break;
      case GL_INVALID_OPERATION:             message("line %d: glError() = 0x%x: invalid operation\n", line, err);             break;
      case GL_INVALID_FRAMEBUFFER_OPERATION: message("line %d: glError() = 0x%x: invalid framebuffer operation\n", line, err); break;
      case GL_OUT_OF_MEMORY:                 message("line %d: glError() = 0x%x: out of memory\n", line, err);                 break;
      default:                               message("line %d: glError() = 0x%x\n", line, err);
   }
}

#endif

static void set_default_width_and_height(mmark_context* ctx)
{
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

	ctx->window_width  = vinfo.xres;
	ctx->window_height = vinfo.yres;
}

void set_defaults(mmark_context* ctx) {
   /* default options */
   ctx->infinite_loop = 1;
   ctx->fpsevery      = 10;
   ctx->n_warmup      = 2;
   ctx->use_quads     = GL_FALSE;
   ctx->swaponly      = GL_FALSE;
   ctx->vshader       = 0;
   ctx->fshader       = 0;
   ctx->use_vbo       = GL_FALSE;
    
   ctx->box_width     = 32;
   ctx->box_height    = 32;
   ctx->tri_scale     = TRI_SCALE;
   ctx->depth         = 32;
   ctx->fsaa          = 4;

   set_default_width_and_height(ctx);
}

static void usage(char *progname)
{
   mmark_context context;
   set_defaults(&context);
   message("Usage: %s\n", progname );
   message("\t[--width n]         - Set the screen width (default %d)\n", context.window_width );
   message("\t[--height n]        - Set the screen height (default %d)\n", context.window_height );
   message("\t[--depth n]         - Set the screen depth (16 | 32) (default %d)\n", context.depth );
   message("\t[--box_width n]     - Set the box width (default %d)\n", context.box_width );
   message("\t[--box_height n]    - Set the box height (default %d)\n", context.box_height );
   message("\t[--tri_scale n]     - Set the triangle scale-factor (default %d)\n", context.tri_scale );
   message("\t[--fsaa n]          - Set number of samples for anti-aliasing (0|4|16) (default %d)\n", context.fsaa );
   message("\t[--maxframe n]      - Quit after n frames (Endless by default)\n" );
   message("\t[--fpsevery n]      - Report FPS every n frames (default %d)\n", context.fpsevery );
   message("\t[--quads]           - Draw scene using many quads instead of few tristrips (tristrips by default)\n" );
   message("\t[--vbo]             - Use VBO's for all data (client arrays by default)\n" );
   message("\t[--swaponly]        - Don't send any primitives - just swap (default draws primitives)\n" );
   message("\t[--fshader=x]       - Set fragmentshader to be used (default %d)\n", context.vshader);
   message("\t[--vshader=x]       - Set vertexshader to be used (default %d)\n", context.fshader);
   message("\t[--help]            - Show this help\n" );
}

static void parse_cli( mmark_context *ctx, int argc, char **argv )
{
   int i;
   for ( i = 1; i < argc; ++i)
   {
      if( 0 == strcmp( "--width", argv[i] ))
      {
         int value;
         if( (i + 1) >= argc )
         {
            usage( argv[0] );
            exit(1);
         }
         ++i;
         value = atoi( argv[i] );
         if( value < 0 )
         {
            value = 0;
         }
         ctx->window_width = value;
      } 
        else if( 0 == strcmp( "--height", argv[i] ))
      {
         int value;
         if( (i + 1) >= argc )
         {
            usage( argv[0] );
            exit(1);
         }
         ++i;
         value = atoi( argv[i] );
         if( value < 0 )
         {
            value = 0;
         }
         ctx->window_height = value;
      }        
        else if( 0 == strcmp( "--depth", argv[i] ))
      {
         int value;
         if( (i + 1) >= argc )
         {
            usage( argv[0] );
            exit(1);
         }
         ++i;
         value = atoi( argv[i] );
         if( (value != 32 ) && (value != 16 ) )
         {
            fprintf(stderr, "line %d: wrong value for --depth\n", __LINE__);
            usage(argv[0]);
            exit(1);
         }
         ctx->depth = value;
      }
        else if( 0 == strcmp( "--box_width", argv[i] ))
      {
         int value;
         if( (i + 1) >= argc )
         {
            usage( argv[0] );
            exit(1);
         }
         ++i;
         value = atoi( argv[i] );
         if( value < 0 )
         {
            value = 0;
         }
         ctx->box_width = value;
      }              
        else if( 0 == strcmp( "--box_height", argv[i] ))
      {
         int value;
         if( (i + 1) >= argc )
         {
            usage( argv[0] );
            exit(1);
         }
         ++i;
         value = atoi( argv[i] );
         if( value < 0 )
         {
            value = 0;
         }
         ctx->box_height = value;
      }              
        else if( 0 == strcmp( "--tri_scale", argv[i] ))
      {
         double value;
         if( (i + 1) >= argc )
         {
            usage( argv[0] );
            exit(1);
         }
         ++i;
         value = atof( argv[i] );
         if( value < 0 )
         {
            value = 0;
         }
         ctx->tri_scale = value;
      }              
        else if( 0 == strcmp( "--fsaa", argv[i] ))
      {
         int value;
         if( (i + 1) >= argc )
         {
            usage( argv[0] );
            exit(1);
         }
         ++i;
         value = atoi( argv[i] );
         if( (value != 0 ) && (value != 4 ) && (value != 16 ) )
         {
            fprintf(stderr, "line %d: wrong value for --fsaa\n", __LINE__);
            usage(argv[0]);
            exit(1);
         }
         ctx->fsaa = value;
      }         
      else if( 0 == strcmp( "--maxframe", argv[i] ))
      {
         int value;
         if( (i + 1) >= argc )
         {
            usage( argv[0] );
            exit(1);
         }
         ++i;
         value = atoi( argv[i] );
         if( value < 0 )
         {
            value = 0;
         }

         message( "Quit after %d frames\n", value );
         ctx->quit_on_frame = value;
         ctx->infinite_loop = 0;
      }
      else if( 0 == strcmp( "--fpsevery", argv[i] ))
      {
         int value;
         if( (i + 1) >= argc )
         {
            usage( argv[0] );
            exit(1);
         }
         ++i;
         value = atoi( argv[i] );
         if( value < 0 )
         {
            value = 0;
         }

         message( "FPS report every %d frames\n", value );
         ctx->fpsevery = value;
      }
      else if( 0 == strcmp( "--quads", argv[i] ))
      {
         message( "Scene will be drawn as QUADS!\n");
         ctx->use_quads = GL_TRUE;
      }
      else if( 0 == strcmp( "--vbo", argv[i] ) )
      {
         message( "VBOs will be used!\n" );
         ctx->use_vbo = GL_TRUE;
      }
      else if( 0 == strcmp( "--swaponly", argv[i] ))
      {
         message( "Triangles will not be drawn!\n");
         ctx->swaponly = GL_TRUE;
      }
      else if( 0 == strncmp( "--fshader=", argv[i], 10 ))
      {
         ctx->fshader = atoi(argv[i]+10);
      }
      else if( 0 == strncmp( "--vshader=", argv[i], 10 ))
      {
         ctx->vshader = atoi(argv[i]+10);
      }
      else if( 0 == strcmp( "--help", argv[i] ))
      {
         usage(argv[0]);
         exit(0);
      }
      else
      {
         message( "Unknown argument: %s\n", argv[i] );
            usage(argv[0]);
         exit(0);
      }
   }
}

#ifdef GLES

static EGLDisplay dpy;
static EGLSurface surface;
static EGLContext context;

static void *display = (void*)0;

static GLfloat model_view_matrix[16];
static GLfloat saved_matrix[16];
void print_matrix(GLfloat A[16]);


void matrix_copy( GLfloat *src, GLfloat *dst )
{
   (void) memcpy( dst, src, sizeof(GLfloat) * 16 );
}

         /*  AB = C */
void matrix_mult(GLfloat A[16], GLfloat B[16], GLfloat C[16])
{
   int i, j, k;
   for(j = 0; j < 4; ++j )
   {
      for(i=0; i<4; ++i)
      {
         C[i+4*j] = 0.0;
         for(k=0; k<4; ++k)
         {
            C[i+4*j] += A[k+j*4]*B[i+k*4];
         }
      }
   }
}
void load_identity() {
   int i;
   for(i=0; i<16; i++) model_view_matrix[i] = (i%5==0) ? 1.0 : 0.0;
}
void matrix_transpose(GLfloat A[16]) {
   int i, j;
   GLfloat tmp;
   for(j=0; j<4; ++j) {
      for(i=j+1; i<4; ++i) {
         tmp      = A[i+j*4];
         A[i+j*4] = A[j+i*4];
         A[j+i*4] = tmp;
      }
   } 
}
void mv_translate(GLfloat dx, GLfloat dy, GLfloat dz) {
   GLfloat translate_matrix[16] = {
      1.0,   0.0,   0.0,   dx,
      0.0,   1.0,   0.0,   dy,
      0.0,   0.0,   1.0,   dz,
      0.0,   0.0,   0.0,   1.0};
   GLfloat new_MV_mat[16];
   matrix_mult(model_view_matrix, translate_matrix, new_MV_mat);
   matrix_copy(new_MV_mat, model_view_matrix);
}
void mv_scale(GLfloat sx, GLfloat sy, GLfloat sz) {
   GLfloat scale_matrix[16] = {
      sx,      0.0,   0.0,   0.0,
      0.0,   sy,      0.0,   0.0,
      0.0,   0.0,   sz,      0.0,
      0.0,   0.0,   0.0,   1.0};
   GLfloat new_MV_mat[16];
   matrix_mult(model_view_matrix, scale_matrix, new_MV_mat);
   matrix_copy(new_MV_mat, model_view_matrix);
}
void print_matrix(GLfloat A[16]) {
   int i, j;
   for(j=0; j<4; j++) {
      for(i=0; i<4; i++) {
         message("%f\t", A[i+j*4]);
      }
      message("\n");
   }
}

#define FLOAT_TO_FIXED(val) ((int)((val)*65536.0))

void init_egl(mmark_context* ctx) {
   EGLint      attrib_list[64];   
   int         attribIndex = 0;
   EGLint      max_num_config;
   EGLint      num_configs;
   EGLConfig   *configs = NULL;
   int         i;
   int         cfg_found = 0;
   EGLConfig   config;   
   const EGLint context_attributes[] =
   {
      EGL_CONTEXT_CLIENT_VERSION, 2,  
      EGL_NONE
   };
   GLuint   red;
   GLuint   green;
   GLuint   blue;
   GLuint   alpha;
   GLuint   fsaa;
   EGLint major, minor;
   
   
   if( ctx->depth == 32 ) {
      red   = 8;
      green = 8;
      blue  = 8;
      alpha = 8;
   } else if ( ctx->depth == 16 ) {
      red   = 5;
      green = 6;
      blue  = 5;
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
   attrib_list[attribIndex++] = EGL_RENDERABLE_TYPE;
   attrib_list[attribIndex++] = EGL_OPENGL_ES2_BIT;

   attrib_list[attribIndex++] = EGL_NONE;     

   message("initializing EGL...\n");
   dpy = eglGetDisplay( EGL_DEFAULT_DISPLAY );
   if (dpy == EGL_NO_DISPLAY) {
      fprintf(stderr, "line %d: no display\n", __LINE__ );
      exit(1);
   }
   
   if (!eglInitialize(dpy, &major, &minor)) {
      eglErrorCheck(__LINE__);
      exit(1);
   }
   
   message("\nEGL INFO\nVersion: %i.%i (%s)\nVendor: %s\n", major, minor, eglQueryString(dpy, EGL_VERSION), eglQueryString(dpy, EGL_VENDOR));
   
   if ( !eglGetConfigs(dpy, NULL, 0, &max_num_config) )
   {
      eglErrorCheck(__LINE__);
      exit(1);
   }

   configs = (EGLConfig *)malloc( sizeof( EGLConfig) * max_num_config );
   if ( NULL == configs )
   {
      fprintf(stderr, "line %d: Out of memory\n", __LINE__ );
      exit(1);
   }   
   
   if (!eglChooseConfig(dpy, attrib_list, configs, max_num_config, &num_configs)) {
      eglErrorCheck(__LINE__);
      exit(1);
   }

   if (num_configs == 0) {
      fprintf(stderr, "line %d: no matching config found\n", __LINE__);
      exit(1);
   }
   
   for ( i=0; i<num_configs; i++ )
   {
      EGLint value;
      /*Use this to explicitly check that the EGL config has the expected color depths */
      eglGetConfigAttrib( dpy, configs[i], EGL_RED_SIZE, &value );
      if ( red != value ) continue;
      message("red OK: %d \n", value);
      eglGetConfigAttrib( dpy, configs[i], EGL_GREEN_SIZE, &value );
      if ( green != value ) continue;
      message("green OK: %d \n", value);
      eglGetConfigAttrib( dpy, configs[i], EGL_BLUE_SIZE, &value );
      if ( blue != value ) continue;
      message("blue OK: %d \n", value);
      eglGetConfigAttrib( dpy, configs[i], EGL_ALPHA_SIZE, &value );
      if ( alpha != value ) continue;
      message("alpha OK: %d \n", value);
      eglGetConfigAttrib( dpy, configs[i], EGL_SAMPLES, &value );
      if ( fsaa != value ) continue;
      message("fsaa OK: %d \n",value);

      config = configs[i];
      cfg_found=1;
      break;
   }

   if( !cfg_found ) {
      fprintf(stderr, "line %d: no matching config found\n", __LINE__);
      exit(1);
   }

   surface = eglCreateWindowSurface(dpy, config, win, 0);

   /* Works with most things: */
   /*surface = eglCreateWindowSurface(dpy, config, NULL, 0);*/
   if (surface == EGL_NO_SURFACE) {
      eglErrorCheck(__LINE__);
      exit(1);
   }
         
   context = eglCreateContext(dpy, config, EGL_NO_CONTEXT, context_attributes);
   if (context == EGL_NO_CONTEXT) {
      eglErrorCheck(__LINE__);
      exit(1);
   }
      
   if (!eglMakeCurrent(dpy, surface, surface, context)) {
      eglErrorCheck(__LINE__);
      exit(1);
   }

}
#endif


static double get_time(  )
{
#ifdef UNDER_CE
   return (double)GetTickCount() / 1000.0;
#else
   struct timeval tv;
   gettimeofday(&tv, 0);
   return tv.tv_sec + tv.tv_usec*1e-6;
#endif
}


/* Remember to stick the ',' between all shader declerations */
const char *fshader[] =
{
   /* Fragment shader 0 */
   "precision mediump float;      \n"
   "uniform vec4 draw_color;      \n"
   "void main() {                 \n"
   "   gl_FragColor = draw_color; \n"
   "}                             \n",

   /* Fragment shader 1 */
   "precision mediump float;                \n"
   "void main() {                           \n"
   "   gl_FragColor = vec4( .3, .0, .5, 1); \n"
   "}                                       \n",

   /* Fragment shader 2 */
   "precision mediump float;          \n"
   "varying vec4 glw_FrontColor;      \n"
   "void main() {                     \n"
   "   gl_FragColor = glw_FrontColor; \n"
   "}                                 \n",
};
const char *vshader[] = 
{
   /* Vertex shader 0 */
   "attribute vec4 glw_Vertex;                    \n"
   "uniform mat4 modelview_mat;                   \n"
   "void main() {                                 \n"
   "   gl_Position = modelview_mat * glw_Vertex;  \n"
   "}                                             \n",

   /* Vertex shader 1*/
   "varying vec4 glw_FrontColor;                                           \n"
   "attribute vec4 glw_Vertex;                                             \n"
   "attribute vec3 glw_Normal;                                             \n"
   "uniform mat4 modelview_mat;                                            \n"
   "                                                                       \n"
   "void main( void )                                                      \n"
   "{                                                                      \n"
   "   vec4 lc_temp_pos;                                                   \n"
   "   vec4 lc_N;                                                          \n"
   "   vec4 lc_diffuseLight;                                               \n"
   "   vec4 lc_L;                                                          \n"
   "   vec4 lc_R0;                                                         \n"
   "                                                                       \n"
   "   lc_temp_pos =  (modelview_mat * glw_Vertex) / glw_Vertex.wwww;      \n"
   "   lc_N = modelview_mat * vec4( glw_Normal.xy, -glw_Normal.z, 1.0 );   \n"
   "                                                                       \n"
   "   lc_L = vec4( 50.0, 50.0, 0.0, 0.0 ) + -lc_temp_pos;                 \n"
   "   lc_R0.x = dot(lc_L.xyz, lc_L.xyz);                                  \n"
   "   lc_R0.x = inversesqrt(lc_R0.x);                                     \n"
   "   lc_L = lc_L * lc_R0.xxxx;                                           \n"
   "                                                                       \n"
   "   lc_diffuseLight.x = dot(lc_N.xyz, lc_L.xyz);                        \n"
   "   lc_diffuseLight.x = max(lc_diffuseLight.x, 0.0);                    \n"
   "                                                                       \n"
   "   lc_diffuseLight.x = lc_diffuseLight.x * 0.5;                        \n"
   "   glw_FrontColor = lc_diffuseLight.xxxx * vec4( 0.8, 0.8, 0.8, 0.8 ); \n"
   "   gl_Position = modelview_mat * glw_Vertex;                           \n"
   "}                                                                      \n"
};

GLuint program;
void shader_setup( int fragment_shader, int vertex_shader ) 
{
   int n_vshaders = sizeof(vshader)/sizeof(char*);
   int n_fshaders = sizeof(fshader)/sizeof(char*);
   GLint compiled;
   char  buffer[4096];
   GLint linked;
   GLuint vs, fs;

   if(fragment_shader >= n_fshaders || fragment_shader < 0) {
      message("Invalid fragment shader %d\n", fragment_shader);
      exit(1);
   }
   if(vertex_shader >= n_vshaders || vertex_shader < 0) {
      message("Invalid vertex shader %d \n", vertex_shader);
      exit(1);
   }
   vs = glCreateShader(GL_VERTEX_SHADER);
   fs = glCreateShader(GL_FRAGMENT_SHADER);
   program = glCreateProgram();
   glAttachShader(program, fs);
   glAttachShader(program, vs);

   message("Setting up vertex shader %d... ", vertex_shader);
   glShaderSource(vs, 1, &(vshader[vertex_shader]), NULL);
   glCompileShader(vs);
   glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
   if(compiled == GL_FALSE)
   {
      message("Critical Error: Unable to compile vertex shader '%s' \n", buffer);
      glGetShaderInfoLog(vs, 4095, NULL, buffer);
      message(" - reason:\n%s\n\n", buffer);
      exit(1);
   }
   message("Succesfully\n");

   message("Setting up fragment shader %d... ", fragment_shader);
   glShaderSource(fs, 1, &(fshader[fragment_shader]), NULL);
   glCompileShader(fs);
   glGetShaderiv(fs, GL_COMPILE_STATUS, &compiled);
   if(compiled == GL_FALSE)
   {
      message("Critical Error: Unable to compile fragment shader '%s' \n", buffer);
      glGetShaderInfoLog(fs, 4095, NULL, buffer);
      message(" - reason:\n%s\n\n", buffer);
      exit(1);
   }
   message("Succesfully\n");

   glLinkProgram(program);
   glGetProgramiv(program, GL_LINK_STATUS, &linked);
   if(linked == GL_FALSE)
   {
      glGetProgramInfoLog(program, 4095, NULL, buffer);
      message("Critical Error: Unable to link program - reason:\n%s\n\n", buffer);
      exit(1);
   }

   glUseProgram(program);

   { /* specific shader uniform setup */
      GLuint pos = glGetUniformLocation(program, "color");
      glUniform3f( pos, 1,1,1 );
   }
   return;
}


static int create_tri_strip(mmark_context *ctx)
{
   int i, j;
   elem_t z = 0;
   elem_t *elems;
   elem_t *normals;
   index_t *indices;
   unsigned idx;

   int width  = ctx->box_width;
    int height = ctx->box_height;    
    
    
   assert(width % 2 == 0 && "width must be even");
   assert(width <= 256 && "width must fit in a byte");
   
   ctx->elems = elems = (elem_t*)malloc(width*height * sizeof(elem_t) * COMPONENTS);
   ctx->normals = normals = (elem_t*)malloc(width*height * sizeof(elem_t) * COMPONENTS);
   ctx->n_elems = width * height;

   for(j = 0; j < width; ++j)
   {
      for(i = 0; i < height; ++i)
      {
         elem_t *elem = &(elems[(j*height + i)*COMPONENTS]);
         elem_t *normal = &(normals[(j*height + i)*COMPONENTS]);
#if USE_FIXEDP
         elem[0]   = j << 16;
         elem[1]   = i << 16;   
         elem[2]   = z << 16;
         normal[0] = 0 << 16;
         normal[1] = 0 << 16;
         normal[2] = 1 << 16;
#else
         elem[0]   = j;
         elem[1]   = i;
         elem[2]   = z;
         normal[0] = 0;
         normal[1] = 0;
         normal[2] = 1;
#endif
      }
   }
   
   ctx->n_indices = (width-1)*(height*4-2) + (width-1)*2;
   ctx->indices = indices = malloc(ctx->n_indices*sizeof(index_t));

   idx = 0;

   for(j = 0; j < width; ++j)
   {
      if(j != width - 1)
      {
         for(i = 0; i < height; ++i)
         {
            /* Upfaced triangles */
            indices[idx++] = (j+1)*height + i;
            indices[idx++] = j*height + i;
         }
         
         for(i = height-1; i --> 0; )
         {
            /* Downfaced triangles */
            indices[idx++] = (j+1)*height + i;
            indices[idx++] = j*height + i;
            
         }
         /* degenerate triangles */
         indices[idx++] = (j+1)*height + 0;
         indices[idx++] = (j+1)*height + 0;
      }
   }
   assert(idx == ctx->n_indices);
   ctx->n_indices = idx;
#ifndef NDEBUG
   {
      int *enableds = calloc(width*height, sizeof(int));
      for(idx = 0; idx < ctx->n_indices; ++idx)
      {
         assert(indices[idx] < width*height);
#if USE_FIXEDP
         assert(( elems[indices[idx]*COMPONENTS+0] >> 16 ) < width);
         assert(( elems[indices[idx]*COMPONENTS+1] >> 16 ) < height);   
         assert(( elems[indices[idx]*COMPONENTS+2] >> 16 ) == z);
#else
         assert(elems[indices[idx]*COMPONENTS+0] < width);
         assert(elems[indices[idx]*COMPONENTS+1] < height);   
         assert(elems[indices[idx]*COMPONENTS+2] == z);
#endif
         enableds[indices[idx]] = 1;
      }
      for(idx = 0; idx < width*height; ++idx)
      {
         assert(enableds[idx]);
      }
	free(enableds);
   }
#endif
   return 0;
}



static int init(mmark_context *ctx)
{
   create_tri_strip(ctx);
   
   glDisable( GL_DEPTH_TEST );
   glEnable( GL_CULL_FACE );
   ctx->n_frames = -((int)ctx->n_warmup); /* negative start will cause a 'warmup' */
   ctx->total_frames = ctx->n_frames;
   ctx->frame_start_time = 0.0;

   if( ctx->use_vbo )
   {
      int i;

      glGenBuffers( 3, ctx->vbo_names );
      glErrorCheck(__LINE__);

      assert( ctx->vbo_names[VBO_INDEX] != 0 );
      assert( ctx->vbo_names[VBO_VERTEX] != 0 );
      assert( ctx->vbo_names[VBO_NORMAL] != 0 );

      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ctx->vbo_names[VBO_INDEX] );
      glBufferData( GL_ELEMENT_ARRAY_BUFFER, ctx->n_indices * sizeof( index_t ), ( void * ) ctx->indices, GL_STATIC_DRAW );
      glErrorCheck(__LINE__);

      glBindBuffer( GL_ARRAY_BUFFER, ctx->vbo_names[VBO_VERTEX] );
      glBufferData( GL_ARRAY_BUFFER, ctx->n_elems * COMPONENTS * sizeof( elem_t ), ( void * ) ctx->elems, GL_STATIC_DRAW );
      glErrorCheck(__LINE__);

      glBindBuffer( GL_ARRAY_BUFFER, ctx->vbo_names[VBO_NORMAL] );
      glBufferData( GL_ARRAY_BUFFER, ctx->n_elems * COMPONENTS * sizeof( elem_t ), ( void * ) ctx->normals, GL_STATIC_DRAW );
      glErrorCheck(__LINE__);

      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
      glBindBuffer( GL_ARRAY_BUFFER, 0 );

      free( ctx->indices );
      free( ctx->elems );
      free( ctx->normals );
      ctx->indices = NULL;
      ctx->elems = NULL;
      ctx->normals = NULL;
   }
   return 0;
}


static void reshape(mmark_context *ctx)
{
   float scale  = 2*ctx->tri_scale;
   float fudge  = 0.0;
    GLint width  = ctx->window_width;
    GLint height = ctx->window_height;
   
   glViewport(0, 0, width, height);

   load_identity();
#ifdef GLES
   mv_scale(scale/width, scale/height, 1.0);
   mv_translate(-width/scale+fudge, -height/scale+fudge, 0);
#else
   glScalef(scale/width, scale/height, 1); 
   glTranslatef(-width/scale+fudge, 0, 0);
   glTranslatef(0, -height/scale+fudge, 0);
#endif
}

static void setup_shader_symbols(mmark_context *ctx) {

   GLint vertex_pos;
   GLint normal_pos;
   GLint color_pos;
   GLint mv_mat_pos;
   GLfloat draw_color[] = { 1,1,1,1 };

   vertex_pos   = glGetAttribLocation(program, "glw_Vertex");
   normal_pos   = glGetAttribLocation(program, "glw_Normal");
   mv_mat_pos   = glGetUniformLocation(program, "modelview_mat");
   color_pos   = glGetUniformLocation(program, "draw_color");

   if(vertex_pos != -1)
   {
      glEnableVertexAttribArray( vertex_pos );
      if( ctx->use_vbo )
      {
         glBindBuffer( GL_ARRAY_BUFFER, ctx->vbo_names[VBO_VERTEX] );
         glErrorCheck(__LINE__);
      }
      glVertexAttribPointer( vertex_pos, 3, ELEM_TYPE, GL_TRUE, STRIDE, ctx->elems );
   }
   if(normal_pos != -1)
   {
      glEnableVertexAttribArray( normal_pos );
      if( ctx->use_vbo )
      {
         glBindBuffer( GL_ARRAY_BUFFER, ctx->vbo_names[VBO_NORMAL] );
         glErrorCheck(__LINE__);
      }
      glVertexAttribPointer( normal_pos, 3, ELEM_TYPE, GL_TRUE, STRIDE, ctx->normals );
   }
   if(color_pos != -1) {
      glUniform4fv(color_pos, 1, draw_color);
   }
   if(mv_mat_pos == -1) {
      message("Warning: No modelview matrix present in shader.\n");
      exit(1);
   }
}

/* draws the demo */
static void drawFrame(mmark_context *ctx)
{
   GLenum errcode;   
   unsigned i, j;
   GLint mv_mat_pos;

   if( ctx->n_frames == 0 )
   {
      ctx->frame_start_time = get_time();
   }

   glClearColor( 1,0,0,0 );
   glClear( GL_COLOR_BUFFER_BIT );

   if( ctx->use_vbo )
   {
      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ctx->vbo_names[VBO_INDEX] );
      glErrorCheck(__LINE__);
   }

   if( ! ctx->swaponly )
   {

      mv_mat_pos   = glGetUniformLocation(program, "modelview_mat");
      matrix_copy(model_view_matrix, saved_matrix);
      for( j = 0; j < ctx->n_draws_vertical; ++j )
      {
         for( i = 0; i < ctx->n_draws_horizontal; ++i )
         {
#ifdef GLES
            mv_translate(ctx->box_width * i, ctx->box_height*j, 0);

            matrix_transpose(model_view_matrix);

            glUniformMatrix4fv(mv_mat_pos, 1, GL_FALSE, model_view_matrix);
#else
            glTranslatef(ctx->box_width * i * ctx->tri_scale, ctx->box_height * j * ctx->tri_scale, 0);
#endif
            if( ctx->use_quads )
            {
               int k;
               for( k = 0; k < ctx->n_indices - 2; k += 2 )
               {
                  glDrawElements( GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, ctx->indices + k );
               }
            }
            else
            {
               glDrawElements( GL_TRIANGLE_STRIP, ctx->n_indices, GL_UNSIGNED_SHORT, ctx->indices );
            }

            matrix_copy(saved_matrix, model_view_matrix);
            glErrorCheck(__LINE__);
         }
      }
   }

   ++ctx->n_frames;
   ++ctx->total_frames;
   if(ctx->n_frames > 0 && (ctx->n_frames % ctx->fpsevery) == 0)
   {
      float fps = ctx->n_frames / (get_time() - ctx->frame_start_time);

      ctx->n_frames = 0;
   }

#ifdef GLES
   eglSwapBuffers(dpy, surface);
#else
   glutSwapBuffers();
   glutPostRedisplay();
#endif
}

int main(int argc, char** argv)
{
   GLenum errcode;
    mmark_context context;

    /* Set context defaults */
   set_defaults(&context);

    /* Read in command-line arguments */
   parse_cli(&context, argc, argv);

    /* Set derived values in context */
   context.n_draws_horizontal = context.window_width/context.tri_scale/context.box_width;
   context.n_draws_vertical = context.window_height/context.tri_scale/context.box_height;
   assert(context.box_width*context.n_draws_horizontal <= context.window_width);
   assert(context.box_height*context.n_draws_vertical <= context.window_height);
    
#ifdef GLES
   init_egl(&context);
   shader_setup( context.fshader, context.vshader );
   reshape(&context);
#else
   glutInit(&argc, argv);
   glutInitWindowSize(context.window_width, context.window_height);
   glutInitDisplayMode ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
   glutCreateWindow("MMark");
#endif
   init(&context);
   glErrorCheck(__LINE__);

#ifdef GLES
   setup_shader_symbols(&context);
   while(context.infinite_loop || context.total_frames < context.quit_on_frame)
   {
      drawFrame(&context);
   }
#else
   glutDisplayFunc(drawFrame);
   glutReshapeFunc(reshape);
   glutMainLoop();
#endif
   return 0;
}
