import asyncio
import sys
import traceback
import signal
import os
import functools

SIGNAMES = dict((k, v) for v, k in reversed(sorted(signal.__dict__.items()))
     if v.startswith('SIG') and not v.startswith('SIG_'))

# This method wraps a coroutine and prints out any errors that occur.
# If a parent is specified, the parent is cancelled when the child throws an error.
def task(f):
    @functools.wraps(f)
    async def wrapper(self, *args, parent = None, **kwargs):
        try:
            await f(self, *args, **kwargs)
        except asyncio.CancelledError:
            if parent is not None and not parent.done():
                my_task = asyncio.Task.current_task()
                parent.cancel()
        except Exception:
            print("Asynchronous exception:", file = sys.stderr)
            traceback.print_exc()
            if parent is not None:
                parent.cancel()
    return wrapper

# This method wraps a thread and prints out any errors that occur.
# If a parent task is specified, the parent is cancelled when the thread throws an error.
def thread(f):
    @functools.wraps(f)
    def wrapper(self, *args, parent = None, **kwargs):
        try:
            f(self, *args, **kwargs)
        except asyncio.CancelledError:
            if parent is not None:
                schedule_shutdown(parent)
        except Exception:
            print("Asynchronous exception in thread:", file = sys.stderr)
            traceback.print_exc()
            if parent is not None:
                schedule_shutdown(parent)
    return wrapper

class ChildTask(object):
    def child_start(self, task):
        my_task = asyncio.Task.current_task()
        loop = asyncio.get_event_loop()
        return loop.create_task(task(parent = my_task))

    async def child_stop(self, runner):
        if not runner.done():
            runner.cancel()
            await runner

# This method safely schedules the shutdown of a task
def schedule_shutdown(task, loop = None):
    if loop is None:
        loop = asyncio.get_event_loop()

    loop.call_soon_threadsafe(task.cancel)

# This method sets a handler for Ctrl-C that cancels the given task
def handle_ctrl_c(task = None, loop = None):
    if loop is None:
        loop = asyncio.get_event_loop()
    if task is None:
        task = asyncio.Task.current_task(loop)

    def handler(signum, frame):
        signal.signal(signal.SIGTERM, signal.SIG_DFL)
        signal.signal(signal.SIGINT, signal.SIG_DFL)
        print("\nProgram received {0}, attempting graceful shutdown".format(SIGNAMES.get(signum, signum)), file = sys.stderr)
        schedule_shutdown(task, loop = loop)

    signal.signal(signal.SIGTERM, handler)
    signal.signal(signal.SIGINT, handler)

# This method adds a timeout to a future (in the form of another future)
# If the first future completes first, its result is returned.
# If the second future completes first, the first is cancelled and asyncio.TimeoutError is thrown.
async def timeout_event(fut, timeout_fut, loop = None):
    if loop is None:
        loop = asyncio.get_event_loop()

    fut = asyncio.Task(fut)
    timeout_fut = asyncio.Task(timeout_fut)

    done, pending = await asyncio.wait((fut, timeout_fut), loop = loop, return_when = asyncio.FIRST_COMPLETED)

    if fut in pending:
        fut.cancel()
        try:
            await fut
        except asyncio.CancelledError:
            pass
        raise asyncio.TimeoutError()
    if timeout_fut in pending:
        timeout_fut.cancel()
        try:
            await timeout_fut
        except asyncio.CancelledError:
            pass
    return fut.result()

# This coroutine calls an object's __aexit__, and never throws an exception.
async def safe_aexit(obj, type, value, tb):
    try:
        await obj.__aexit__(type, value, tb)
    except Exception:
        traceback.print_exc()

# This function gathers together multiple asyncio.Lock-like objects
# into one Lock-like object. It also adds a feature to the context manager:
# If you do "with await gather_locks" then it blocks until the lock becomes available.
# If you do "with gather_locks" then it errors if the lock is not immediately available.
def gather_locks(locks, loop=None):
    if loop is None:
        loop = asyncio.get_event_loop()

    class _MultiLockCM:
        def __init__(self, ml):
            self.ml = ml

        def __enter__(self):
            pass

        def __exit__(self, type, value, tb):
            self.ml.release()

    class _MultiLock:
        def locked(self):
            return any([l.locked() for l in locks])

        # returns future
        def acquire(self):
            return asyncio.gather(*[l.acquire() for l in locks], loop=loop)

        def release(self):
            for l in locks:
                l.release()

        def __await__(self):
            yield from self.acquire()
            return _MultiLockCM(self)

        def __enter__(self):
            if self.locked():
                raise RuntimeError("Resource in use")
            else:
                nl = asyncio.new_event_loop()
                nl.run_until_complete(asyncio.gather(*[l.acquire() for l in locks], loop=nl))

        def __exit__(self, type, value, tb):
            self.release()

    return _MultiLock()
