# test3
import time

class Foo:
    pass

start = time.perf_counter()
for i in range(100000):
    Foo(); Foo(); Foo(); Foo(); Foo()
    Foo(); Foo(); Foo(); Foo(); Foo()
print(time.perf_counter() - start)