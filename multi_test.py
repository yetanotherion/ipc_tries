from ctypes import c_float
from ctypes import CDLL
import os


libsimtime = CDLL('./libsimtime.so')
libsimtime.get_time.restype = c_float


def set_shift(st, f):
    return libsimtime.set_shift(st, c_float(f))

st = libsimtime.init_sim_time("multi_test.py")
set_shift(st, 3.0)
libsimtime.set_idle(st)
assert(libsimtime.get_time(st) == 3.0)
set_shift(st, 9.0)
set_shift(st, 4.0)
libsimtime.set_idle(st)
assert(libsimtime.get_time(st) == 7.0)
libsimtime.set_idle(st)
assert(libsimtime.get_time(st) == 12.0)
print "%d finished\n" % (os.getpid(),)
libsimtime.free_sim_time(st)
