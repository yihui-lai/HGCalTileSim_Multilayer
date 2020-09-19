#!/bin/sh

N_event1=50
N_event2=50

python3 SubmitJobs.py --beamx 0 --beamy 0 -L 30  -w 15 -W 1.3 -t 3.8 -r 6.35 -d 1.7 -b 3.175 -S 0.2 -G 0 -m 0.98984772  -p 1 -N ${N_event1} -n 1 -i 1 -E 0 -a 3800
python3 SubmitJobs.py --beamx 0 --beamy 0 -L 30  -w 15 -W 1.3 -t 3.8 -r 6.35 -d 1.7 -b 3.175 -S 0.2 -G 0 -m 0.98984772  -p 1 -N ${N_event1} -n 2 -i 1 -E 0 -a 3800
python3 SubmitJobs.py --beamx 0 --beamy 0 -L 30  -w 15 -W 1.3 -t 3.8 -r 6.35 -d 1.7 -b 3.175 -S 0.2 -G 0 -m 0.98984772  -p 1 -N ${N_event2} -n 3 -i 1 -E 0 -a 3800

python3 SubmitJobs.py --beamx 0 --beamy 0 -L 40  -w 20 -W 1.3 -t 3.8 -r 6.35 -d 1.7 -b 3.175 -S 0.2 -G 0 -m 0.98984772  -p 1 -N ${N_event1} -n 1 -i 1 -E 0 -a 3800
python3 SubmitJobs.py --beamx 0 --beamy 0 -L 40  -w 20 -W 1.3 -t 3.8 -r 6.35 -d 1.7 -b 3.175 -S 0.2 -G 0 -m 0.98984772  -p 1 -N ${N_event1} -n 2 -i 1 -E 0 -a 3800
python3 SubmitJobs.py --beamx 0 --beamy 0 -L 40  -w 20 -W 1.3 -t 3.8 -r 6.35 -d 1.7 -b 3.175 -S 0.2 -G 0 -m 0.98984772  -p 1 -N ${N_event2} -n 3 -i 1 -E 0 -a 3800

python3 SubmitJobs.py --beamx 0 --beamy 0 -L 50  -w 25 -W 1.3 -t 3.8 -r 6.35 -d 1.7 -b 3.175 -S 0.2 -G 0 -m 0.98984772  -p 1 -N ${N_event1} -n 1 -i 1 -E 0 -a 3800
python3 SubmitJobs.py --beamx 0 --beamy 0 -L 50  -w 25 -W 1.3 -t 3.8 -r 6.35 -d 1.7 -b 3.175 -S 0.2 -G 0 -m 0.98984772  -p 1 -N ${N_event1} -n 2 -i 1 -E 0 -a 3800
python3 SubmitJobs.py --beamx 0 --beamy 0 -L 50  -w 25 -W 1.3 -t 3.8 -r 6.35 -d 1.7 -b 3.175 -S 0.2 -G 0 -m 0.98984772  -p 1 -N ${N_event2} -n 3 -i 1 -E 0 -a 3800



