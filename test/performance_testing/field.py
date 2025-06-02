# test4
import time
class Foo:
    def __init__(self):
        for i in range(1, 21):
            setattr(self, f'field{i}', 1)

    def __getattr__(self, name):
        if name.startswith("method"):
            index = name[6:]
            return lambda: getattr(self, f'field{index}')
        raise AttributeError(name)

    def method1(self): return self.field1
    def method2(self): return self.field2
    def method3(self): return self.field3
    def method4(self): return self.field4
    def method5(self): return self.field5
    def method6(self): return self.field6
    def method7(self): return self.field7
    def method8(self): return self.field8
    def method9(self): return self.field9
    def method10(self): return self.field10
    def method11(self): return self.field11
    def method12(self): return self.field12
    def method13(self): return self.field13
    def method14(self): return self.field14
    def method15(self): return self.field15
    def method16(self): return self.field16
    def method17(self): return self.field17
    def method18(self): return self.field18
    def method19(self): return self.field19
    def method20(self): return self.field20

x = Foo()
start = time.perf_counter()
for i in range(50000):
    x.method1(); x.method2(); x.method3(); x.method4(); x.method5()
    x.method6(); x.method7(); x.method8(); x.method9(); x.method10()
    x.method11(); x.method12(); x.method13(); x.method14(); x.method15()
    x.method16(); x.method17(); x.method18(); x.method19(); x.method20()
print(time.perf_counter() - start)
