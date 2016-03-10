from usb1 import *
import asyncio
import select
import functools
import ctypes

class USBTransferError(USBError):
    TRANSFER_STATUS_STRINGS = {
        TRANSFER_COMPLETED: 'TRANSFER_COMPLETED',
        TRANSFER_ERROR:     'TRANSFER_ERROR',
        TRANSFER_TIMED_OUT: 'TRANSFER_TIMED_OUT',
        TRANSFER_CANCELLED: 'TRANSFER_CANCELLED',
        TRANSFER_STALL:     'TRANSFER_STALL',
        TRANSFER_NO_DEVICE: 'TRANSFER_NO_DEVICE',
        TRANSFER_OVERFLOW:  'TRANSFER_OVERFLOW'
    }

    def __str__(self):
        return "{0} [{1}]".format(self.TRANSFER_STATUS_STRINGS.get(self.value, "Unknown error"), self.value)

# returns future
def asyncTransfer(transfer):
    fut = asyncio.Future()
    def callback(t):
        try:
            result = t.getStatus()
            if result != TRANSFER_COMPLETED:
                raise USBTransferError(result)
            fut.set_result(t)
        except Exception as e:
            fut.set_exception(e)
    transfer.setCallback(callback)
    result = transfer.submit()
    return fut

# returns future
def asyncControlTransfer(handle, *args, **kwargs):
    ct = handle.getTransfer()
    ct.setControl(*args, **kwargs)
    return asyncTransfer(ct)

# returns future
def asyncBulkTransfer(handle, *args, **kwargs):
    ct = handle.getTransfer()
    ct.setBulk(*args, **kwargs)
    return asyncTransfer(ct)

# Monkey-patch USBContext
usb1_USBContext = USBContext
class USBContext(usb1_USBContext):
    def _handleEvents(self):
        self.handleEventsTimeout()

    def _pollFDAddedCallback(self, fd, events, userData = None):
        if events & select.EPOLLIN:
            self.loop.add_reader(fd, self._handleEvents)
        if events & select.EPOLLOUT:
            self.loop.add_writer(fd, self._handleEvents)
        if events & ~(select.EPOLLIN | select.EPOLLOUT):
            raise Exception("Unknown events bitmask: {0}".format(events))

    def _pollFDRemovedCallback(self, fd, userData = None):
        self.loop.remove_reader(fd)
        self.loop.remove_writer(fd)

    def __init__(self, *args, loop = None, **kwargs):
        if loop is None:
            self.loop = asyncio.get_event_loop()
        else:
            self.loop = loop

        super(USBContext, self).__init__(*args, **kwargs)

        for fd, events in self.getPollFDList():
            self._pollFDAddedCallback(fd, events)
        self.setPollFDNotifiers(self._pollFDAddedCallback,
                                self._pollFDRemovedCallback)

    def __del__(self):
        #try:
        #    self.setPollFDNotifiers(None, None)
        #except ctypes.ArgumentError:
        #    pass 
        super(USBContext, self).__del__()

if __name__ == '__main__':
    import sys

    VENDOR_ID = 0x16d0
    PRODUCT_ID = 0x0a16
    INTERFACE = 0

    loop = asyncio.get_event_loop()

    context = USBContext(loop = loop)
    handle = context.openByVendorIDAndProductID(
        VENDOR_ID, PRODUCT_ID,
        skip_on_error=True,
    )

    if handle is None:
        print("Device not found.")
        sys.exit(1)

    handle.claimInterface(INTERFACE)

    result = loop.run_until_complete(asyncControlTransfer(handle, 0xC0, 0x28, 0, 0, 64, timeout = 1))
    print(result)
