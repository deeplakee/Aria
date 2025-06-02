# test10
import time
class Switch:
    def __init__(self, state):
        self.state = state

    def toggle(self):
        self.state = not self.state
        return self

    def get_state(self):
        return self.state

x = Switch(True)
start = time.perf_counter()
for i in range(100000):
    x.toggle().toggle().get_state()
    x.toggle().toggle().get_state()
    x.toggle().toggle().get_state()
    x.toggle().toggle().get_state()
    x.toggle().toggle().get_state()
    x.toggle().toggle().get_state()
    x.toggle().toggle().get_state()
    x.toggle().toggle().get_state()
    x.toggle().toggle().get_state()
    x.toggle().toggle().get_state()
print(time.perf_counter() - start)