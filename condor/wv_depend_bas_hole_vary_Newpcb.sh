#!/bin/sh

N_event1=50
N_event2=50

#hole
python3 SubmitJobs.py --beamx 0 --beamy 0 -L 30 -w 15 -W 1.3 -t 3 -r 3.1 -d 1.6 -b 1.6 -S 0.2 -G 0 -E 1 -m 0.985 0.975 0.965 0.955 0.9 -p 0 1 -N ${N_event1} -n 1 -i 1 -a 3800 
python3 SubmitJobs.py --beamx 0 --beamy 0 -L 30 -w 15 -W 1.3 -t 3 -r 3.1 -d 1.6 -b 2.55 -S 0.2 -G 0 -E 1 -m 0.985 0.975 0.965 0.955 0.9 -p 0 1 -N ${N_event1} -n 1 -i 1 -a 3800
python3 SubmitJobs.py --beamx 0 --beamy 0 -L 30 -w 15 -W 1.3 -t 3 -r 3.1 -d 1.6 -b 3.175 -S 0.2 -G 0 -E 1 -m 0.985 0.975 0.965 0.955 0.9 -p 0 1 -N ${N_event1} -n 1 -i 1 -a 3800




