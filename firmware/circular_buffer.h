#ifndef __CIRCULAR_BUFFER_H
#define __CIRCULAR_BUFFER_H

#include <stdint.h>

typedef struct {
    void* addr;
    int16_t read_ptr;
    int16_t write_ptr;
    int16_t size;
} circ_buffer_t;

int16_t buf_num_free(circ_buffer_t* buf);
int16_t buf_num_data(circ_buffer_t* buf);

void buf_read(circ_buffer_t* buf, void* dest, int16_t len);
void buf_read_to_pma(circ_buffer_t* buf, uint16_t wPMABufAddr, int16_t len);
void buf_write(circ_buffer_t* buf, void* src, int16_t len);
void buf_empty(circ_buffer_t* buf);

#endif
