# test6
import time
import random
def make_big_object():
    a = {}
    for i in range(1000):
        a["key"+str(i)] = random.random()
    return a
  
start = time.perf_counter()
for i in range(10000):
    x = make_big_object()
print(time.perf_counter() - start)
