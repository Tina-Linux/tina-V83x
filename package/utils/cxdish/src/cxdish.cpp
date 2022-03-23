/*
* Conexant CxDish
*
* Copyright:   (C) 2013 Conexant Systems
*
*
*************************************************************************
*  Modified Date:  6/25/2015
*  File Version:   1.0.0.6
*************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include "capehostx.h"
#include "CxFlash.h"
#include <unistd.h>
#if defined(_MSC_VER)
inline void* malloc(size_t size)
{
	return (void*) new char[size];
}
#endif
#define CXDISH_VERSION_STR "1.0.0.6"
#define HAS_GPIO_RESET_PIN 1

#define DEF_MAX_I2C_WRITE_LEN 64 /*max = 0x1000*/
#define DEF_MAX_I2C_READ_LEN 16

#define MAX_DEV_NAME 256
char g_devname[MAX_DEV_NAME]="/dev/i2c-2";
#define I2C_CHIP_ADDRESS    0x41
#define GPIO_RESET_PIN     1

typedef void *HANDLE;
HANDLE OpenI2cDevice();
void CloseI2cDevice(HANDLE hI2cDevice);
HANDLE OpenGpioDevice(int gpio_reset_pin);
void CloseGpioDevice(HANDLE device);
void SetResetPin(HANDLE hI2cDevice, unsigned long bHigh);
void *__gxx_personality_v0;
static int quiet = 0;
extern "C" {
int debugflag = 0;
}
static int ignore_error = 0;
static int golem_intf   = 0;
static int g_gpio_reset_pin = GPIO_RESET_PIN;
static int  g_max_i2c_write = DEF_MAX_I2C_WRITE_LEN;
static int  g_max_i2c_read = DEF_MAX_I2C_READ_LEN;
static unsigned char  g_ChipAddress = I2C_CHIP_ADDRESS;
extern "C" {
	extern unsigned int        g_cbMaxI2cWrite;
	extern unsigned int        g_cbMaxI2cRead;
	extern unsigned char       g_bChipAddress;                   /*Specify the i2c chip address*/
}

static int g_flash_legacy = 0;
static int g_hot_flash = 0;

Command cmd;


#define PUT_CHAR(_x_) putchar(_x_)

#define SENDCMD_MSG_BEGIN      0x98
#define SENDCMD_MSG_END        0x9c
#define SENDCMD_MSG_TYPE_OK    0Xa9
#define SENDCMD_MSG_TYPE_ERROR 0xa2
#define SENDCMD_MSG_DATA_MASK  0xd0

#define HINIBBLE(_x_) (((unsigned char) (_x_)) >> 4)
#define LONIBBLE(_x_) (((unsigned char) (_x_)) & 0x0f)

#define PUT_DATA(_data_,_check_sum_) \
	PUT_CHAR( HINIBBLE(_data_) | SENDCMD_MSG_DATA_MASK ); \
	PUT_CHAR( LONIBBLE(_data_) | SENDCMD_MSG_DATA_MASK ); \
	(_check_sum_) += (_data_);

int i2c_write_imp(HANDLE i2c_dev, unsigned char slave_addr, unsigned long sub_addr,
	unsigned long write_len,unsigned char* write_buf);

int i2c_read_imp(HANDLE i2c_dev, unsigned char slave_addr,
	unsigned long sub_addr, unsigned long rd_len, unsigned char*rd_buf);


void send_to_golem(bool bIsErr, unsigned char size, unsigned long * pData)
{
	unsigned char type = bIsErr? SENDCMD_MSG_TYPE_ERROR : SENDCMD_MSG_TYPE_OK;
	unsigned char checksum =0;
	PUT_CHAR(SENDCMD_MSG_BEGIN);
	PUT_CHAR( type ) ;
	PUT_DATA( size , checksum) ;
	for(int i=0;i<size;i++)
	{
		unsigned long lData = pData[i] ;
		PUT_DATA(  lData >> 24          , checksum) ;
		PUT_DATA( (lData >> 16 ) & 0xff , checksum) ;
		PUT_DATA( (lData >> 8 )  & 0xff , checksum) ;
		PUT_DATA( (lData)        & 0xff , checksum) ;
	}
	PUT_DATA( checksum , checksum) ;
	PUT_CHAR(SENDCMD_MSG_END);
}

int cxdish_sendcmd(unsigned int argc, char *argv[])
{
	int err = 0;

	if (argc < 2) {
		printf( "Specify a register address\n");
		return -EINVAL;
	}

	int          num_32b_words = argc -2;
	unsigned int app_module_id;
	unsigned int command_id;

	if (sscanf(argv[0], "%x", &app_module_id) != 1)
		return -EINVAL;


	if (sscanf(argv[1], "%x", &command_id) != 1)
		return -EINVAL;

	cmd.num_32b_words =  num_32b_words;
	cmd.command_id    = command_id;
	cmd.reply         = 0;
	cmd.app_module_id = app_module_id;

	for (int n = 0 ; n < num_32b_words ; n++)
	{
		if (sscanf(argv[n+2], "%x", &cmd.data[n]) != 1)
		{
			return -EINVAL;
		}
	}

	int result = SendCmdV(&cmd) ;
	if( result >= 0 )
	{
		if(golem_intf)
		{   // send to golem tool.
			send_to_golem(false,result,(unsigned long*)cmd.data);
		}
		else
		{
			// print out the result to screen
			printf("=> ");
			for(int i=0;i<cmd.num_32b_words;i++)
				printf("0x%08x ",cmd.data[i]);
			printf("\n");
		}

	}
	else
	{
		// error occured
		if(golem_intf)
		{   // send to golem tool.
			send_to_golem(true,1,(unsigned long*)&result);
		}
		else
		{
			// print out the result to screen
			printf("ERROR: failed to call sendcmd : err no=%d \n",result);
		}
	}
	return err;
}
int file_length(FILE *f);
int cxdish_flash(unsigned int argc, char *argv[])
{


	HANDLE            hDevice      = NULL;
	HANDLE	      hGpioDevice  = NULL;
	unsigned char    *pBootloader  =  NULL;
	unsigned char    *pImage       =  NULL;
	unsigned long     cbBootloader = 0;
	unsigned long     cbImage      = 0;

	int               ErrNo        = 0;
	FILE*             hBootLoader  = NULL;
	FILE*             hImage       = NULL;
	void             *buf          = NULL;



	do{
		if (argc < 1) {
			printf( "did not specify sfs file path.\n");
			return -EINVAL;
		}

		// Attempt to open i2c device.
		hDevice = OpenI2cDevice();
		if(hDevice == NULL )
		{
			// I2C device is not ready, return 1.
			printf( "I2c device is not present.\n");
			return -ENODEV;
		}

		hGpioDevice = OpenGpioDevice(g_gpio_reset_pin);
		if(hGpioDevice == NULL )
		{
			// GPIO is not available.
			printf( "GPIO %d is not present.\n",g_gpio_reset_pin);
			return -ENODEV;
		}


		//
		// Load BootLoader from file to memory.
		//
		if( argc ==2 )
		{
			hBootLoader= fopen(argv[1],"rb");
			if(hBootLoader  )
			{
				// store the file size for later use.
				cbBootloader = file_length(hBootLoader);

				// allocate the memory to store the bootlaod.
				pBootloader = (unsigned char*) malloc(cbBootloader);
				if (fread(pBootloader, sizeof(char), cbBootloader, hBootLoader) != cbBootloader)
				{
					printf( "read bootload file failed.\n");
					break;
				}
				fclose(hBootLoader);
				hBootLoader = NULL;
			}
		}


		//
		// Load cape image from file to memory.
		//
		hImage= fopen(argv[0],"rb");
		if(hImage == NULL )
		{
			printf( "Open eeprom file failed.\n");
			break;
		}
		// save the file size for later use.
		cbImage = file_length(hImage);
		// allocate the memory to store the bootlaod.
		pImage = (unsigned char*) malloc(cbImage);
		if (fread(pImage, sizeof(char), cbImage, hImage) != cbImage)
		{
			printf( "read bootload file failed.\n");
			break;
		}
		fclose(hImage);
		hImage = NULL;

		//
		// setup the i2c callback functions.
		//
		SetupI2cWriteMemCallback( (void *) hDevice, (fn_I2cWriteMem) i2c_write_imp,g_max_i2c_write);
		SetupI2cReadMemCallback( (void *) hDevice, (fn_I2cReadMem) i2c_read_imp, g_max_i2c_read);

#ifdef  HAS_GPIO_RESET_PIN
		// setup the reset pin callback function.
		SetupSetResetPin((void *) hGpioDevice,(fn_SetResetPin) SetResetPin );
#endif // HAS_GPIO_RESET_PIN


		buf = malloc(GetSizeOfBuffer());


		//
		//  Download FW.
		//
		//If the operation completes successfull,the return value is zero. Otherwise,
		//return EERON_* error code. For more information about return code, please refer
		//to cxpump.h file.
		ErrNo = DownloadFW(buf, pBootloader, cbBootloader,pImage, cbImage, g_ChipAddress , SFS_UPDATE_AUTO,g_hot_flash?0:1,g_flash_legacy);


		if(ErrNo)
		{
			printf("\nFailed! ERROR CODE = %d\n\n",ErrNo);
		}
		else
		{
			printf("Firmware Downloaded Successfully\n");
		}


	}while(0);

	// Clean up.
	if(pImage) free(pImage);
	if(pBootloader) free(pBootloader);
	CloseI2cDevice(hDevice);
	if(hGpioDevice)  CloseGpioDevice(hGpioDevice);
	if(  hBootLoader ) fclose(hBootLoader);
	if( hImage ) fclose( hImage );
	if( buf ) free( buf);
	return ErrNo;
}

static int getfwver(void)
{
	int ret_val = 0;
	HANDLE dev = OpenI2cDevice();
	if(dev)
	{
		//
		// setup the i2c callback functions.
		//
		SetupI2cWriteMemCallback( (void *) dev, (fn_I2cWriteMem) i2c_write_imp,g_max_i2c_write);
		SetupI2cReadMemCallback( (void *) dev, (fn_I2cReadMem) i2c_read_imp, g_max_i2c_read);

		cmd.command_id    =  0x0103;
		cmd.reply         = 0;
		cmd.app_module_id = 0xb32d2300;

		if( SendCmdV(&cmd) >0 )
		{
			printf("Firmware Version: %u.%u.%u.%u\n",cmd.data[0],cmd.data[1],cmd.data[2],cmd.data[3] );
		}
		else
		{
			printf("ERROR: Failed to get firmware version!\n");
			ret_val =2;
		}
		CloseI2cDevice(dev);
	}
	else
	{
		printf("ERROR: Failed to open I2C device!\n");
		ret_val = 1;
	}
	return ret_val;
}

static int get_mode()
{

	int ret ;
	char data[5];
	unsigned char index;
	HANDLE dev = OpenI2cDevice();
	if(dev == NULL)
	{
		printf("ERROR: Failed to open I2C device!\n");
		return -1;
	}
	SetupI2cWriteMemCallback( (void *) dev, (fn_I2cWriteMem) i2c_write_imp,g_max_i2c_write);
	SetupI2cReadMemCallback( (void *) dev, (fn_I2cReadMem) i2c_read_imp, g_max_i2c_read);

	cmd.command_id = 0x12f; /*CMD_GET(SOS_RESOURCE);*/
	cmd.reply = 0;
	cmd.app_module_id = APP_ID ( 'S', 'O', 'S', ' ');
	cmd.num_32b_words = 1;
	cmd.data[0] = APP_ID ( 'C', 'T', 'R', 'L');
	ret = SendCmdV(&cmd);
	if( ret <= 0 || ret > MAX_COMMAND_SIZE )
		printf("Failed to get tv mode, sendcmd error = %d\n",ret);
	else {
		data[0]= CHAR_FROM_CAPE_ID_A(cmd.data[0]);
		data[1] = CHAR_FROM_CAPE_ID_B(cmd.data[0]);
		data[2] = CHAR_FROM_CAPE_ID_C(cmd.data[0]);
		data[3] = CHAR_FROM_CAPE_ID_D(cmd.data[0]);
		index   = (unsigned char)(cmd.data[0]&0xff);
		if( index == 0)
		{
			printf("Current tv mode = \"%c%c%c%c\" \n",
				data[0],
				data[1],
				data[2],
				data[3]);
		}
		else
			printf("Current tv mode = \"%c%c%c%c|%d\" \n",
			data[0],
			data[1],
			data[2],
			data[3],
			index);
		ret = 0;
	}

	CloseI2cDevice(dev);
	return ret;
}

static int set_mode(unsigned int argc, char *argv[])
{

	int ret;
	int len;
	unsigned int index = 0;
	HANDLE dev ;
	if (argc < 1) {
		printf( "ERROR: Did not specify MODE id.\n");
		return -EINVAL;
	}

	len = strlen(argv[0]);

	if( len < 4 || len == 5) {
		printf( "ERROR: set-mode failed, mode id is not correct\n");
		return -EINVAL;
	}

	if( len >5 && argv[0][4]!='|')
	{
		printf( "ERROR: set-mode failed, mode id is not correct\n");
		return -EINVAL;
	}

	if ( len >5  )
	{
		sscanf(&(argv[0][5]),"%u",&index );
	}


	dev = OpenI2cDevice();
	if(dev == NULL)
	{
		printf("ERROR: Failed to open I2C device!\n");
		return -1;
	}

	SetupI2cWriteMemCallback( (void *) dev, (fn_I2cWriteMem) i2c_write_imp,g_max_i2c_write);
	SetupI2cReadMemCallback( (void *) dev, (fn_I2cReadMem) i2c_read_imp, g_max_i2c_read);

	cmd.command_id =  4;/*CMD_SET(CONTROL_APP_EXEC_FILE)*/
	cmd.reply = 0;
	cmd.app_module_id = APP_ID ( 'C', 'T', 'R', 'L');
	cmd.num_32b_words = 1;
	cmd.data[0] = APP_ID(argv[0][0],
		argv[0][1],
		argv[0][2],
		argv[0][3]) + (index&0xff);

	ret = SendCmdV(&cmd);
	if( ret < 0 || ret > MAX_COMMAND_SIZE ) {
		printf( "ERROR: failed to set mode %s, sendcmd error = %d \n",argv[0],ret );
		ret = -1;
	} else {
		printf( "set mode to \"%s\"\n",argv[0] );
		ret = 0 ;
	}
	CloseI2cDevice(dev);
	return ret;
}



/*
* split a line into tokens
* the content in the line buffer is modified
*/
int split_line(char *buf, char **token, int max_token)
{
	char *dst;
	int n, esc, quote;

	for (n = 0; n < max_token; n++) {
		while (isspace(*buf))
			buf++;
		if (! *buf || *buf == '\n')
			return n;
		/* skip comments */
		if (*buf == '#' || *buf == '!')
			return n;
		esc = 0;
		quote = 0;
		token[n] = buf;
		for (dst = buf; *buf && *buf != '\n'; buf++) {
			if (esc)
				esc = 0;
			else if (isspace(*buf) && !quote) {
				buf++;
				break;
			} else if (*buf == '\\') {
				esc = 1;
				continue;
			} else if (*buf == '\'' || *buf == '"') {
				if (! quote) {
					quote = *buf;
					continue;
				} else if (*buf == quote) {
					quote = 0;
					continue;
				}
			}
			*dst++ = *buf;
		}
		*dst = 0;
	}
	return n;
}
#define MAX_ARGS  32

static int exec_stdin(void)
{
	int narg;
	char buf[256], *args[MAX_ARGS];
	int err = 0;
	HANDLE dev = OpenI2cDevice();
	if(dev == 0)
	{
		printf("ERROR: Failed to open device\n");
		return -1;
	}

	//
	// setup the i2c callback functions.
	//
	SetupI2cWriteMemCallback( (void *) dev, (fn_I2cWriteMem) i2c_write_imp,g_max_i2c_write);
	SetupI2cReadMemCallback( (void *) dev, (fn_I2cReadMem) i2c_read_imp, g_max_i2c_read);

	/* quiet = 1; */
	ignore_error = 1;

	while (fgets(buf, sizeof(buf), stdin)) {
		narg = split_line(buf, args, MAX_ARGS);
		if (narg > 0) {
			if (!strcmp(args[0], "sendcmd"))
				err = cxdish_sendcmd(narg - 1, args + 1);
			if (!strcmp(args[0], "quit"))
			{
				break;
			}
		}
	}
	CloseI2cDevice(dev);
	return err;
}


static int help(void)
{
	printf("Conexant CxDish version: %s\n",CXDISH_VERSION_STR);
	printf("Usage: cxdish <options> [command]\n");
	printf("\nAvailable options:\n");
	printf("  -h, --help                this help\n");
	printf("  -D, --device STR          specifies the i2c device path, default is '%s'\n", "/dev/i2c-0");
	printf("  -r, --reset GPIO_NUM      specifies the RESET GPIO pin number, default is '%d'\n", GPIO_RESET_PIN );
	printf("  -a, --address I2C_ADDRESS specifies i2c address, default is '0x%02x'\n",I2C_CHIP_ADDRESS) ;
	printf("  -d, --debug LEVEL         specifies device level from 0 to 2\n");
	printf("  -W, --max_write           specifies max writing lenght,default is %d, max= 4000\n",DEF_MAX_I2C_WRITE_LEN);
	printf("  -R, --max_read            specifies max writing lenght,default is %d, max= 4000\n",DEF_MAX_I2C_READ_LEN);
	printf("  -L, --legacy              enables legacy mode flash for legacy device\n");
	printf("  -H, --hot_flash           download image during firmware is running\n");
	printf("  -d, --debug               enables i2c dump\n");
	printf("  -v, --version             prints cxdish version\n");
	printf("  -q, --quiet               enables quiet mode\n");
	printf("\nAvailable commands:\n");
	printf("  sendcmd STREAM_ID COMMAND [arg]\n");
	printf("  flash SFS_FILE [BOOTLOADER] \n");
	printf("  set-mode MODE\n");
	printf("  get-mode\n");
	printf("  fw-version\n");
	return 0;
}

HANDLE device =NULL;
int main(int argc, char *argv[])
{

	int morehelp= 0;
	int read_stdin = 0;
	uint32_t  addr;
	static const struct option long_option[] =
	{
		{"help",no_argument, NULL, 'h'},
		{"quiet", no_argument, NULL, 'q'},
		{"debug", required_argument, NULL, 'd'},
		{"version", no_argument, NULL, 'v'},
		{"stdin", no_argument, NULL, 's'},
		{"golem",required_argument,NULL,'g'},
		{"device",required_argument,NULL,'D'},
		{"address",required_argument,NULL,'a'},
		{"reset",required_argument,NULL,'r'},
		{"max_write",required_argument,NULL,'W'},
		{"max_read",required_argument,NULL,'R'},
		{"legacy",no_argument,NULL,'L'},
		{"hot_flash",no_argument,NULL,'H'},
		{NULL, 0, NULL, 0},
	};


	morehelp = 0;
	while (1) {
		int c;

		if ((c = getopt_long(argc, argv, "qd:ghvsD:a:r:W:R:LH", long_option, NULL)) < 0)
			break;
		switch (c) {
		case 'h':
			help();
			return 0;
		case 'q':
			quiet = 1;
			break;
		case 'd':
			sscanf(optarg,"%d",&debugflag );
			break;
		case 'v':
			printf("cxdish version " CXDISH_VERSION_STR "\n");
			return 1;
		case 's':
			read_stdin = 1;
			break;
		case 'g':
			golem_intf = 1;
			break;
		case 'r':
			sscanf(optarg,"%d",&g_gpio_reset_pin );
			break;
		case 'a':
			sscanf(optarg,"%x",&addr);
			g_ChipAddress =(uint8_t)addr;
			break;
		case 'D':
			strncpy(g_devname,optarg,MAX_DEV_NAME-1);
			break;
		case 'W':
			sscanf(optarg,"%d",&g_max_i2c_write );
			if ( g_max_i2c_write < 6)
			{
				printf( "ERROR: writing length is too small, length must >=6\n\n");
				return -1;
			}
			break;
		case 'R':
			sscanf(optarg,"%d",&g_max_i2c_read );
			if ( g_max_i2c_read < 4)
			{
				printf( "ERROR: reading length is too small, length must >=4\n\n");
				return -1;
			}
			break;
		case 'L':
			g_flash_legacy = 1;
			break;
		case 'H':
			g_hot_flash = 1;
			break;
		default:
			printf( "ERROR: Invalid switch or option needs an argument.\n\n");
			morehelp++;
		}
	}

	if (morehelp) {
		help();
		return 1;
	}

	if (read_stdin)
		return exec_stdin();


	if (argc - optind <= 0) {
		help();
		return 1;
	}

	//update I2C settings
	g_bChipAddress = g_ChipAddress;
	g_cbMaxI2cWrite = g_max_i2c_write;
	g_cbMaxI2cRead = g_max_i2c_read;

	if (!strcmp(argv[optind], "help")) {
		return help() ? 1 : 0;
	} else if (!strcmp(argv[optind], "sendcmd")) {
		HANDLE dev = OpenI2cDevice();
		if(dev)
		{
			//
			// setup the i2c callback functions.
			//
			SetupI2cWriteMemCallback( (void *) dev, (fn_I2cWriteMem) i2c_write_imp,g_max_i2c_write);
			SetupI2cReadMemCallback( (void *) dev, (fn_I2cReadMem) i2c_read_imp, g_max_i2c_read);

			cxdish_sendcmd(argc - optind - 1, argc - optind > 1 ? argv + optind + 1 : NULL);
			CloseI2cDevice(dev);
		}
		else
		{
			printf( "ERROR: Failed to open I2C device!\n");
		}
	} else if (!strcmp(argv[optind], "flash")) {
		return cxdish_flash(argc - optind - 1, argc - optind > 1 ? argv + optind + 1 : NULL) ? 1 : 0;
	} else if (!strcmp(argv[optind], "fw-version")) {
		return getfwver() ? 1 : 0;
	} else if (!strcmp(argv[optind], "set-mode")) {
		return set_mode(argc - optind - 1, argc - optind > 1 ? argv + optind + 1 : NULL) ? 1 : 0;
	} else if (!strcmp(argv[optind], "get-mode")) {
		return get_mode() ? 1 : 0;
	} else {
		printf( "ERROR: Unknown command '%s'...\n", argv[optind]);
	}
	return 0;
}




