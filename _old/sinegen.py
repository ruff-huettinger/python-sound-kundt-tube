#!/usr/bin/env python

import sys, math, time, wave, pyaudio, numpy, pygame

frequency = 200.0
freq_steps = frequency / 10.0
volume = 0.01
counter = 0.0


def audio_callback(in_data, frame_count, time_info, status):
    sys.stdout.write("-")
    global frequency, volume, counter
    data = []
    #print time_info['output_buffer_dac_time']
    for i in range(0, frame_count):
        #data.append(  math.sin(time.clock() * frequency)  )
        #t = time_info['output_buffer_dac_time'] + i / 2048
        #data.append( math.sin(t * frequency) )
        data.append( math.sin(counter) )
        counter += 0.001 * frequency  # * frequency
        #while counter > 1.0: counter -= 1.0
    data = numpy.array( data, dtype=numpy.float32 )
    print(":")
    return (data, pyaudio.paContinue)
    #data = numpy.sin(numpy.arange(frame_count) * frequency)
    #data = data.astype(numpy.float32).tostring()
    #return (data, pyaudio.paContinue)


adevice = pyaudio.PyAudio()
adevice.open(output=True,
             format=pyaudio.paFloat32,
             rate=22050,
             channels=1,
             stream_callback=audio_callback,
             frames_per_buffer=4096*4)


def main():
    global frequency
    
    pygame.init()
    screen = pygame.display.set_mode((640, 480))

    while True:
        pressed = pygame.key.get_pressed()

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return

            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    return
                if event.key == pygame.K_UP:
                    frequency += freq_steps
                if event.key == pygame.K_DOWN:
                    frequency -= freq_steps


main()
adevice.terminate()


    
"""
if len(sys.argv) < 2:
    print("Plays a wave file.\n\nUsage: %s filename.wav" % sys.argv[0])
    sys.exit(-1)

wf = wave.open(sys.argv[1], 'rb')

# instantiate PyAudio (1)
p = pyaudio.PyAudio()

# define callback (2)
def callback(in_data, frame_count, time_info, status):
    data = wf.readframes(frame_count)
    return (data, pyaudio.paContinue)

# open stream using callback (3)
stream = p.open(format=p.get_format_from_width(wf.getsampwidth()),
                channels=wf.getnchannels(),
                rate=wf.getframerate(),
                output=True,
                stream_callback=callback)

# start the stream (4)
stream.start_stream()

# wait for stream to finish (5)
while stream.is_active():
    time.sleep(0.1)

# stop stream (6)
stream.stop_stream()
stream.close()
wf.close()

# close PyAudio (7)
p.terminate()
"""
