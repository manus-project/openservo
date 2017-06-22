
#ifndef LIB_I2C_H
#define LIB_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

int i2cOpen(char *filename, int *file);
int i2cClose(int file);
int i2cSetSlave(int file, int i2c_dev_addr);
int i2cRead(int file, unsigned char* buffer, int length);
int i2cWrite(int file, unsigned char* buffer, int length);
int i2cScan(int file, unsigned char* addr);

#ifdef __cplusplus
}
#endif

#endif
