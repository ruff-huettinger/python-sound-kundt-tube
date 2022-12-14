#sudo apt-get install  python-gst0.10

import gobject
import gst
import gtk

from tone import GstToneGenerator
from tone import ToneGeneratorWidget

import serial

# open serial port
arduino = serial.Serial(port="/dev/ttymxc3", baudrate=9600, timeout=1)

if __name__ == '__main__':
    def delete_cb(widget, event, tone_gen):
        tone_gen.stop()
        gtk.main_quit()

    def end_tone_cb(widget, tone_gen):
        tone_gen.stop()
        
    def start_tone_cb(widget, tone_gen):
        tone_gen.start()
        
    def tone_value_cb(widget, freq, volume, tone_gen):
        print 'tone-value', freq, volume
        tone_gen.set_values(freq, volume)
        
    tone_gen = GstToneGenerator()
    
    tone_wid = ToneGeneratorWidget()
    tone_wid.connect('delete-event', delete_cb, tone_gen)
    tone_wid.connect('end-tone', end_tone_cb, tone_gen)
    tone_wid.connect('start-tone', start_tone_cb, tone_gen)
    tone_wid.connect('tone-value', tone_value_cb, tone_gen)
    
    gobject.threads_init()
gtk.main()


while 1:
  if(arduino.inWaiting()>1):
      arduino.timeout=0.1
      freq = int(arduino.readline())
      tone_gen.set_values(freq, volume=1)   
      print freq       
