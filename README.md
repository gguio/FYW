# FYW
Program allows to generate and analyze particle collisions using PYTHIA and ROOT

Hey! This is code which I wrote for my Final Year Work. It allows to generate particle collisions using Monte-Carlo generator PYTHIA and to analyze collected data with ROOT library (check out "CERN ROOT"). Main feature of the program supposed to be flexibility in generating different types of events. One can use bash script to make parallel calculations of collisions with different particles. Also tools are provided to make some data analyses, as calculation of invariant mass and nuclear modification factor.
If you have questions or ideas how I might improve this code, you can text me on email: nikvish15@mail.ru.
Feel free to use this code as you want but I have two requests:
1. If you are making any improvments or functionality extentions by your own, please make sure to share it with others so anybody else could use it too.
2. In case this code happend to be useful for you, feel free to mention it :^)

Here is a step-by-step instruction of usage:
1. Open pi0_phi.cc and change defolt path 
2. Open parallel.sh and change defolt path there too
3. Open Makefile.inc and change paths to yours (you need atleast PYTHIA and ROOT installed)
4. In case you are not using parallel calculation you need compile pi0_phi.cc and new_funcs.cc. To do it write in command line "make pi0_phi" or "make new_funcs". Makefile will make file you writed and second one too but if the first file is up to date then Makefile will skip the second one. So be sure to make file that you changed last
5. In case you are using parallel calculation just start bash script by putting in command line "./parallel.sh". Don't forget to make it executable before starting by "chmod u+x parallel.sh".


