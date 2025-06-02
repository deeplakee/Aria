# test2
import time
class Foo:
    def __init__(self, x):
        self.val = x

start = time.perf_counter()
for i in range(100000):
    Foo(i); Foo(i); Foo(i); Foo(i); Foo(i)
    Foo(i); Foo(i); Foo(i); Foo(i); Foo(i)
print(time.perf_counter() - start)