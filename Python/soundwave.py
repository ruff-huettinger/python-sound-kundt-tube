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

# move script to from PC to UDOO via Network
# C:\pscp -pw 1199 C:/Projects/9355-KundtTube/ArduinoScript/KundtTube/KundtTube.ino udooer@udoo:/home/udooer/Project/soundwave/Arduino/soundwave/
# Upload the script
# C:\huettinger\Programs\putty.exe -ssh udoo:22 -l udooer -pw 1199

# 05.04.2019 - Jan Fischer - Small Code Update
#                            Standby Settings changed from F:10 V0.8 to F:0 V:0
#                            This Values can damage the Speaker. Timeout Conditions adjusted to 10 Seconds
# 28.06.2019 - Jan Fischer - Volume adjustment for Frequences (66Hz/160Hz/220Hz/330Hz/440Hz)
# 04.07.2019 - Jan Fischer - Volume adjustment for 220Hz and 440Hz
# 09.07.2019 - Jan Fischer - Added Config File (config.json) for Frequence/Volume Settings
# 26.07.2019 - Jan Fischer - Limit max Frequence to 500 Hz

print("Start Soundwave\n===============")

import serial
import json
import os
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

print("+ read config file")
# Read JSON Config File for Frequence/Volume Settings
currentPath = os.path.dirname(os.path.realpath(__file__))
configPath = os.path.join(currentPath, "config.json")
freqList = None
if (os.path.exists(configPath)):
	fconf = open(configPath, "r")
	config = json.load(fconf)
	freqList = config["frequences"]

print("+ initialize tone generator")
tone_gen = GstToneGenerator()
print("+ connect to arduino")
s = serial.Serial(port="/dev/ttymxc3", baudrate=115200, timeout=1)


class SerialThread(threading.Thread):
    def __init__(self, queue):
        threading.Thread.__init__(self)
        print("+ initialize frequence thread")
        self.queue = queue
        # frequency/volume adjustments
        self.freqvol = [
            {"start": 0, "end": 129, "volume": 60},
            {"start": 130, "end": 190, "volume": 90},
            {"start": 191, "end": 220, "volume": 50},
            {"start": 221, "end": 270, "volume": 20},
            {"start": 171, "end": 299, "volume": 60},
            {"start": 300, "end": 350, "volume": 70},
            {"start": 351, "end": 500, "volume": 100}]

    # adjust volume for preset frequencies
    def adjust_volume(self, frequency):
        newvolume = 100
        for e in self.freqvol:
            if frequency >= e["start"] and frequency <= e["end"]:
                newvolume = e["volume"]
                break
        newvolume = newvolume / 100.0
        return newvolume
    
    def run(self):
        start_time = time.time()
        
        s.write("freq_min 10\n")
        s.write("freq_max 500\n")
        s.write("freq_set 0\n")
        s.write("freq_step 1\n")
        s.write("vol_min 10\n")
        s.write("vol_max 100\n")
        s.write("vol_set 0\n")
        s.write("vol_step 1\n")
        
        print("+ start tone generator")
        volume = 0.0
        frequency = 0.0
        tone_gen.start()
        tone_gen.set_values(volume, frequency)
        #tone_gen.start()        
        print("Audio stream started ...") 
        self.queue.put("Audio stream started")
        isStandby = False
        
        print("+ run thread loop")
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
                        tone_gen.set_values(frequency, volume2)
                if time.time() - start_time > timeout and not isStandby:
                    s.write("off\n")
                    s.write("freq_min 10\n")
                    s.write("freq_max 500\n")
                    s.write("freq_set 0\n")
                    s.write("freq_step 1\n")
                    s.write("vol_min 10\n")
                    s.write("vol_max 100\n")
                    s.write("vol_set 0\n")
                    s.write("vol_step 1\n")		
                    print("standby mode")
                    isStandby = True                
                    volume = 0.0
                    frequency = 0.0
                    tone_gen.set_values(frequency, volume)
                    self.queue.put("Freq: %s \nVolu: %s" % (frequency,volume))

class App(tk.Tk):
    def __init__(self, newFreqList=None):
        tk.Tk.__init__(self)
        print("+ setup application window")
        self.geometry("300x300")
        frameLabel = tk.Frame(self, padx=40, pady =40)
        self.text = tk.Text(frameLabel, wrap='word', font='TimesNewRoman 30',
                            bg=self.cget('bg'), relief='flat')
        frameLabel.pack()
        self.text.pack()
        self.title("Sound Wave")
        self.queue = Queue.Queue()
        thread = SerialThread(self.queue)
        if (newFreqList != None):
			thread.freqvol = newFreqList
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
        
print("+ start application")
app = App(freqList)
try:
      app.mainloop()
except KeyboardInterrupt:
        # stop and close
        tone_gen.stop()
        s.close()
        exit()  
