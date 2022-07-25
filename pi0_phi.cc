
#include <iostream>
#include <cstdlib>
#include <bits/stdc++.h>
#include <ctime>
#include <stdio.h>
#include "Pythia8/Pythia.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "Pythia8/HeavyIons.h"
#include "TFile.h"
#include <vector>
#include "TLorentzVector.h"
#include "funcs.h"
#include "new_funcs.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TText.h"
#include "TLine.h"
#include "TPavesText.h"
#include "TPaveLabel.h"
#include "TBenchmark.h"

using namespace Pythia8;

void initset (char* argv[], std::string settings[6]) {
	for(int i=0; i<6; ++i) {
		settings[i] = argv[i+1];
	}
}

void initsetin (std::string settings[6]) {
	std::string settings1[6] = {	"111",										//seed
					"100000",									//number of events
					"2",										//collision(0-dAu,1-pAu,2-CuAu,3-He3Au)
					"8",										//particle code
					"/media/nikitix/Seagate_Expansion_Drive/diploma_output/channels/",	 	//output path
					"p"}; 										//target
	for (int i=0; i<6; ++i) settings[i]=settings1[i];
}

void print (int ** M) {
	int rows = sizeof(M) / sizeof(M[0]);
	int cols = sizeof(M[0]) / sizeof(int);
	std::cout << std::endl;
	std::cout << "bin" << "	" << "weight" << std::endl;
	for (int j=0; j<5; ++j) {
		std::cout << M[0][j] << "	" << M[1][j] << std::endl;	
	}
}


int main(int argc, char* argv[]) {
	//1.инициализация настроек
	std::string settings[6]; 
	if (argc==7) {initset(argv, settings);} else {initsetin(settings);} 

	//2.расчеты для протонов
	int ** Mpp = centralities(settings);
	print(Mpp);

	//3.расчеты для систем
	settings[5] = "Au";
	settings[1] = "100000";
	settings[2] = "0";
	hist3D_channels(settings);
	int ** MAB = centralities(settings);
	print(MAB);
	proj(settings, MAB);
	std::cout << "AB projecties made" << std::endl;
	
	//4.расчет факторов
	rfactors(settings, MAB, Mpp);


	//5.очистка памяти
	Free(Mpp); Free(MAB);

	return 0;
}


