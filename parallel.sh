#!/bin/bash

make pi0_phi
#make new_funcs

#./pi0_phi 999 1000000 0 8 /media/nikitix/Seagate_Expansion_Drive/diploma_output/test/test_888/channels/ Au >> /media/nikitix/Seagate_Expansion_Drive/diploma_output/test/test_888/channels/logs/dAu_888_pSet8.log &

#./pi0_phi 999 1000000 1 8 /media/nikitix/Seagate_Expansion_Drive/diploma_output/test/test_888/channels/ Au >> /media/nikitix/Seagate_Expansion_Drive/diploma_output/test/test_888/channels/logs/pAu_888_pSet8.log &

./pi0_phi 999 50000 2 8 /media/nikitix/Seagate_Expansion_Drive/diploma_output/test/test_888/channels/ Au >> /media/nikitix/Seagate_Expansion_Drive/diploma_output/test/test_888/channels/logs/CuAu_888_pSet8.log &

#./pi0_phi 999 1000000 3 8 /media/nikitix/Seagate_Expansion_Drive/diploma_output/test/test_888/channels/ Au >> /media/nikitix/Seagate_Expansion_Drive/diploma_output/test/test_888/channels/logs/3HeAu_888_pSet8.log &

./pi0_phi >> /media/nikitix/Seagate_Expansion_Drive/diploma_output/test/test_888/channels/logs/pp_888_pSet8.log &




