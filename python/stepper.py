import asyncusb
import asyncio

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
                self.handle, asyncusb.ENDPOINT_IN | self.ENDPOINT_STATUS,
                64)
            chunk = transfer.getBuffer()[:transfer.getActualLength()]
            print(chunk)

if __name__ == "__main__":
    async def run():
        s = Stepper()
        await s.status()

    asyncio.get_event_loop().run_until_complete(run())
