# test1
import time
start = time.perf_counter()
x = 1
for i in range(1, 1000000):
    x = x + 1
print(time.perf_counter() - start)