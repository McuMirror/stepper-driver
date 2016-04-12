import stepper
import asyncio
import matplotlib.pyplot as plt
import numpy as np

MOTOR = 1

async def measure(s, speed, setp, mode, voltage = False):
    LEAD_IN = 0.2
    LEAD_OUT = 0.02
    TIME = 0.3

    data = b""
    async def handle(chunk):
        nonlocal data
        data += chunk
    s.stream = handle

    await s.program_start(MOTOR)
    await s.program_instruction(MOTOR, s.cmd_control_mode(mode, setp))
    if voltage:
        await s.program_instruction(MOTOR, s.cmd_stream(s.STREAM_VOLTAGE))
    else:
        await s.program_instruction(MOTOR, s.cmd_stream(s.STREAM_CURRENT))
    await s.program_instruction(MOTOR, s.cmd_move_rel(TIME, speed * TIME, speed, 0, 0, speed))
    await s.program_instruction(MOTOR, s.cmd_stream(s.STREAM_NONE))
    await s.program_instruction(MOTOR, s.cmd_control_mode(s.CONTROL_MODE_VOLTAGE, 0.1))
    await s.program_instruction(MOTOR, s.cmd_halt())
    await s.program_end(MOTOR)
    await s.program_load(MOTOR)

    await s.until_finished(MOTOR)

    if voltage:
        data = np.frombuffer(data, dtype='int16')
    else:
        data = np.frombuffer(data, dtype='float32')
    data = np.reshape(data, (-1, 2)).T
    discard_start = int(data.shape[1] * LEAD_IN / TIME)
    discard_end = int(data.shape[1] * LEAD_OUT / TIME)
    data = data[:,discard_start:-discard_end]
    a = data[0]
    b = data[1]
    plt.plot(range(len(a)), a, 'r-', range(len(b)), b, 'b-')
    plt.show()
    rms = np.std(np.hstack((a, b)))
    return rms * 2 ** 0.5

voltages = np.linspace(0.1, 1, 8)
speeds = np.linspace(0, 300, 8)

async def run():
    s = stepper.Stepper()
    async with s:
        pts = []
        #for v in voltages:
        #    for sp in speeds:
        #        current = await measure(s, sp, v, s.CONTROL_MODE_VOLTAGE) 
        #        pts.append((v, sp, current))

        print(pts)

        v1 = 0.3
        v2 = 0.8
        sp1 = 75
        sp2 = 150
        c11 = await measure(s, sp1, v1, s.CONTROL_MODE_VOLTAGE)
        c12 = await measure(s, sp1, v2, s.CONTROL_MODE_VOLTAGE)
        c21 = await measure(s, sp2, v1, s.CONTROL_MODE_VOLTAGE)
        c22 = await measure(s, sp2, v2, s.CONTROL_MODE_VOLTAGE)

        r1 = (v2 - v1) / (c12 - c11)
        r2 = (v2 - v1) / (c22 - c21)
        r = (r1 + r2) / 2
        print(r1, r2)
        km1 = -r * (c21 - c11) / (sp2 - sp1)
        km2 = -r * (c22 - c12) / (sp2 - sp1)
        km = (km1 + km2) / 2
        print(km1, km2)

        by_speed = {}

        #by_speed[sp1] = [(v1, c11), (v2, c12)]
        #by_speed[sp2] = [(v1, c21), (v2, c22)]

        for (v, sp, c) in pts:
            by_speed[sp] = by_speed.get(sp, []) + [(v, c)]

        for (sp, pts) in by_speed.items():
            pts = zip(*pts)
            plt.plot(*pts)

        for sp in speeds:
            v1 = voltages[0]
            v2 = voltages[-1]
            c1 = (v1 - km * sp) / r
            c2 = (v2 - km * sp) / r
            plt.plot([v1, v2], [c1, c2])
            pts = zip(*pts)
            plt.plot(*pts)

        plt.show()

        await s.program_immediate(MOTOR, s.cmd_tuning(km, r))

        pts = []
        currents = np.linspace(0.2, 1, 2)
        for i in currents:
            for sp in speeds:
                current = await measure(s, sp, i, s.CONTROL_MODE_CURRENT_OL, voltage=False)
                pts.append((i, sp, current))

        by_current = {}

        for (i, sp, c) in pts:
            by_current[i] = by_current.get(i, []) + [(sp, c)]

        for (sp, pts) in by_current.items():
            pts = zip(*pts)
            plt.plot(*pts)

        plt.show()


asyncio.get_event_loop().run_until_complete(run())
