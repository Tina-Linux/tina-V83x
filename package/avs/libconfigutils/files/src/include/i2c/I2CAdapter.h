#ifndef __I2C_ADAPTER_H__
#define __I2C_ADAPTER_H__

#include <memory>
#include <string>
#include <stdio.h>

namespace AW {

class I2CHandler
{
public:
    static std::shared_ptr<I2CHandler> create(int index);
    ~I2CHandler();

    int init();
    void release();

    int transfer(unsigned long packets);

private:
    I2CHandler(int index) : m_index{index}{};

private:
    int m_handler = -1;
    int m_index;
};

class I2CAdapter {
public:
    static std::shared_ptr<I2CAdapter> create(std::shared_ptr<I2CHandler> handler, unsigned char slave_address);
    int read_a8_d8(unsigned char reg_addr, unsigned char *val){};
    int write_a8_d8(unsigned char reg_addr, unsigned char val);

    int read_a8_d16(unsigned char reg_addr, unsigned short *val){};
    int write_a8_d16(unsigned char reg_addr, unsigned short val){};

    int read_a16_d8(unsigned short reg_addr, unsigned char *val){};
    int write_a18_d8(unsigned short reg_addr, unsigned char val){};

    int read_a16_d16(unsigned short reg_addr, unsigned short *val){};
    int write_a16_d16(unsigned short reg_addr, unsigned short val){};

    ~I2CAdapter() = default;

private:
    I2CAdapter(std::shared_ptr<I2CHandler> handler, unsigned char slave_address) :
            m_handler{handler},m_slave_address{slave_address}{};



private:
    std::shared_ptr<I2CHandler> m_handler;
    unsigned char m_slave_address;

};/*class I2CAdapter*/

} /*namespace AW*/

#endif /*__I2C_ADAPTER_H__*/
