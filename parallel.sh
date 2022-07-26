#!/bin/bash

#compile before start
make pi0_phi

#creating log dir if there is not yet
DIR="/media/nikitix/Seagate_Expansion_Drive/diploma_output"
[ ! -d "${DIR}/logs" ] && mkdir -p "${DIR}/logs"

./pi0_phi 999 1000000 0 8 ${DIR} Au >> ${DIR}/logs/dAu_888_pSet8.log &

./pi0_phi 999 1000000 1 8 ${DIR} Au >> ${DIR}/logs/pAu_888_pSet8.log &

./pi0_phi 999 50000 2 8 ${DIR} Au >> ${DIR}/logs/CuAu_888_pSet8.log &

./pi0_phi 999 1000000 3 8 ${DIR} Au >> ${DIR}/logs/3HeAu_888_pSet8.log &

./pi0_phi >> ${DIR}/logs/pp_888_pSet8.log &




