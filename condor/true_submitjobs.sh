#!/bin/sh
ls -l results/condor | grep -i $1 |awk '{print substr($NF,0,length($NF))}' > sub_$1.txt
awk '{print "condor_submit "$0}' sub_$1.txt > suball_$1.sh
rm sub_$1.txt
