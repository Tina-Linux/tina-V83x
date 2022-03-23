struct CXI2CINTERFACE;

typedef void (*FnCloseI2cDevice)(CXI2CINTERFACE* hI2cDevice);
typedef int (*FnI2cWrite)(CXI2CINTERFACE* hI2cDevice, unsigned char ChipAddr, unsigned long cbBuf, unsigned char* pBuf);
typedef int (*FnI2cWriteThenRead)(CXI2CINTERFACE* hI2cDevice, unsigned char ChipAddr, unsigned long cbBuf,
    unsigned char* pBuf, unsigned long cbReadBuf, unsigned char*pReadBuf);
typedef void (*FnSetI2cSpeed)(CXI2CINTERFACE* hI2cDevice,bool  b400Khz);  
typedef bool (*FnGpioSetPort)(CXI2CINTERFACE* hI2cDevice,
            unsigned char portNumber,
            unsigned char portDirection,
            unsigned char portValue);
typedef bool (*FnGpioSetPin)(CXI2CINTERFACE* hI2cDevice,
            unsigned char portNumber,
            unsigned char pinNumber,
            unsigned char pinValue);

typedef void* HANDLE;
struct CXI2CINTERFACE{
   HANDLE                   hHandle;
   FnCloseI2cDevice         CloseDevice;
   FnI2cWrite               I2cWrite;
   FnI2cWriteThenRead       I2cWriteThenRead;
   FnSetI2cSpeed            SetI2cSpeed;  
   FnGpioSetPort            GpioSetPort;
   FnGpioSetPin             GpioSetPin;
};



