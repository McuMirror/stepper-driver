import stepper
import asyncio
import matplotlib.pyplot as plt
import numpy as np

async def run():
    s = stepper.Stepper()
    async with s:
        data = b""
        async def handle(chunk):
            nonlocal data
            data += chunk
        s.stream = handle

        await s.program_start(0)
        await s.program_instruction(0, s.cmd_stream(s.STREAM_CURRENT))
        await s.program_instruction(0, s.cmd_move_rel(0.5, 100, 200, 0, 0, 200))
        await s.program_instruction(0, s.cmd_move_rel(0.5, -100, -200, 0, 0, -200))
        await s.program_instruction(0, s.cmd_stream(s.STREAM_NONE))
        await s.program_instruction(0, s.cmd_halt())
        await s.program_end(0)
        await s.program_load(0)

        await s.until_finished(0)

    data = np.frombuffer(data, dtype='float32')
    data = np.reshape(data, (-1, 2)).T
    a = data[0]
    b = data[1]
    plt.plot(range(len(a)), a, 'r-', range(len(b)), b, 'b-')
    plt.show()

asyncio.get_event_loop().run_until_complete(run())
