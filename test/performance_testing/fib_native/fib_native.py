import ctypes
import time

lib = ctypes.CDLL('./libfib++.so')
lib.fib.argtypes = [ctypes.c_int64]
lib.fib.restype = ctypes.c_int64

start = time.perf_counter()
print(lib.fib(35))
print(time.perf_counter() - start)
