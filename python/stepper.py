import asyncusb
import asyncio
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

class Stepper:
    VENDOR_ID = 0xffff
    PRODUCT_ID = 0x7a66
    INTERFACE = 0
    ENDPOINT_PROGRAM = 1
    ENDPOINT_STATUS = 2
    ENDPOINT_STREAM = 3
    CT_TIMEOUT = 100

    def __init__(self, loop = None):
        if loop is None:
            self.loop = asyncio.get_event_loop()
        else:
            self.loop = loop

        self.context = asyncusb.USBContext(loop = self.loop)

    def __enter__(self):
        self.handle = self.context.openByVendorIDAndProductID(
            self.VENDOR_ID, self.PRODUCT_ID,
            skip_on_error=True,
        )

        if self.handle is None:
            raise StepperError("Could not find Stepper")

        self.handle.claimInterface(self.INTERFACE)

    def __exit__(self, type, value, tb):
        self.handle.close()
        self.handle = None

    async def status(self):
        with self:
            transfer = await asyncusb.asyncBulkTransfer(
                self.handle, asyncusb.ENDPOINT_IN | self.ENDPOINT_STREAM,
                4 * 1024 * 1024)
            chunk = transfer.getBuffer()[:transfer.getActualLength()]
            s = len(chunk)
            data = struct.unpack("<" + "f" * (s // 4), chunk)
            data = np.array(data)
            data = np.reshape(data, (-1, 2)).T
            return (data[0], data[1])

    async def program_start(self, motor):
        with self:
            transfer = await asyncusb.asyncControlTransfer(
                self.handle, USB_CONTROL_WRITE,
                USB_PROGRAM_START,
                motor, 0, b"", timeout = self.CT_TIMEOUT)
            await self.check_error()

    async def program_instruction(self, motor, command, data = b""):
        with self:
            transfer = await asyncusb.asyncControlTransfer(
                self.handle, USB_CONTROL_WRITE,
                USB_PROGRAM_INSTRUCTION,
                motor, command, data, timeout = self.CT_TIMEOUT)
            await self.check_error()

    async def program_end(self, motor):
        with self:
            transfer = await asyncusb.asyncControlTransfer(
                self.handle, USB_CONTROL_WRITE,
                USB_PROGRAM_END,
                motor, 0, b"", timeout = self.CT_TIMEOUT)
            await self.check_error()

    async def program_load(self, motor):
        with self:
            transfer = await asyncusb.asyncControlTransfer(
                self.handle, USB_CONTROL_WRITE,
                USB_PROGRAM_LOAD,
                motor, 0, b"", timeout = self.CT_TIMEOUT)
            await self.check_error()

    async def program_immediate(self, motor, command, data = b""):
        with self:
            transfer = await asyncusb.asyncControlTransfer(
                self.handle, USB_CONTROL_WRITE,
                USB_PROGRAM_IMMEDIATE,
                motor, command, data, timeout = self.CT_TIMEOUT)
            await self.check_error()

    async def check_error(self):
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
        #await s.program_start(0)
        #await s.program_instruction(0, 2, struct.pack("fffffff", 1, 0, 0, 200, 0, 200, 200))
        #await s.program_instruction(0, 0)
        #await s.program_end(0)
        #await s.program_load(0)
        for i in range(5):
            await s.program_immediate(0, 2, struct.pack("fffffff", 1, 0, 0, 200, 0, 200, 200))
            await asyncio.sleep(0.2)
            await s.program_immediate(0, 2, struct.pack("fffffff", 1, 0, 0, -200, 0, -200, -200))
            await asyncio.sleep(0.2)
        return
        (a, b) = await s.status()
        import matplotlib.pyplot as plt
        plt.plot(range(len(a)), a, 'r-', range(len(b)), b, 'b-')
        plt.show()

    asyncio.get_event_loop().run_until_complete(run())
