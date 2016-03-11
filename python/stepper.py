import asyncusb
import asyncio
import asyncutil
import struct
import numpy as np

class StepperError(Exception): pass

USB_CONTROL_READ = 0xC0
USB_CONTROL_WRITE = 0x40

USB_PROGRAM_START = 0x20
USB_PROGRAM_INSTRUCTION = 0x21
USB_PROGRAM_END = 0x22
USB_PROGRAM_LOAD = 0x23
USB_PROGRAM_IMMEDIATE = 0x24
USB_GET_ERROR = 0x25

CMD_START = 0
CMD_FINISHED = 1
CMD_ERROR = 2

class Stepper(asyncutil.ChildTask):
    VENDOR_ID = 0xffff
    PRODUCT_ID = 0x7a66
    INTERFACE = 0
    ENDPOINT_PROGRAM = 1
    ENDPOINT_STATUS = 2
    ENDPOINT_STREAM = 3
    STATUS_CHUNK_SIZE = 4096
    STREAM_CHUNK_SIZE = 4096
    CT_TIMEOUT = 100

    def __init__(self, loop = None):
        if loop is None:
            self.loop = asyncio.get_event_loop()
        else:
            self.loop = loop

        self.context = asyncusb.USBContext(loop = self.loop)

    async def __aenter__(self):
        self.handle = self.context.openByVendorIDAndProductID(
            self.VENDOR_ID, self.PRODUCT_ID,
            skip_on_error=True,
        )

        if self.handle is None:
            raise StepperError("Could not find Stepper")

        self.handle.claimInterface(self.INTERFACE)

        self.status_futures = []
        self.__runner1 = self.child_start(self.__run1)
        self.__runner2 = self.child_start(self.__run2)

    async def __aexit__(self, type, value, tb):
        try:
            try:
                await self.child_stop(self.__runner1)
            finally:
                await self.child_stop(self.__runner2)
        finally:
            for (m, s, f) in self.status_futures:
                f.cancel()
            self.status_futures = []
            self.handle.close()
            self.handle = None

    @asyncutil.task
    async def __run2(self):
        try:
            while True:
                transfer = await asyncusb.asyncBulkTransfer(
                    self.handle, asyncusb.ENDPOINT_IN | self.ENDPOINT_STREAM,
                    self.STREAM_CHUNK_SIZE)
                chunk = transfer.getBuffer()[:transfer.getActualLength()]

                print(len(chunk))
                #data = struct.unpack("<" + "f" * (s // 4), chunk)
                #data = np.array(data)
                #data = np.reshape(data, (-1, 2)).T
                #return (data[0], data[1])
        except asyncusb.USBTransferError as e:
            if e.value != 3: # TRANSFER_CANCELLED
                raise
        except asyncio.CancelledError:
            pass

    def until_status(self, motor, status):
        f = asyncio.Future()
        self.status_futures.append((motor, status, f))
        return f

    @asyncutil.task
    async def __run1(self):
        try:
            while True:
                transfer = await asyncusb.asyncBulkTransfer(
                    self.handle, asyncusb.ENDPOINT_IN | self.ENDPOINT_STATUS,
                    self.STATUS_CHUNK_SIZE)
                data = transfer.getBuffer()[:transfer.getActualLength()]
                while len(data):
                    (motor, pc, status) = struct.unpack("<BBB", data[0:3])
                    data = data[3:]
                    #print(motor, pc, status)
                    self.status_futures = [(m, s, f) for (m, s, f) in self.status_futures if not f.cancelled()]
                    status_resolved = [f for (m, s, f) in self.status_futures if m == motor and (status == s or status == CMD_ERROR)]
                    for f in status_resolved:
                        if status == CMD_ERROR:
                            f.set_exception(StepperError("Error in program PC={}".format(pc)))
                        else:
                            f.set_result(pc)
                    self.status_futures = [(m, s, f) for (m, s, f) in self.status_futures if f not in status_resolved]
        except asyncusb.USBTransferError as e:
            if e.value != 3: # TRANSFER_CANCELLED
                raise
        except asyncio.CancelledError:
            pass

    async def program_start(self, motor):
        transfer = await asyncusb.asyncControlTransfer(
            self.handle, USB_CONTROL_WRITE,
            USB_PROGRAM_START,
            motor, 0, b"", timeout = self.CT_TIMEOUT)
        await self.program_error()

    async def program_instruction(self, motor, command, data = b""):
        transfer = await asyncusb.asyncControlTransfer(
            self.handle, USB_CONTROL_WRITE,
            USB_PROGRAM_INSTRUCTION,
            motor, command, data, timeout = self.CT_TIMEOUT)
        await self.program_error()

    async def program_end(self, motor):
        transfer = await asyncusb.asyncControlTransfer(
            self.handle, USB_CONTROL_WRITE,
            USB_PROGRAM_END,
            motor, 0, b"", timeout = self.CT_TIMEOUT)
        await self.program_error()

    async def program_load(self, motor):
        transfer = await asyncusb.asyncControlTransfer(
            self.handle, USB_CONTROL_WRITE,
            USB_PROGRAM_LOAD,
            motor, 0, b"", timeout = self.CT_TIMEOUT)
        await self.program_error()

    async def program_immediate(self, motor, command, data = b""):
        transfer = await asyncusb.asyncControlTransfer(
            self.handle, USB_CONTROL_WRITE,
            USB_PROGRAM_IMMEDIATE,
            motor, command, data, timeout = self.CT_TIMEOUT)
        await self.program_error()

    async def program_error(self):
        transfer = await asyncusb.asyncControlTransfer(
            self.handle, USB_CONTROL_READ,
            USB_GET_ERROR,
            0, 0, 1, timeout = self.CT_TIMEOUT)
        result = transfer.getBuffer()[:transfer.getActualLength()]
        if len(result) != 1: raise StepperError("Short read")
        if ord(result) != 0: raise StepperError("Error code: {}".format(ord(result)))

if __name__ == "__main__":
    async def run():
        s = Stepper()
        async with s:
        #await s.program_start(0)
        #await s.program_instruction(0, 2, struct.pack("fffffff", 1, 0, 0, 200, 0, 200, 200))
        #await s.program_instruction(0, 0)
        #await s.program_end(0)
        #await s.program_load(0)
            await s.program_start(0)
            await s.program_instruction(0, 5, struct.pack("B", 1))
            await s.program_instruction(0, 2, struct.pack("fffffff", 0.5, 0, 0, 200, 0, 100, 200))
            await s.program_instruction(0, 2, struct.pack("fffffff", 0.5, 0, 0, -200, 0, -100, -200))
            await s.program_instruction(0, 5, struct.pack("B", 0))
            await s.program_instruction(0, 0)
            await s.program_end(0)
            await s.program_load(0)

            await s.until_status(0, CMD_FINISHED)
        #(a, b) = await s.status()
        #import matplotlib.pyplot as plt
        #plt.plot(range(len(a)), a, 'r-', range(len(b)), b, 'b-')
        #plt.show()

    asyncio.get_event_loop().run_until_complete(run())
