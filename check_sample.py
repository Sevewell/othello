import engine
from matplotlib import pyplot
import time

engine.SetSeed()

time_before = time.time()
sample = [engine.WrapSampleBeta(8, 2) for i in range(10000)]
print(sample[:10])
print(time.time() - time_before)

pyplot.hist(sample, range=(0, 1), bins=50, alpha=0.5)
pyplot.show()