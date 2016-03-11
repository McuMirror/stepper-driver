import asyncusb
import asyncio
import asyncutil
import struct

class StepperError(Exception): pass

USB_CONTROL_READ = 0xC0
USB_CONTROL_WRITE = 0x40

USB_PROGRAM_START = 0x20
USB_PROGRAM_INSTRUCTION = 0x21
USB_PROGRAM_END = 0x22
USB_PROGRAM_LOAD = 0x23
USB_PROGRAM_IMMEDIATE = 0x24
USB_GET_ERROR = 0x25
USB_GET_N_COMMANDS = 0x26
USB_GET_COMMAND_NAME = 0x27
USB_GET_COMMAND_DATA_DESCRIPTOR = 0x28

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
    MAX_DESCRIPTOR_SIZE = 4096
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

        await self.load()

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

    async def load(self):
        transfer = await asyncusb.asyncControlTransfer(
            self.handle, USB_CONTROL_READ,
            USB_GET_N_COMMANDS,
            0, 0, 2, timeout = self.CT_TIMEOUT)
        data = transfer.getBuffer()[:transfer.getActualLength()]
        (n,) = struct.unpack("<H", data)
        command_list = {}
        for i in range(n):
            transfer = await asyncusb.asyncControlTransfer(
                self.handle, USB_CONTROL_READ,
                USB_GET_COMMAND_NAME,
                i, 0, self.MAX_DESCRIPTOR_SIZE, timeout = self.CT_TIMEOUT)
            data = transfer.getBuffer()[:transfer.getActualLength()]
            if data[-1] == 0: data = data[:-1]
            name = str(data, encoding="latin-1")
            transfer = await asyncusb.asyncControlTransfer(
                self.handle, USB_CONTROL_READ,
                USB_GET_COMMAND_DATA_DESCRIPTOR,
                i, 0, self.MAX_DESCRIPTOR_SIZE, timeout = self.CT_TIMEOUT)
            data = transfer.getBuffer()[:transfer.getActualLength()]
            if data[-1] == 0: data = data[:-1]
            descriptor = str(data, encoding="latin-1")
            self.make_fn(i, name, descriptor)
            command_list["CMD_" + name.upper()] = i
        self.COMMAND_LIST = command_list
        self.__dict__.update(command_list)

    def make_fn(self, num, name, descriptor):
        args = [a.strip() for a in descriptor.split(";")]
        assert args[-1] == ""
        args = args[:-1]

        parsed_args = []
        sw_internal = False

        for arg in args:
            (kind, _, arg) = arg.partition(" ")
            arg = arg.strip()
            if kind == "arg":
                assert not sw_internal, "Args must come first"
                pass
            elif kind == "internal":
                sw_internal = True
                pass
            else:
                assert False, "Expected 'arg' or 'internal'"
            (dtype, _, arg) = arg.partition(" ")
            arg = arg.strip()
            struct_letter = {
                "float": "f",
                "enum8": "B"
            }[dtype]
            if dtype == "enum8":
                (enum_values, _, arg) = arg.partition('}')
                arg = arg.strip()
                enum_values = enum_values.strip()
                assert enum_values[0] == "{", "enum type needs value list"
                enum_values = [v.strip() for v in enum_values[1:].split(",")]
                enum_values = dict(zip(enum_values, range(len(enum_values))))
                self.__dict__.update(enum_values)
            else:
                enum_values = None
            p_name = arg
            parsed_args.append((sw_internal, struct_letter, enum_values, p_name))

        def mk(*args):
            parsed = [(l, v, n) for (i, l, v, n) in parsed_args if not i]
            if len(args) > len(parsed):
                raise TypeError("Too many arguments: expected {}, got {}".format(len(parsed), len(args)))
            struct_str = ""
            for (i, arg, parsed) in zip(range(1, 1 + len(args)), args, parsed):
                (l, v, n) = parsed
                struct_str += l
                if v is not None and arg not in v.values():
                    raise TypeError("Argument {} must be one of [{}]".format(i, v.keys()))
            b = struct.pack(struct_str, *args)
            return (num, b)

        self.__dict__["cmd_" + name] = mk

    @asyncutil.task
    async def __run2(self):
        try:
            while True:
                transfer = await asyncusb.asyncBulkTransfer(
                    self.handle, asyncusb.ENDPOINT_IN | self.ENDPOINT_STREAM,
                    self.STREAM_CHUNK_SIZE)
                chunk = transfer.getBuffer()[:transfer.getActualLength()]

                await self.stream(chunk)
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

    async def program_instruction(self, motor, instruction):
        (command, data) = instruction
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

    async def program_immediate(self, motor, instruction):
        (command, data) = instruction
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

    async def until_finished(self, motor):
        f = self.until_status(motor, CMD_FINISHED)
        # TODO: query motor, if already finished, cancel F and return
        await f


