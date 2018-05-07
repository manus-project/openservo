
#ifndef __I2C_H
#define __I2C_H

#define I2C_DIRECT 0
#define I2C_MPSSE 1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct i2c_object {
	void* data;
	int selected;
    int flags;
    int last_error;
} i2c_object;

typedef i2c_object* i2c_handle;

i2c_handle i2c_open(const char *filename, int type);
int i2c_close(i2c_handle* handle);
int i2c_select(i2c_handle handle, int address);
int i2c_read(i2c_handle handle, unsigned char* buffer, int length);
int i2c_write(i2c_handle handle, unsigned char* buffer, int length);
int i2c_scan(i2c_handle handle, unsigned char* addr);
int i2c_get_error(i2c_handle handle);

#ifdef __cplusplus
}
#endif

#endif
