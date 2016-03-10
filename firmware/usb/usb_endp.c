#include "usb_lib.h"
#include "buffers.h"
#include "circular_buffer.h"
#include "usb_desc.h"
#include "usb_prop.h"

void EP1_OUT_Callback(void) {
}

void EP1_Check_Ready() {
}

void EP2_IN_Callback(void) {
    uint16_t len = buf_num_data(&status_buffer);
    if(len > 0) {
        if(len == BULK_PACKET_LEN) ep2_needs_zlp = 1;
        if(len > BULK_PACKET_LEN) len = BULK_PACKET_LEN;
        buf_read_to_pma(&status_buffer, ENDP2_TXADDR, len);
        _SetEPTxCount(ENDP2, len);
        _SetEPTxValid(ENDP2);
    } else if(ep2_needs_zlp) {
        _SetEPTxCount(ENDP2, len);
        _SetEPTxValid(ENDP2);
        ep2_needs_zlp = 0;
    } else {
        ep2_needs_data = 1;
    }
}

void EP2_Check_Ready() {
    if(ep2_needs_data && buf_num_data(&status_buffer) > 0) {
        ep2_needs_data = 0;
        EP2_IN_Callback();
    }
}

void EP3_IN_Callback(void) {
    uint16_t len = buf_num_data(&stream_buffer);
    if(len >= BULK_PACKET_LEN || (len > 0 && stream_flush)) {
        if(len == BULK_PACKET_LEN) ep3_needs_zlp = 1;
        if(len > BULK_PACKET_LEN) len = BULK_PACKET_LEN;
        stream_flush = 0;
        buf_read_to_pma(&stream_buffer, ENDP3_TXADDR, len);
        _SetEPTxCount(ENDP3, len);
        _SetEPTxValid(ENDP3);
    } else if(!len && ep3_needs_zlp) {
        _SetEPTxCount(ENDP3, len);
        _SetEPTxValid(ENDP3);
        ep3_needs_zlp = 0;
    }
}

void EP3_Check_Ready() {
}

