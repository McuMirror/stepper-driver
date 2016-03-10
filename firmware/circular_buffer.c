#include "circular_buffer.h"
#include "usb_lib.h"

int16_t buf_num_free(circ_buffer_t* buf) {
  int16_t result = buf->read_ptr - buf->write_ptr - 1;
  if(result < 0) {result += buf->size;}
  return result;
}

int16_t buf_num_data(circ_buffer_t* buf) {
  int16_t result = buf->write_ptr-buf->read_ptr;
  if(result < 0) {result += buf->size;}
  return result;
}

void buf_read(circ_buffer_t* buf, void* dest, int16_t len) {
  void* buf_addr = buf->addr;
  int16_t buf_read_ptr = buf->read_ptr;
  int16_t buf_size = buf->size;

  for(; len > 0; len--) {
    *(uint8_t*)dest = *(uint8_t*)(buf_addr + buf_read_ptr);
    dest++;
    buf_read_ptr++;
    if(buf_read_ptr >= buf_size) {
        buf_read_ptr -= buf_size;
    }
  }
  buf->read_ptr = buf_read_ptr;
}

void buf_read_to_pma(circ_buffer_t* buf, uint16_t wPMABufAddr, int16_t len) {
    uint16_t *pdwVal = (uint16_t *)(wPMABufAddr + PMAAddr);
    uint16_t temp;
    void* buf_addr = buf->addr;
    int16_t buf_read_ptr = buf->read_ptr;
    int16_t buf_size = buf->size;
    int i;
    for(i = 0; i < len; i++) {
        if((i & 1) == 0) {
            ((uint8_t*)&temp)[0] = *(uint8_t*)(buf_addr + buf_read_ptr);
        } else {
            ((uint8_t*)&temp)[1] = *(uint8_t*)(buf_addr + buf_read_ptr);
            pdwVal[i / 2] = temp;
        }
        buf_read_ptr++;
        if(buf_read_ptr >= buf_size) {
            buf_read_ptr -= buf_size;
        }
    }
    if((i & 1) == 1) {
        pdwVal[i / 2] = temp;
    }
    buf->read_ptr = buf_read_ptr;
}

void buf_write(circ_buffer_t* buf, void* src, int16_t len) {
  void* buf_addr = buf->addr;
  int16_t buf_write_ptr = buf->write_ptr;
  int16_t buf_size = buf->size;

  for(; len > 0; len--) {
    *(uint8_t*)(buf_addr + buf_write_ptr) = *(uint8_t*)src;
    src++;
    buf_write_ptr++;
    if(buf_write_ptr >= buf_size) {
        buf_write_ptr -= buf_size;
    }
  }
  buf->write_ptr = buf_write_ptr;
}

void buf_empty(circ_buffer_t* buf) {
  buf->read_ptr = buf->write_ptr = 0;
}
