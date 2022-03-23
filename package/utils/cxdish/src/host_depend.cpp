
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>		/* open */
#include <stdint.h>
#include <unistd.h>		/* exit */
#include <sys/ioctl.h>		/* ioctl */
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
/*#include <endian.h>*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdarg.h>
#include <ctype.h>

#include <errno.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include "CxFlash.h"
#include "capehostx.h"

extern "C" {
extern unsigned int        g_cbMaxI2cWrite;
extern unsigned int        g_cbMaxI2cRead;
extern unsigned char       g_bChipAddress;                   /*Specify the i2c chip address*/
extern unsigned char       g_bAddressOffset;
extern void *              g_pContextI2cWrite;
extern void *              g_pContextI2cWriteThenRead ;
extern void *              g_pContextSetResetpin ;
}
static char* g_i2c_buf= NULL;
#define MAX_DEV_NAME 256
extern char g_devname[MAX_DEV_NAME];

static char gpio_name[MAX_DEV_NAME];

typedef void *HANDLE;
typedef HANDLE *PHANDLE;
#if defined(_MSC_VER)
/////////////////////////////////
// Microsoft windows environment.
#ifndef __BYTE_ORDER
#define  __BYTE_ORDER       __LITTLE_ENDIAN
#define  __LITTLE_ENDIAN    1234

//
// Handle to an Object
//

#endif //#define __BYTE_ORDER
#include <string.h>
#pragma comment( lib,"..\\..\\Debug\\alsalib.lib" )
#pragma comment(lib,"setupapi")
#endif

#if defined( __BIG_ENDIAN) && !defined(__BYTE_ORDER)
#define __BYTE_ORDER __BIG_ENDIAN
#elif defined( __LITTLE_ENDIAN ) && !defined(__BYTE_ORDER)
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif

extern "C" {
extern int debugflag ;
}

HANDLE        g_hI2cDevice  = NULL;
int file_length(FILE *f);
HANDLE OpenI2cDevice();

void CloseI2cDevice(HANDLE hI2cDevice);
int I2cWrite(HANDLE hI2cDevice, unsigned char ChipAddr, unsigned long cbBuf, unsigned char* pBuf);
int I2cWriteThenRead(HANDLE hI2cDevice, unsigned char ChipAddr, unsigned long cbBuf,
    unsigned char* pBuf, unsigned long cbReadBuf, unsigned char*pReadBuf);
void SetResetPin(HANDLE hI2cDevice, unsigned long bHigh);

int i2c_write_imp(HANDLE i2c_dev, unsigned char slave_addr, unsigned long sub_addr,
    unsigned long write_len,unsigned char* write_buf);

int i2c_read_imp(HANDLE i2c_dev, unsigned char slave_addr,
    unsigned long sub_addr, unsigned long rd_len, unsigned char*rd_buf);

HANDLE OpenGpioDevice(int gpio_reset_pin);
void CloseGpioDevice(HANDLE device);

/*
 * Convert a 4-byte number from a ByteOrder into another ByteOrder.
 */
unsigned long ByteOrderSwapULONG(unsigned long i)
{
    return((i&0xff)<<24)+((i&0xff00)<<8)+((i&0xff0000)>>8)+((i>>24)&0xff);
}
/*
 * Convert a 4-byte number from generic byte order into Big Endia
 */
unsigned long ToLittleEndiaULONG(unsigned long i)
{
#if (__BYTE_ORDER != __LITTLE_ENDIAN)
    return ByteOrderSwapULONG(i);
#else
    return i;
#endif
}


int  i2c_write (int address, unsigned int *buffer, int size)
{
	i2c_write_imp(g_pContextI2cWrite,  g_bChipAddress, address,
			size*4,(unsigned char*)buffer);
    return 0;
}

int  i2c_read(int address, unsigned int *buffer, int size)
{
	i2c_read_imp(g_pContextI2cWriteThenRead, g_bChipAddress,
			address, size *4,(unsigned char*) buffer);
    return 0;
}
extern "C" void sys_mdelay(unsigned int ms_delay)
{
#ifdef _WINDOWS
    Sleep(ms_delay);
#else
    usleep(ms_delay*1000);
#endif
}
//
// GPIO functions
//
/* export the gpio to user mode space */
static int export_gpio(int gpio)
{
        int err = 0;
        int fd;
        int wr_len;

        /* check if gpio is already exist or not */
        sprintf(gpio_name, "/sys/class/gpio/gpio%d/value", gpio);

        fd = open(gpio_name,O_RDONLY);
        if ( fd >= 0 ) {
                /* gpio device has already exported */
                close(fd);
                return 0;
        }

        /* gpio device is no exported, export it */
        strcpy(gpio_name,"/sys/class/gpio/export");

        fd = open(gpio_name,O_WRONLY);

        if (fd < 0) {
                fprintf(stderr, "failed to open device: %s\n", gpio_name);
                return -ENOENT;
        }

        sprintf(gpio_name,"%d",gpio);
        wr_len = strlen(gpio_name);

        if (write(fd,gpio_name,wr_len)!=wr_len) {
                fprintf(stderr, "failed to export gpio: %d\n", gpio);
                return -EACCES;
        }

        return err;
}

static int free_gpio( int gpio)
{
        int err = 0;
        int fd;
        int wr_len;

        /* check if gpio exist */
        sprintf(gpio_name, "/sys/class/gpio/gpio%d/value", gpio);

        fd = open(gpio_name,O_RDONLY);
        if ( fd < 0 ) {
                /* gpio device doesn't exist */
                return -ENOENT;
        }
        close(fd);

        /* gpio device is no exported, export it */
        strcpy(gpio_name,"/sys/class/gpio/unexport");

        fd = open(gpio_name,O_WRONLY);

        if (fd < 0) {
                fprintf(stderr, "failed to open device: %s\n", gpio_name);
                return -ENOENT;
        }

        sprintf(gpio_name,"%d",gpio);
        wr_len = strlen(gpio_name);

        if (write(fd,gpio_name,wr_len)!=wr_len) {
                fprintf(stderr, "failed to free gpio: %d\n", gpio);
                return -EACCES;
        }

        return err;

}

static int set_gpio_dir(int gpio)
{
        int err = 0;
        int fd;
        int wr_len;
        char str_dir[4];

        err = export_gpio(gpio);

        if (err)
                return err;

        sprintf(gpio_name, "/sys/class/gpio/gpio%d/direction", gpio);

        fd = open(gpio_name,O_WRONLY);

        if (fd < 0) {
                fprintf(stderr, "failed to open device: %s\n", gpio_name);
                err = -ENOENT;
                goto exit;
        }

        strcpy(str_dir,"out");
        wr_len = strlen(str_dir);
        if (write(fd, str_dir,wr_len) != wr_len) {
                fprintf(stderr, "Failed to set gpio %s direction to %s \n", gpio_name, str_dir);
                err =  -EACCES;
                goto exit;
        }
	if (debugflag)
        	printf("set gpio %d to %s\n",gpio,str_dir);
exit:
        close(fd);
	return err;
}

static int set_gpio_value(int gpio, int value)
{
        int err = 0;
        int fd;

//        err = export_gpio(gpio);

        if (err)
                return err;

        sprintf(gpio_name, "/sys/class/gpio/gpio%d/value", gpio);

        fd = open(gpio_name,O_WRONLY);

        if (fd < 0) {
                fprintf(stderr, "failed to open device: %s\n", gpio_name);
                err = -ENOENT;
                goto exit;
        }


        if (write(fd, value? "1":"0", 1) != 1) {
                fprintf(stderr, "Failed to set gpio %s to %d\n", gpio_name, value);
                err =  -EACCES;
                goto exit;
        }

	if (debugflag)
        	printf("set gpio %d to %d\n",gpio,value);
exit:
        close(fd);

        return err;
}
HANDLE OpenI2cDevice()
{
    	int hfile = 0;
#ifndef _MSC_VER
	//TODO: change the device name if codec is not at i2c bus 0
	hfile = open(g_devname,O_RDWR);

	if (debugflag)
        	printf("I2C: open i2c device from %s\n",g_devname);

#else
    	hfile =1;
#endif
	g_hI2cDevice = *((HANDLE*)&hfile);
    	return g_hI2cDevice;
}
void CloseI2cDevice(HANDLE hI2cDevice)
{
	if(g_i2c_buf) free(g_i2c_buf);
     	close(*((int*)&hI2cDevice));
}

/* This is an I2C Write function.
 *
 * PARAMETERS
 *            i2c_dev          [in] - A pointer to a caller-supplied
 *                                    context area as specified in the
 *                                    CallbackContext parameter of
 *                                    SetupI2cWriteMemCallback.
 *            slave_addr       [in] - The i2c chip address.
 *            sub_addr         [in] - The i2c data address.
 *            write_len        [in] - The size of the output buffer, in bytes.
 *            write_buf        [in] - A pointer to the putput buffer that contains
 *                                    the data required to perform the operation.
 * RETURN
 *    If the operation completes successfully, the return value is ERRNO_NOERR.
 *    Otherwise, return ERRON_* error code.
 */
int i2c_write_imp(HANDLE i2c_dev, unsigned char slave_addr, unsigned long sub_addr,
    unsigned long write_len,unsigned char* write_buf)
{
	int fd = (size_t) i2c_dev;
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg msg[1];
	int ret;
	int i;

	if (g_i2c_buf == NULL) {
		g_i2c_buf = (char*)malloc(g_cbMaxI2cWrite+g_bAddressOffset);
		if (g_i2c_buf == NULL) {
			return -ERRNO_NOMEM;
		}
	}

	for(i=0; i<g_bAddressOffset;i++)
		g_i2c_buf[i] = (unsigned char)(sub_addr>>((g_bAddressOffset-i-1)*8));

	memcpy(&g_i2c_buf[i],write_buf,write_len);


	msg[0].addr = slave_addr;
	msg[0].flags = 0; //7 bits
	msg[0].len = write_len+g_bAddressOffset;
	msg[0].buf = (uint8_t*) g_i2c_buf;

	packets.msgs = msg;
	packets.nmsgs = 1;

	if (debugflag){
        	unsigned int i=0;
        	printf("I2C: <START> %02X", slave_addr<<1);
        	for(i=0;i<msg[0].len ;i++) {
            		printf(" %02X",(0xff& g_i2c_buf[i]) );
        	}
        	printf(" <STOP>");
    }

	ret = ioctl(fd,I2C_RDWR, &packets);

	if (ret < 0) {
		if(debugflag)
			printf(" Failed: error = %d\n", ret);
       		return -ERROR_I2C_ERROR;
   	} else {
		if(debugflag)
			printf("\n");
	}

   return ERRNO_NOERR;
}

/*
 in case the syste has no htobe32 function we re-define it
*/
#ifndef htobe32
uint32_t htobe32( uint32_t x)
{
	union { uint32_t u32; uint8_t v[4]; } ret;
	ret.v[0] = (uint8_t) (x>>24);
	ret.v[1] = (uint8_t) (x>>16);
	ret.v[2] = (uint8_t) (x>> 8);
	ret.v[3] = (uint8_t) (x);
	return ret.u32;
}
#endif

/* This is a call back routine for doing I2C READ.
 * Before eading data from the salve device, you must tell slave device
 * what sub-address number is. So a read of the salve actually starts off by writing sub-address
 * to it.
 *
 * PARAMETERS
 *          i2c_dev            [in]  - A pointer to a caller-supplied
 *                                     context area as specified in the
 *                                     CallbackContext parameter of
 *                                     SetupI2cWriteMemCallback.
 *          slave_addr         [in]  - The i2c chip address.
 *          sub_addr           [in]  - slave addr.
 *          rd_len             [in]  - Specify the read data size.
 *          rd_buf             [out] - A pointer to the input buffer
 * RETURN
 *    If the operation completes successfully, the return value is ERRNO_NOERR.
 *    Otherwise, return ERRON_* error code.
 */
int i2c_read_imp(HANDLE i2c_dev, unsigned char slave_addr,
    unsigned long sub_addr, unsigned long rd_len, unsigned char*rd_buf)
{
  	int ret;
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg msg[2];
	int fd = (size_t) i2c_dev;
	unsigned long be_sub_addr =	htobe32(sub_addr);
	int i;

	msg[0].addr = slave_addr;
	msg[0].flags = 0; //7 bits
	msg[0].len = g_bAddressOffset;
	msg[0].buf = (uint8_t*)&be_sub_addr;

	for(i=0; i<g_bAddressOffset;i++)
		msg[0].buf[i] = (unsigned char)(sub_addr>>((g_bAddressOffset-i-1)*8));


	msg[1].addr = slave_addr;
	msg[1].flags = I2C_M_RD; //read flag
	msg[1].len = rd_len;
	msg[1].buf = rd_buf;

	packets.msgs = msg;
	packets.nmsgs = 2;

	ret = ioctl(fd,I2C_RDWR, &packets);
    	if( debugflag  ) {
        	unsigned int i=0;
        	printf("I2C: <START> %02X", slave_addr<<1);
        	for(i=0;i<msg[0].len ;i++)
            		printf(" %02X", msg[0].buf[i] );

        	printf(" <RESTART> %02X [", slave_addr<<1 |1);

        	for(i=0;i<msg[1].len;i++){
			if(i==0)
            		printf("%02X", msg[1].buf[i]);
			else
            		printf(" %02X", msg[1].buf[i]);
		}
        	printf("] <STOP>");

        	if( ret < 0 )
            		printf (" Failed\n");
		else
			printf ("\n");
    	}

	if (ret < 0)
       		return -ERROR_I2C_ERROR;

   	return ERRNO_NOERR;
}


HANDLE OpenGpioDevice(int gpio_reset_pin)
{
	if(set_gpio_dir(gpio_reset_pin)<0)
	{
		return NULL;
	}
	return *((HANDLE*) &gpio_reset_pin);
}
void CloseGpioDevice(HANDLE device)
{
	free_gpio(*((int*) &device));
}
void SetResetPin(HANDLE gpio_device, unsigned long bHigh)
{

	if (debugflag)
        	printf("GPIO: sets RESET pin to %s\n",bHigh?"HIGH":"LOW");

	set_gpio_value(*((int*) &gpio_device), bHigh?1:0);

}

/*
 * Return the size of the specified file, in bytes.
 */
int file_length(FILE *f)
{
    int pos;
    int end;

    pos = ftell (f);
    fseek (f, 0, SEEK_END);
    end = ftell (f);
    fseek (f, pos, SEEK_SET);

    return end;
}

#if defined(_MSC_VER)
extern "C" __declspec(dllimport) void __stdcall  Sleep(unsigned long dwMilliseconds);
int usleep (__useconds_t __useconds)
{
    Sleep(__useconds /1000);
    return 0;
}
#endif
