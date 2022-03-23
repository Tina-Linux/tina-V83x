#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>

#include <iostream>
#include <atomic>

#include "i2c/I2CAdapter.h"

namespace AW {

std::shared_ptr<I2CHandler> I2CHandler::create(int index)
{
    return std::shared_ptr<I2CHandler>(new I2CHandler(index));
}

I2CHandler::~I2CHandler()
{
    release();
}

int I2CHandler::init()
{
    std::string dev = std::string("/dev/i2c-") + std::to_string(m_index);
    m_handler = open(dev.data(), O_RDWR);
    if(m_handler < 0) {
        printf("I2c device [%s] is not present(%s).\n",dev.data(), strerror(errno));
        return m_handler;
    }
    return 0;
}

void I2CHandler::release()
{
    close(m_handler);
}

int I2CHandler::transfer(unsigned long packets)
{
    int ret = ioctl(m_handler,I2C_RDWR, packets);
    if (ret < 0) {
        printf(" Failed: I2C_RDWR error = %d (%s)\n", ret, strerror(errno));
        return -1;
    }
    return 0;
}

std::shared_ptr<I2CAdapter> I2CAdapter::create(std::shared_ptr<I2CHandler> handler, unsigned char slave_address)
{
    return std::shared_ptr<I2CAdapter>(new I2CAdapter(handler, slave_address));
}

int I2CAdapter::write_a8_d8(unsigned char reg_addr, unsigned char val)
{
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg msg[1];
    unsigned char led_i2c_buf[2];
    unsigned int write_len = 2; //Addr + Data 2 bytes
    int ret = 0;
    int i;

    led_i2c_buf[0] = reg_addr;
    led_i2c_buf[1] = val;

    msg[0].addr = m_slave_address;
    msg[0].flags = 0; //7 bits
    msg[0].len = write_len;
    msg[0].buf = led_i2c_buf;

    packets.msgs = msg;
    packets.nmsgs = 1;
/*
    printf("I2C: <START> %02X", m_slave_address<<1);
    for(i=0;i<msg[0].len ;i++)
    {
        printf(" %02X",(0xff& led_i2c_buf[i]) );
    }
    printf(" <STOP>\n");
*/
    ret = m_handler->transfer((unsigned long)&packets);

    if (ret < 0) {
        printf(" Failed: i2c_write error = %d (%s)\n", ret, strerror(errno));
        return -1;
    }
    return 0;
}

}
