import engine
import random

engine.SetSeed(random.randint(0, 100))

engine.TestDrawLotsExisting()
engine.TestDrawLotsNew()
engine.TestUpdate()
engine.TestPlayOut()
