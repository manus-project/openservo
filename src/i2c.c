#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "i2c.h"
#include "debug.h"

#ifdef _BUILD_MPSSE

#include "mpsse.h"

#endif

i2c_handle i2c_open(const char *filename, int type) {

#ifdef _BUILD_MPSSE
  if (type == I2C_MPSSE) {
    mpsse_handle mpsse = MPSSE(I2C, ONE_HUNDRED_KHZ, MSB);

    if (!mpsse) return NULL;

    DEBUGMSG("Opened MPSSE context \n");

    i2c_handle handle = (i2c_handle) malloc(sizeof(i2c_object));
    handle->flags = type;
    handle->selected = 0;
    handle->data = mpsse;
    return handle;
  }
#endif
  if (type == I2C_DIRECT) {
    int file = -1;

    if ((file = open(filename, O_RDWR)) < 0) {
      return NULL;
    }

    DEBUGMSG("Opened I2C direct context \n");

    i2c_handle handle = (i2c_handle) malloc(sizeof(i2c_object));
    handle->flags = type;
    handle->selected = 0;
    handle->data = malloc(sizeof(int));
    ((int *) (handle->data))[0] = file;
    return handle;

  }

  return NULL;
}

int i2c_close(i2c_handle* handle) {
  if (!(*handle))
    return -1;

#ifdef _BUILD_MPSSE
  if ((*handle)->flags == I2C_MPSSE) {
    
    Close((mpsse_handle) (*handle)->data);

    free((*handle)); (*handle) = NULL;
    return 0;
  }
#endif
  if ((*handle)->flags == I2C_DIRECT) {
    int file = -1;

    if (close(*((int*)(*handle)->data))) {
      
    }

    free((*handle)->data);

    free((*handle)); (*handle) = NULL;
    return 0;
  }


  return -1;
}

//int addr = 0x5a; // I2C address of the slave
int i2c_select(i2c_handle handle, int address) {

  if (!handle)
    return -1;

#ifdef _BUILD_MPSSE
  if ((handle)->flags == I2C_MPSSE) {
    
    handle->selected = address;

    return 0;
  }
#endif
  if ((handle)->flags == I2C_DIRECT) {
    if (ioctl(*((int*)(handle)->data), I2C_SLAVE, address) < 0) {
      // ERROR HANDLING: you can chech error no. to see what went wrong
      return -1;
    }
    return 0;
  }
  return -1;
}

int i2c_read(i2c_handle handle, unsigned char* buffer, int length) {

  char* data;

  if (!handle)
    return -1;

#ifdef _BUILD_MPSSE
  if ((handle)->flags == I2C_MPSSE) {
    
    char address = ((handle)->selected << 1) + 1;

    Start((mpsse_handle) (handle)->data);
    Write((mpsse_handle) (handle)->data, &address, 1);

    if (GetAck((mpsse_handle) (handle)->data) != ACK) return -2;

    SendAcks((mpsse_handle) (handle)->data);

    data = Read((mpsse_handle) (handle)->data, length);

    if (data == NULL) return -1;

    memcpy(buffer, data, length);

    free(data);

    SendNacks((mpsse_handle) (handle)->data);

    Read((mpsse_handle) (handle)->data, 1);

    Stop((mpsse_handle) (handle)->data);

    return 0;
  }
#endif
  if ((handle)->flags == I2C_DIRECT) {
    // read() returns the number of bytes actually read, if 
    // it doesn't match, then an error occurred (e.g. no 
    // response from the device)
    if (read(*((int*)(handle)->data), buffer, length) != length) 
    {
      // ERROR HANDLING: i2c transaction failed
      return -1;
    }
    return 0;
  }
  return -1;
}

int i2c_write(i2c_handle handle, unsigned char* buffer, int length) {

  if (!handle)
    return -1;

#ifdef _BUILD_MPSSE
  if ((handle)->flags == I2C_MPSSE) {
    
    char address = ((handle)->selected << 1);

    Start((mpsse_handle) (handle)->data);
    Write((mpsse_handle) (handle)->data, &address, 1);

    if (GetAck((mpsse_handle) (handle)->data) != ACK) return -2;

    Write((mpsse_handle) (handle)->data, buffer, length);

    if (GetAck((mpsse_handle) (handle)->data) != ACK) return -3;

    Stop((mpsse_handle) (handle)->data);

    return 0;
  }
#endif
  if ((handle)->flags == I2C_DIRECT) {
    if (write(*((int*)(handle)->data), buffer, length) != length)
    {
      // ERROR HANDLING: i2c transaction failed
      return -4;
    }
    return 0;
  }
  return -5;
}

//---- SCAN ADDRESSES ----
// scans from 8 to 119
int i2c_scan(i2c_handle handle, unsigned char* addr) {

  if (!handle)
    return -1;

  int count = 0;
  unsigned char buff[1];
  int q;
  for (q = 8; q < 120; q++)
  {
    i2c_select(handle, q);
    if (i2c_read(handle, buff, 1) == 0)
    {
      addr[count++] = q;
    }
  }
  return count;
}
