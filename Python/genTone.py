import time
from tone import GstToneGenerator
from tone import ToneGeneratorWidget

tone_gen = GstToneGenerator()

tone_gen.set_values(1.0, 450)
tone_gen.start()

for x in range(5000):
	print("set tone")
	tone_gen.set_values(250, 10.0)

        
tone_gen.stop()