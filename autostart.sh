#!/bin/bash
sleep 5
#python /home/udooer/Projects/ChladniPatterns/Python/soundwave.py &
#python /home/udooer/Projects/MagicFluid/Python/soundwave.py &
python /home/udooer/Projects/KundtTube/Python/soundwave.py &
sleep 15 
amixer -c 0 set Speaker 100% 
amixer -c 0 set PCM 37% 
