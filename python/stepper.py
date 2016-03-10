import asyncusb
import asyncio
import struct
import numpy as np

class StepperError(Exception): pass

class Stepper:
    VENDOR_ID = 0xffff
    PRODUCT_ID = 0x7a66
    INTERFACE = 0
    ENDPOINT_PROGRAM = 1
    ENDPOINT_STATUS = 2
    ENDPOINT_STREAM = 3

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

if __name__ == "__main__":
    async def run():
        s = Stepper()
        (a, b) = await s.status()
        import matplotlib.pyplot as plt
        plt.plot(range(len(a)), a, 'r-', range(len(b)), b, 'b-')
        plt.show()

    asyncio.get_event_loop().run_until_complete(run())
