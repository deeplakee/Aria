# test5
import time
import random
def make_big_array():
    a = []
    for i in range(1000):
        a.append(random.random())
    return a

start = time.perf_counter()
for i in range(10_000):
    x = make_big_array()
print(time.perf_counter() - start)
