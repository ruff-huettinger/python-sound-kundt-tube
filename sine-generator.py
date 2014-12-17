#!/usr/bin/env python

# import required libraries
import pyximport; pyximport.install()
import pyaudio, sys, time, math, struct, serial, traceback, sinearray


# ------------------------------------------------------------------------------
# reset at this seconds of activity and inactivity
activity_reset_at = 120
inactivity_reset_at = 5
max_volume = 0.5
lock_while_reset = True
arduino_flush_serial = False

# reset sequence
reset = [ \
    { "frequency": 230, "time": 4, "volume": 1.0 }, \
    { "frequency": 345, "time": 4, "volume": 1.0 }, \
    { "frequency": 545, "time": 4, "volume": 1.0 }, \
    { "frequency": 10,  "time": 1, "volume": 0.0 } ]


# ------------------------------------------------------------------------------
# global variables (dont touch)
frequency = 30.0
volume = 0
button = 0
count = 0

activity_time = 0
inactive_time = 0
old_time = time.time()
reset_state = -1


# minmax function
def minmax(val, min, max):
    if val < min: return min
    if val > max: return max
    return val

# sound callback
def callback(in_data, frame_count, time_info, status):
    global frequency, volume, count
    data, count = sinearray.sinearray2(frame_count, float(frequency), volume, count)
    return (data, pyaudio.paContinue)

# redirect stdout
sys.stdout = open("log.txt", "w")

# wait a few secs
time.sleep(2.0)

# create pyaudio object
p = pyaudio.PyAudio()

# open output stream
stream = p.open(format=pyaudio.paInt16, channels=1, rate=22050,
                output=True, stream_callback=callback, frames_per_buffer=4096)

# open serial port
arduino = serial.Serial(port="/dev/ttymxc3", baudrate=9600, timeout=0)

# start stream
stream.start_stream()
try:
    while stream.is_active():

        if reset_state == -1 or lock_while_reset == False:
            
            # read arduino input (serial) (nonblocking if timeout=0; see above)
            try:
                if arduino.inWaiting() > 1:
                    #time.sleep(0.005)
                    frequency_in, volume_in, button_in = arduino.readline().split(',')
                    button = int(button_in)
                    if button == 0: volume_in = 0.0
                    frequency = minmax(float(frequency_in), 10.0, 999.0)
                    volume = float(volume_in) / 100.0 * (32767.0 * max_volume)
                    
                    # flush and ignore serial from arduino
                    if arduino_flush_serial == True:
                        while arduino.inWaiting() > 1:
                            arduino.readline()
            except:
                pass
            
            # increase activity_time when active
            if button == 1:
                if time.time() - old_time >= 1.0:
                    activity_time += 1
                    inactive_time = 0
                    reset_state = -1
                    old_time = time.time()
        else:
            # flush and ignore serial from arduino
            if arduino.inWaiting() > 1:
                arduino.readline()
            
        # increate inactive_time when inactive
        if button == 0 or reset_state != -1:
            if time.time() - old_time >= 1.0:
                inactive_time += 1
                old_time = time.time()
        
        # start reset sequence if conditions are met
        if activity_time >= activity_reset_at and inactive_time >= inactivity_reset_at and reset_state == -1:
            if lock_while_reset: arduino.write('1')
            inactive_time = 0
            activity_time = 0
            reset_state = 0
        
        
        # run reset sequence
        if reset_state != -1:
            # use frequency and volume from current reset state
            frequency = reset[reset_state]["frequency"]
            volume = reset[reset_state]["volume"] * 32767.0
            
            # change to next reset_state if ...
            if inactive_time >= reset[reset_state]["time"]:
                inactive_time = 0
                reset_state += 1
                if reset_state >= len(reset):
                    # reset sequence finished here!
                    reset_state = -1
                    activity_time = 0
                    if lock_while_reset: arduino.write('0')
        
        #print "  Frequency: %d  - Volume: %f  - Button: %s" % (frequency, volume, button)
        #time.sleep(0.1)
except:
    print traceback.format_exc()


# stop and close
stream.stop_stream()
stream.close()
p.terminate()
arduino.close()
sys.stdout.write("Exited successfully.\n\n")

