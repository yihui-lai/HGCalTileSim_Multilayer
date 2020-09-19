#!/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

export LD_LIBRARY_PATH=/home/yichen/local/lib64/:/home/yichen/local/lib/:$LD_LIBRARY_PATH
source ${SCRIPT_DIR}/../g4env.sh

echo $LD_LIBRARY_PATH
ldd /data/users/eno/geant4.9.6.p04-install/lib64/libG4graphics_reps.so
${SCRIPT_DIR}/../bin/LY_SquareTrigger $@ > /dev/null ## Suppressing all outputs
exit $?
