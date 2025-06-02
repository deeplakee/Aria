# test7
import time
a = "lzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzylzy";
b = "yzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzlyzl";

start = time.perf_counter()

for i in range(100000):
    a == b; a == b[::-1]; a + b
    a == b; a == b[::-1]; a + b
    a == b; a == b[::-1]; a + b
    a == b; a == b[::-1]; a + b
    a == b; a == b[::-1]; a + b
    a == b; a == b[::-1]; a + b
    a == b; a == b[::-1]; a + b
    a == b; a == b[::-1]; a + b
    a == b; a == b[::-1]; a + b
    a == b; a == b[::-1]; a + b

print(time.perf_counter() - start)