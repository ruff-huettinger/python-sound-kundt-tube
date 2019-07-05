# sudo apt-get autoremove pyaudio*
# sudo apt-get autoremove pulseaudio*
# sudo apt-get install python-alsaaudio
# sudo apt-get install python python-tk
# sudo apt-get install gst0.10 
# sudo apt-get install xfce4-mixer 

# sudo apt-get install gstreamer0.10-plugins
# sudo apt-get install python-pyaudio python3-pyaudio 
# sudo apt-get install software-properties-common
# sudo add-apt-repository ppa:psyke83/ppa
# sudo apt-get update
# sudo apt-get install pulseaudio-equalizer
# sudo apt-get install libasound2-plugin-equal
# alsamixer -D equal

# move script to from PC to UDOO 
# C:\pscp -pw 1199 C:/Projects/9355-KundtTube/ArduinoScript/KundtTube/KundtTube.ino udooer@udoo:/home/udooer/Project/soundwave/Arduino/soundwave/
# Upload the script
# C:\huettinger\Programs\putty.exe -ssh udoo:22 -l udooer -pw 1199

# 05.04.2019 - Jan Fischer - Small Code Update
# Standby Settings changed from F:10 V0.8 to F:0 V:0
# This Values can damage the Speaker
# Timeout Conditions adjusted to 10 Seconds

# 28.06.2019 - Jan Fischer - Volume adjustment for Frequences (66Hz/160Hz/220Hz/330Hz/440Hz)
# 04.07.2019 - Jan Fischer - Volume adjustment for 220Hz and 440Hz

import serial
import threading
import Queue
import Tkinter as tk
import gobject
import gst
import gtk
import traceback
import time
import sys
import serial
from tone import GstToneGenerator
from tone import ToneGeneratorWidget

timeout=10.0

tone_gen = GstToneGenerator()
s = serial.Serial(port="/dev/ttymxc3", baudrate=115200, timeout=1)


class SerialThread(threading.Thread):
    def __init__(self, queue):
        threading.Thread.__init__(self)
        self.queue = queue
        # frequency/volume adjustments
        self.freqvol = [
            {"start": 0, "end": 129, "volume": 0.6},
            {"start": 130, "end": 190, "volume": 0.9},
            {"start": 191, "end": 220, "volume": 0.5},
            {"start": 221, "end": 270, "volume": 0.2},
            {"start": 171, "end": 299, "volume": 0.6},
            {"start": 300, "end": 350, "volume": 0.7},
            {"start": 351, "end": 500, "volume": 1.0}]

    # adjust volume for preset frequencies
    def adjust_volume(self, frequency):
        volume = 1.0
        for e in self.freqvol:
            if frequency >= e["start"] and frequency <= e["end"]:
                return e["volume"]
        return volume

    def run(self):
        start_time = time.time()
        
        s.write("freq_min 10\n")
        s.write("freq_max 999\n")
        s.write("freq_set 0\n")
        s.write("freq_step 1\n")
        s.write("vol_min 10\n")
        s.write("vol_max 100\n")
        s.write("vol_set 0\n")
        s.write("vol_step 1\n")
        
        volume = 0.0
        frequency = 0.0
        tone_gen.start()
        tone_gen.set_values(volume, frequency)
        #tone_gen.start()        
        print("Audio stream started ...") 
        self.queue.put("Audio stream started")
        isStandby = False
        while True:
                if s.inWaiting()>1:
                    isStandby = False
                    time.sleep(0.002)
                    serialIn = s.readline()                    
                    dataIn = serialIn.split(",")
                    if dataIn[0] == 'F' or dataIn[0] == 'V':
                        frequency = int(dataIn[1])
                        volume = float(dataIn[2]) / 100.0
                        volume2 = volume * self.adjust_volume(frequency)
                        print ("F: %s V: %s R: %s" % (frequency, volume, volume2))
                        self.queue.put("Freq: %s \nVolu: %s" % (frequency,volume))
                        start_time = time.time()
                        inactive_time = 0.0
                        tone_gen.set_values(frequency, volume2)
                if time.time() - start_time > timeout and not isStandby:
                    s.write("off\n")
                    s.write("freq_min 10\n")
                    s.write("freq_max 999\n")
                    s.write("freq_set 0\n")
                    s.write("freq_step 1\n")
                    s.write("vol_min 10\n")
                    s.write("vol_max 100\n")
                    s.write("vol_set 0\n")
                    s.write("vol_step 1\n")		
                    print "standby mode"
                    isStandby = True                
                    volume = 0.0
                    frequency=0.0
                    tone_gen.set_values(frequency, volume)
                    self.queue.put("Freq: %s \nVolu: %s" % (frequency,volume))

class App(tk.Tk):
    def __init__(self):
        tk.Tk.__init__(self)
        self.geometry("300x300")
        frameLabel = tk.Frame(self, padx=40, pady =40)
        self.text = tk.Text(frameLabel, wrap='word', font='TimesNewRoman 30',
                            bg=self.cget('bg'), relief='flat')
        frameLabel.pack()
        self.text.pack()
        self.title("Sound Wave")
        self.queue = Queue.Queue()
        thread = SerialThread(self.queue)
        thread.start()
        self.process_serial()

    def process_serial(self):
        while self.queue.qsize():
            try:
                self.text.delete(1.0, 'end')
                self.text.insert('end', self.queue.get())
            except Queue.Empty:
                pass
        self.after(100, self.process_serial)
        
app = App()
try:
      app.mainloop()
except KeyboardInterrupt:
        # stop and close
        tone_gen.stop()
        s.close()
        exit()  
