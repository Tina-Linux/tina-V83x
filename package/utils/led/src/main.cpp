// #include <stdarg.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
// #include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <endian.h>
#include <iostream>

#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */


#define SLAVE_ADDRESS       0x3F
#define ADDRESS_LENGTH  2
#define DATA_LENGTH         2

#define I2C_DUMP

static int led_hI2cDevice;
static char led_devname[20]="/dev/i2c-1";

static unsigned char  led_bChipAddress;                   /*Specify the i2c chip address*/
static unsigned char  led_bAddressOffset;


#ifdef I2C_DUMP
static int debugflag = 1;
#else
static int debugflag = 0;
#endif


void sys_mdelay(unsigned int ms_delay)
{
    usleep(ms_delay*1000);
}

int i2c_write(unsigned char reg_address, unsigned char val)
{
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg msg[1];
    unsigned char led_i2c_buf[2];
    unsigned int write_len = 2; //Addr + Data 2 bytes
    int fd = led_hI2cDevice;
    int ret = 0;
    int i;

    // led_i2c_buf[0] = SLAVE_ADDRESS;
    led_i2c_buf[0] = reg_address;
    led_i2c_buf[1] = val;

    msg[0].addr = led_bChipAddress;
    msg[0].flags = 0; //7 bits
    msg[0].len = write_len;
    msg[0].buf = led_i2c_buf;

    packets.msgs = msg;
    packets.nmsgs = 1;

    if (debugflag)
    {
        printf("I2C: <START> %02X", led_bChipAddress<<1);
        for(i=0;i<msg[0].len ;i++)
        {
            printf(" %02X",(0xff& led_i2c_buf[i]) );
        }
        printf(" <STOP>\n");
    }

    ret = ioctl(fd,I2C_RDWR, (unsigned long)&packets);



    if (ret < 0) {
        printf(" Failed: i2c_write error = %d (%s)\n", ret, strerror(errno));
        return -1;
    }
    return 0;
}


int i2c_init (unsigned char slave_address, int address_offset)
{
    int i2cdev  =  0;
    char *devname = NULL;

    led_bChipAddress = slave_address;
    led_bAddressOffset = address_offset;
/*    g_cbMaxI2cWrite = DEF_MAX_I2C_WRITE_LEN;
    g_cbMaxI2cRead = DEF_MAX_I2C_READ_LEN;*/

    devname = led_devname;

    i2cdev = open(devname, O_RDWR);

    printf("DEV I2C: open i2c device from %s\n",devname);

    if( i2cdev < 0 )
    {
        fprintf(stderr, "I2c device [%s] is not present.\n",devname);
        return -ENODEV;
    }

    led_hI2cDevice = i2cdev;
    return i2cdev;
}

int  i2c_close (void)
{
    close(led_hI2cDevice);

    return 0;
}
    unsigned char pwm_value = 0xFF;
    unsigned char pwm_half_value = 0x66;
    unsigned char config_value = 0x01;
    unsigned char reset_value = 0x00;

int global_en() {
    i2c_write(0x4A, 0);
}
int global_disable() {
    i2c_write(0x4A, 1);
}

int boot_stage_one() {
    int i = 0;
    int blue_start = 0x24;
    while(1) {
        i2c_write(blue_start, pwm_half_value);
        i2c_write(blue_start - 3, pwm_half_value);
        i2c_write(0x25, reset_value);
        i2c_write(blue_start - 1, pwm_half_value);
        i2c_write(blue_start - 4, pwm_half_value);
        i2c_write(0x25, reset_value);
        sys_mdelay(100);
        i2c_write(blue_start - 1, reset_value);
        if (blue_start - 4 <= 0) {
            blue_start = 0x24;
        } else {
            blue_start -= 3;
        }
    }
}

int boot_stage_two() {

    int i = 0;
    int blue_start = 0x24;
    while(1) {

        i2c_write(blue_start, pwm_half_value);
        i2c_write(0x25, reset_value);
        i2c_write(blue_start - 1, pwm_half_value);
        i2c_write(blue_start - 4, pwm_half_value);
        i2c_write(0x25, reset_value);
        sys_mdelay(100);
        i2c_write(blue_start - 1, reset_value);
        if (blue_start - 3 <= 0) {
            blue_start = 0x24;
        } else {
            blue_start -= 3;
        }
    }
}


int blue_single_loop(unsigned int msec) {

    int i = 0;
    for (i = 0x24; i > 0; i -= 3) {
        i2c_write(i, pwm_half_value);
        i2c_write(0x25, reset_value);
        sys_mdelay(msec); /*change this value to set different patterns*/
        i2c_write(i, reset_value);
    }  //blue
}

int blue_backgroud(unsigned int msec) {

    int i = 0;
    for (i = 0x24; i > 0; i -= 3) {
        i2c_write(i, 96);
        i2c_write(0x25, reset_value);
        sys_mdelay(msec); /*change this value to set different patterns*/
    }  //blue
}

int dirction_arrival(unsigned int sector) {
    // sector has 10 available values , to simplify, just set 1 to 10
    if (sector < 0 || sector > 11) {
        printf("sector is only valid from 0 to 11");
        return -1;
    }
    blue_backgroud(0); //blue_backgroud

    unsigned int sector_start = (sector + 6) % 12;
    /* it can be reuse*/
    printf("sector_start = %d\n", (int)sector_start);
    int left_start, right_start;

    if (0 == sector_start) {
        left_start = 0x23;
        right_start = 0x02;
    } else if(11 == sector_start) {
        left_start = 0x02;
        right_start = 0x05;
    } else {
        left_start = (12 - sector_start) * 3 - 1;
        right_start = (12 - sector_start + 1) * 3 - 1;
    }
    printf("left_start = %d, right_start = %d\n", left_start, right_start);
    /*fix me*/
    for (int i = 0; i < 5; ++i) {


        i2c_write(left_start, pwm_half_value);
        i2c_write(right_start, pwm_half_value);
        i2c_write(0x25, reset_value);
        sys_mdelay(100);
        i2c_write(left_start, reset_value);
        i2c_write(right_start, reset_value);
        if (left_start - 3 <= 0) {
            left_start = 35;
        } else
            left_start = left_start - 3;
        if (right_start + 3 >= 36) {
            right_start = 2;
        } else
            right_start = right_start + 3;
    }
    i2c_write(left_start, pwm_half_value);
    i2c_write(right_start, pwm_half_value);
    i2c_write(0x25, reset_value);
    return 0;
}

int dynamic_doa(unsigned int updated_sector) {
    blue_backgroud(0);
    int left_start, right_start;

    if (0 == updated_sector) {
        left_start = 0x23;
        right_start = 0x02;
    } else if(11 == updated_sector) {
        left_start = 0x05;
        right_start = 0x02;
    } else {
        left_start = (12 - updated_sector) * 3 - 1;
        right_start = (12 - updated_sector + 1) * 3 - 1;
    }
    i2c_write(left_start, pwm_half_value);
    i2c_write(right_start,pwm_half_value);
    i2c_write(0x25, reset_value);
    sys_mdelay(1000);

    printf("left_start = %d, right_start = %d\n", left_start, right_start);

    //for debug
    i2c_write(left_start, reset_value);
    i2c_write(right_start, reset_value);
    return 0;
}

int thinking(int sec) {
    blue_backgroud(0);

    while (sec > 0) {
        /*stage 1*/
        for (int i = 0x23; i > 0; i -= 6) {
            i2c_write(i, pwm_half_value);
            i2c_write(0x25, reset_value);
        }
        sys_mdelay(100);
        for (int i = 0x23; i > 0; i -= 6) {
            i2c_write(i, reset_value);
        }
        /*stage 2*/
        for (int i = 0x20; i > 0; i -= 6) {
            i2c_write(i, pwm_half_value);
            i2c_write(0x25, reset_value);
        }
        sys_mdelay(100);
        for (int i = 0x20; i > 0; i -= 6) {
            i2c_write(i, reset_value);
        }
        sec -= 1;
    }
    return 0;
}

int controller_init() {
    //SDB low
    int i;
    //SDB high
    i2c_write(0x4B, config_value);
    for (i = 0x01; i <= 0x24; i++) {
        i2c_write(i, reset_value);
    }
    for (i = 0x26; i < 0x4A; i++) {
        i2c_write(i, config_value);
    }
    i2c_write(0x25, reset_value);
    i2c_write(0x00, config_value);
    i2c_write(0x4A, reset_value);
}

int responding() {
    blue_backgroud(0);
    int received = 0;
    int breath_value[22] = {126, 116, 106, 96, 86, 78, 69, 61, 53, 46, 39, 33, 28, 22, 18, 13, 10, 6, 4, 2, 1, 0};
    int step = 15;
    int start = 21;
    while(!received) {
        for (int i = 0x023; i > 0; i -= 3) {
            i2c_write(i, breath_value[start]);
            i2c_write(0x25, reset_value);
        }
        if (start - 1 < 0)
            start = 21;
        else
            start -= 1;
        sys_mdelay(100);
    }
}



int main(int argc, char *argv[]) {
    system("echo 235 > /sys/class/gpio/export");
    system("echo out > /sys/class/gpio/gpio235/direction");
    system("echo 1 > /sys/class/gpio/gpio235/value");
    i2c_init(SLAVE_ADDRESS, 2);
    controller_init();
    responding();

}
