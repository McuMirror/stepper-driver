#include "buffers.h"
#include "usb_istr.h"

static uint8_t status_mem[STATUS_BUF_SIZE];
static uint8_t stream_mem[STREAM_BUF_SIZE];

circ_buffer_t status_buffer = {
  .addr = status_mem,
  .size = sizeof(status_mem)
};

circ_buffer_t stream_buffer = {
  .addr = stream_mem,
  .size = sizeof(status_mem)
};

uint8_t stream_flush;

uint8_t status_buffer_put(uint8_t motor, uint8_t pc, uint8_t status) {
    uint8_t tmp[] = {motor, pc, status};
    if(buf_num_free(&status_buffer) < sizeof(tmp)) return 0;
    buf_write(&status_buffer, tmp, sizeof(tmp));
    EP2_Check_Ready();
    return 1;
}
