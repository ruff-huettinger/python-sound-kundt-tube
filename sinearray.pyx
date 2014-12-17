import numpy as np
import math, struct

prev_frequency = 440.0
prev_volume = 0
freq_div = 1000.0 * 1.12 * math.pi
data = np.empty(4096, dtype=np.int16)

def sinearray2(count, frequency=440, volume=20000, start=0):
    global prev_frequency, prev_volume, freq_div, data
    
    # if frequency has changed interpolate to new frequency
    if frequency != prev_frequency or volume != prev_volume:
        d = 1.0
        di = 1.0 / count
        # sine-wave loop
        for i in range(0,count):
            phase_old = math.sin( (i+start) / (freq_div / prev_frequency) ) * prev_volume * d
            phase_new = math.sin( (i+start) / (freq_div / frequency) ) * volume * (1.0 - d)
            data[i] = int((phase_new + phase_old))
            d -= di
    else:
        # sine-wave loop
        for i in range(0,count):
            phase = math.sin( (i+start) / (freq_div / frequency) )
            data[i] = int(phase * volume)
    
    # remember state
    prev_frequency = frequency
    prev_volume = volume
    
    return (data, start+count)
