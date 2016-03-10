#ifndef __BUFFERS_H
#define __BUFFERS_H

#include "circular_buffer.h"

#define STATUS_BUF_SIZE 64
#define STREAM_BUF_SIZE 256

extern circ_buffer_t status_buffer;
extern circ_buffer_t stream_buffer;
extern uint8_t stream_flush;
uint8_t status_buffer_put(uint8_t motor, uint8_t pc, uint8_t status);

#endif
