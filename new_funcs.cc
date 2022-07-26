
#include <iostream>
#include <cstdlib>
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

//******************************************************************************
//hist3D creates and writes 3D histograms of the desired collisions 
//for phi and pi0 separately. 
//The histogram of charged particles is written to the same file.

int hist3D (std::string argv[]) {
	//1.path initializing
	std::string path = argv[4];
	vector<std::string> name{"dAu_pT_eta_", "pAu_pT_eta_", "CuAu_pT_eta_", "He3Au_pT_eta_"}; 
	path.append(name[std::stoi(argv[2])]+argv[0]+"_pSet"+argv[3]+".root");
	TFile* outFile = new TFile(path.c_str(), "RECREATE");

	//2.histograms for data 
	TH3D *h3d_phi = new TH3D("h3d_phi", "hist for projections of phi", 500, 0, 500, 100, 0, 10, 100, -5, 5);
	TH3D *h3d_pi0 = new TH3D("h3d_pi0", "hist for projections of pi0", 500, 0, 500, 100, 0, 10, 100, -5, 5);
	TH1D *sumch = new TH1D("sumch", "Dist. of charged part.", 500, 0, 500);

	//3.initializing different settings for PYTHIA
	vector<std::string>proj{"1000010020", "2212", "1000290630", "1000020030"}; //{dAu, pAu, CuAu, 3HeAu}
	vector<std::string>SigFitDefPar{"11.54,1.66,0.32,0.0,0.0,0.0,0.0,0.0",     //dAu
					"11.54,1.66,0.32,0.0,0.0,0.0,0.0,0.0",     //pAu
					"11.34,1.66,0.32,0.0,0.0,0.0,0.0,0.0",     //CuAu
					"11.82,1.48,0.32,0.0,0.0,0.0,0.0,0.0"};    //3HeAU
	std::string beam1 = "Beams:idA = ";
	beam1.append(proj[std::stoi(argv[2])]);
	std::string SFDP = "HeavyIon:SigFitDefPar = ";
	SFDP.append(SigFitDefPar[std::stoi(argv[2])]);
	std::string random_seed_command = "Random:seed = ";
	random_seed_command.append(argv[0]);
	std:: string pSet = "PDF:pSet = ";

	//4.settings for PYTHIA
	Pythia pythia;
	pythia.readString(beam1); 
	pythia.readString("Beams:idB = 1000791970"); //Au
	pythia.readString("Beams:eA = 99.9"); 
	pythia.readString("Beams:eB = 100.");
	pythia.readString("Beams:frameType = 2");
	pythia.readString("Random:setSeed = on");
	pythia.readString(random_seed_command);
	pythia.readString("HeavyIon:SigFitErr = " 
			"0.02,0.02,0.1,0.05,0.05,0.0,0.1,0.0");
	pythia.readString("HeavyIon:SigFitDefPar = " +
			SigFitDefPar[std::stoi(argv[2])]);
	pythia.readString(pSet+argv[3]);
	pythia.readString("SoftQCD:all=on");
	pythia.readString("MultipartonInteractions:Kfactor = 0.5");
	pythia.readString("HeavyIon:SigFitNGen = 20");
	pythia.init();

	//5.triggering events
	std::vector<TLorentzVector> phi, pi0;
	TLorentzVector vphi, vpi0;
	int sumcharge=0;
	for (int i=0; i<std::stoi(argv[1]); ++i) {
		if (!pythia.next()) continue;
		sumcharge=0; phi.clear(); pi0.clear();
		for (int k=0; k<pythia.event.size(); ++k) {
			Particle & p = pythia.event[k];
			if (p.isFinal() && abs(p.eta())<4 && abs(p.eta())>3) sumcharge+=abs(p.charge());
			if (p.id()==333) {vphi.SetPxPyPzE(p.px(), p.py(), p.pz(), p.e()); phi.push_back(vphi);}
			if (p.id()==111) {vpi0.SetPxPyPzE(p.px(), p.py(), p.pz(), p.e()); pi0.push_back(vpi0);} 
		}
		sumch->Fill(sumcharge);
		for (int k=0; k<phi.size(); ++k) {
			h3d_phi->Fill(sumcharge, phi[k].Pt(), phi[k].Eta());
		}
		for (int k=0; k<pi0.size(); ++k) {
			h3d_pi0->Fill(sumcharge, pi0[k].Pt(), pi0[k].Eta());
		}
	}

	//6.writing data to the file and clearing allocated memory
	h3d_phi->Write(); h3d_pi0->Write(); sumch->Write();
	outFile->Close(); delete outFile;

	return 0;
}


//******************************************************************************
//centralities reads the histogram of charged particles
//created by the hist3D function and makes a breakdown 
//for centralities 0-20%, 20-40%, 40-60%, 60-100%. 
//Also, each bin is assigned the number of events 
//that fell into this centrality. 
//For example, 0-20%: bin=242, weight=1001

int **centralities(std::string argv[]) {
	//1.path initializing
	std::string path = argv[4];
	if (argv[5]=="Au") {
		vector<std::string> name{"dAu_pT_eta_", "pAu_pT_eta_", "CuAu_pT_eta_", "He3Au_pT_eta_"}; 
		path.append(name[std::stoi(argv[2])]+argv[0]+"_pSet"+argv[3]+".root");
	} else if (argv[5]=="p") {
			path.append("pp_pT_eta_"+argv[0]+"_pSet"+argv[3]+".root");
	} else {std::cout << "Write Au or p" << std::endl;}
	TFile* inputFile = new TFile(path.c_str(), "READ");

	//2.reading histogram of charged particles distribution
	TH1D *hist_sum_ch = (TH1D*)inputFile->Get("sumch");

	//3.allocating memory for bins and weights array
	double integral = hist_sum_ch->Integral(); std::cout << "integral = " << integral << std::endl;
	int centrality[3] = {20, 40, 60};
	int centrality_bins[3] = {0};
	double sum_content = 0.; int count=0;
	int ** M = new int * [2];
	for ( size_t i = 0; i < 2; ++i ) {
		M[i] = new int [5];
	}

	//4.calculation of bins and weights
	for (int i_bin = hist_sum_ch->GetNbinsX(); i_bin > 0; --i_bin) {
		sum_content += hist_sum_ch->GetBinContent(i_bin); 
		for (int j = 0; j < 3; ++j) {
			if ((sum_content/integral * 100. > centrality[j]) && (centrality_bins[j] == 0)) {centrality_bins[j] = i_bin;}
		}
	} sum_content=0; M[0][0] = hist_sum_ch->GetNbinsX(); M[1][0] = integral;
	for (int i_bin = hist_sum_ch->GetNbinsX(); i_bin > 0; --i_bin) {
		sum_content += hist_sum_ch->GetBinContent(i_bin); 
		for (int i = 0; i < 3; ++i) {
			if (i_bin == centrality_bins[i]) {M[0][i+1]=i_bin; M[1][i+1]=sum_content; sum_content=0;}
		}
	}
	M[0][4]=1; M[1][4]=M[1][0]-M[1][1]-M[1][2]-M[1][3];

	//5.printing bins and weight for check
	for (int i=0; i<5; ++i) std::cout << "bin = " << M[0][i] << ", weight = " << M[1][i] << std::endl;

	//6.closing stream and clearing space
	inputFile->Close(); delete inputFile;

	return M;
}

//Supporting function for clearing space 
//after bins and weights array is not needed anymore.

void Free( int ** M) {
    for ( size_t i = 0; i < 2; ++i ) {
        delete [] M[i];
    }
    delete [] M;
}


//******************************************************************************
//proj makes projections of a 3D histogram 
//from hist3D over a layout from centralities.

int proj(std::string argv[], int ** M) {
	//1.инициализация пути
	std::string path = argv[4]; vector<std::string> name{"dAu_pT_eta_", "pAu_pT_eta_", "CuAu_pT_eta_", "He3Au_pT_eta_"}; 
	if (argv[5]=="Au") {
		path.append(name[std::stoi(argv[2])]+argv[0]+"_pSet"+argv[3]+".root");
	} else if (argv[5]=="p") {
			path.append("pp_pT_eta_"+argv[0]+"_pSet"+argv[3]+".root");
	} else {std::cout << "Write Au or p" << std::endl;}
	TFile* inputFile = new TFile(path.c_str(), "READ");

	//2.чтение гистограмм
	TH3D *h_phi = (TH3D*)inputFile->Get("h3d_phi");
	TH3D *h_pi0 = (TH3D*)inputFile->Get("h3d_pi0");

	//3.инициализация гистограмм под распределения 
	vector<const char*> phi{"phi_pT_0_20", "phi_pT_20_40", "phi_pT_40_60", "phi_pT_60_100", "phi_pT_0_100", "phi_eta_0_20", "phi_eta_20_40", "phi_eta_40_60", "phi_eta_60_100", "phi_eta_0_100"};
	vector<const char*> pi0{"pi0_pT_0_20", "pi0_pT_20_40", "pi0_pT_40_60", "pi0_pT_60_100", "pi0_pT_0_100", "pi0_eta_0_20", "pi0_eta_20_40", "pi0_eta_40_60", "pi0_eta_60_100", "pi0_eta_0_100"};
	vector<TH1D*> phiPTmult(5), pi0PTmult(5), phiETAmult(5), pi0ETAmult(5), clones_phi(10), clones_pi0(10);
	int s[4] {20, 100, 45, 55}; int bins[5] = {500, 24, 17, 11, 1};
	for (int i=0; i<4; ++i) {
		phiPTmult[i] = h_phi->ProjectionY(phi[i], M[0][i+1], M[0][i], s[2], s[3], "o"); clones_phi[i] = (TH1D*) phiPTmult[i]->Clone(); clones_phi[i]->SetDirectory(0);
		phiETAmult[i] = h_phi->ProjectionZ(phi[i+5], M[0][i+1], M[0][i], s[0], s[1], "o"); clones_phi[i+5] = (TH1D*) phiETAmult[i]->Clone(); clones_phi[i+5]->SetDirectory(0);
		pi0PTmult[i] = h_pi0->ProjectionY(pi0[i], M[0][i+1], M[0][i], s[2], s[3], "o"); clones_pi0[i] = (TH1D*) pi0PTmult[i]->Clone(); clones_pi0[i]->SetDirectory(0);
		pi0ETAmult[i] = h_pi0->ProjectionZ(pi0[i+5], M[0][i+1], M[0][i], s[0], s[1], "o"); clones_pi0[i+5] = (TH1D*) pi0ETAmult[i]->Clone(); clones_pi0[i+5]->SetDirectory(0);
	} 
	phiPTmult[4] = h_phi->ProjectionY(phi[4], M[0][4], M[0][0], s[2], s[3], "o"); clones_phi[4] = (TH1D*) phiPTmult[4]->Clone(); clones_phi[4]->SetDirectory(0);
	pi0PTmult[4] = h_pi0->ProjectionY(pi0[4], M[0][4], M[0][0], s[2], s[3], "o"); clones_pi0[4] = (TH1D*) pi0PTmult[4]->Clone(); clones_pi0[4]->SetDirectory(0);
	phiETAmult[4] = h_phi->ProjectionZ(phi[9], M[0][4], M[0][0], s[0], s[1], "o"); clones_phi[9] = (TH1D*) phiETAmult[4]->Clone(); clones_phi[9]->SetDirectory(0);
	pi0ETAmult[4] = h_pi0->ProjectionZ(pi0[9], M[0][4], M[0][0], s[0], s[1], "o"); clones_pi0[9] = (TH1D*) pi0ETAmult[4]->Clone(); clones_pi0[9]->SetDirectory(0);

	//4.запись проекций в гистрограммы и очистка памяти
	inputFile->Close(); delete inputFile;
	std::string fout = argv[4]; 
	if (argv[5]=="Au") {
		fout.append(name[std::stoi(argv[2])]+argv[0]+"_pSet"+argv[3]+"_proj.root");
	} else if (argv[5]=="p") {
			fout.append("pp_pT_eta_"+argv[0]+"_pSet"+argv[3]+"_proj.root");
	} else {std::cout << "Write Au or p" << std::endl;}
	TFile* outFile = new TFile(fout.c_str(), "RECREATE"); for (int i=0; i<10; ++i) {clones_phi[i]->Write();} for (int i=0; i<10; ++i) {clones_pi0[i]->Write();}
	outFile->Close(); delete outFile;

	return 0;
}


//******************************************************************************
//hist3D_pp - same as hist3D 
//but for protons beams.
int hist3D_pp (std::string argv[]) {
	//1.инициализация пути
	std::string path = argv[4];
	path.append("pp_pT_eta_"+argv[0]+"_pSet"+argv[3]+".root");
	TFile* outFile = new TFile(path.c_str(), "RECREATE");

	//2.гистограммы для записи 
	TH3D *h3d_phi = new TH3D("h3d_phi", "hist for projections of phi", 500, 0, 500, 100, 0, 10, 100, -5, 5);
	TH3D *h3d_pi0 = new TH3D("h3d_pi0", "hist for projections of pi0", 500, 0, 500, 100, 0, 10, 100, -5, 5);
	TH1D *sumch = new TH1D("sumch", "Dist. of charged part.", 500, 0, 500);

	//3.инициализация вариантов настроек для PYTHIA
	std::string random_seed_command = "Random:seed = ";
	random_seed_command.append(argv[0]);
	std:: string pSet = "PDF:pSet = ";

	//4.настройки PYTHIA
	Pythia pythia;
	pythia.readString("Beams:idA = 2212"); 
	pythia.readString("Beams:idB = 2212"); 
	pythia.readString("Beams:eCM = 200."); 
	pythia.readString("Random:setSeed = on");
	pythia.readString(random_seed_command);
	pythia.readString(pSet+argv[3]);
	pythia.readString("SoftQCD:all=on");
	pythia.readString("MultipartonInteractions:Kfactor = 0.5");
	pythia.init();

	//5.запуск событий
	std::vector<TLorentzVector> phi, pi0;
	TLorentzVector vphi, vpi0;
	int sumcharge=0; static int nCharge[100];
	for (int i=0; i<std::stoi(argv[1]); ++i) {
		if (!pythia.next()) continue;
		sumcharge=0; phi.clear(); pi0.clear();
		for (int k=0; k<pythia.event.size(); ++k) {
			Particle & p = pythia.event[k];
			if (p.isFinal() && abs(p.eta())<4 && abs(p.eta())>3) sumcharge+=abs(p.charge());
			if (p.id()==333) {vphi.SetPxPyPzE(p.px(), p.py(), p.pz(), p.e()); phi.push_back(vphi);}
			if (p.id()==111) {vpi0.SetPxPyPzE(p.px(), p.py(), p.pz(), p.e()); pi0.push_back(vpi0);} 
		}
		sumch->Fill(sumcharge); nCharge[i] = sumcharge;
		for (int k=0; k<phi.size(); ++k) {
			h3d_phi->Fill(sumcharge, phi[k].Pt(), phi[k].Eta());
		}
		for (int k=0; k<pi0.size(); ++k) {
			h3d_pi0->Fill(sumcharge, pi0[k].Pt(), pi0[k].Eta());
		}
	}

	//6.запись в файл и очистка памяти
	h3d_phi->Write(); h3d_pi0->Write(); sumch->Write();
	outFile->Close(); delete outFile;

	return 0;
}


//******************************************************************************
//rfactors plots distributions of nuclear modification factors 
//over transverse momentum and pseudorapidity. 
//The calculation uses the breakdown by centralities made in proj.

int rfactors(std::string argv[], int ** MAB, int ** Mpp) {
	//1.инициализация путей
	std::string path_AB = argv[4]; 	std::string path_pp = argv[4];
	vector<std::string> name{"dAu", "pAu", "CuAu", "He3Au"}; path_AB.append(name[std::stoi(argv[2])]+"_pT_eta_"+argv[0]+"_pSet"+argv[3]+"_proj.root");
	path_pp.append("pp_pT_eta_"+argv[0]+"_pSet"+argv[3]+"_proj.root"); 
	TFile* input_AB = new TFile(path_AB.c_str(), "READ"); TFile* input_pp = new TFile(path_pp.c_str(), "READ");

	//2.чтение гистограмм
	vector<TH1D*> phi_AB(10), pi0_AB(10), phi_pp(10), pi0_pp(10), tphi_AB(10), tpi0_AB(10), tphi_pp(10), tpi0_pp(10); 
	vector<const char*> phi{"phi_pT_0_20", "phi_pT_20_40", "phi_pT_40_60", "phi_pT_60_100", "phi_pT_0_100", "phi_eta_0_20", "phi_eta_20_40", "phi_eta_40_60", "phi_eta_60_100", "phi_eta_0_100"};
	vector<const char*> pi0{"pi0_pT_0_20", "pi0_pT_20_40", "pi0_pT_40_60", "pi0_pT_60_100", "pi0_pT_0_100", "pi0_eta_0_20", "pi0_eta_20_40", "pi0_eta_40_60", "pi0_eta_60_100", "pi0_eta_0_100"};
	for (int i=0; i<10; ++i) {
		tphi_AB[i] = (TH1D*) input_AB->Get(phi[i]); phi_AB[i] = dynamic_cast<TH1D*>(tphi_AB[i]->Rebin(5, phi[i])); phi_AB[i]->SetDirectory(0); std::cout<<"done"<<std::endl;
		tpi0_AB[i] = (TH1D*) input_AB->Get(pi0[i]); pi0_AB[i] = dynamic_cast<TH1D*>(tpi0_AB[i]->Rebin(5, pi0[i])); pi0_AB[i]->SetDirectory(0);
		tphi_pp[i] = (TH1D*) input_pp->Get(phi[i]); phi_pp[i] = dynamic_cast<TH1D*>(tphi_pp[i]->Rebin(5, phi[i])); phi_pp[i]->SetDirectory(0);
		tpi0_pp[i] = (TH1D*) input_pp->Get(pi0[i]); pi0_pp[i] = dynamic_cast<TH1D*>(tpi0_pp[i]->Rebin(5, pi0[i])); pi0_pp[i]->SetDirectory(0);
	}

	//3.промежуточная очистка памяти 
	input_AB->Close(); delete input_AB;
	input_pp->Close(); delete input_pp;
	
	//4.расчет факторов ядерной модификации
	double N_coll[4][5] = {15.1, 10.2, 6.6, 3.2, 7.6,	//dAu
				8.2, 6.1, 4.4, 2.6, 4.7,	//pAU
				313., 129., 41.8, 10.1, 85.6,	//CuAu
				22.3, 14.8, 8.4, 3.4, 10.4};	//3HeAu
	
	double f_bias[4][5] = {0.94, 1., 1.03, 1.03, 0.889, 	//dAu
				0.9, 0.98, 1.02, 1., 0.858,	//pAu
				1., 1., 1., 1., 1., 		//CuAu
				0.95, 1.01, 1.02, 1.03, 0.89}; 	//3HeAu
	double x[5]; for (int i=0; i<5; ++i) {x[i]=f_bias[std::stoi(argv[2])][i]/N_coll[std::stoi(argv[2])][i];}
	for (int i=0; i<4; ++i) {
		phi_AB[i]->Scale(x[i]/MAB[1][i+1]); phi_pp[i]->Scale(1./Mpp[1][i+1]); phi_AB[i]->Divide(phi_pp[i]);
		pi0_AB[i]->Scale(x[i]/MAB[1][i+1]); pi0_pp[i]->Scale(1./Mpp[1][i+1]); pi0_AB[i]->Divide(pi0_pp[i]);
		phi_AB[i+5]->Scale(x[i]/MAB[1][i+1]); phi_pp[i+5]->Scale(1./Mpp[1][i+1]); phi_AB[i+5]->Divide(phi_pp[i+5]);
		pi0_AB[i+5]->Scale(x[i]/MAB[1][i+1]); pi0_pp[i+5]->Scale(1./Mpp[1][i+1]); pi0_AB[i+5]->Divide(pi0_pp[i+5]);
	}
	phi_AB[4]->Scale(x[4]/MAB[1][0]); phi_pp[4]->Scale(1./Mpp[1][0]); phi_AB[4]->Divide(phi_pp[4]);
	pi0_AB[4]->Scale(x[4]/MAB[1][0]); pi0_pp[4]->Scale(1./Mpp[1][0]); pi0_AB[4]->Divide(pi0_pp[4]);
	phi_AB[9]->Scale(x[4]/MAB[1][0]); phi_pp[9]->Scale(1./Mpp[1][0]); phi_AB[9]->Divide(phi_pp[9]);
	pi0_AB[9]->Scale(x[4]/MAB[1][0]); pi0_pp[9]->Scale(1./Mpp[1][0]); pi0_AB[9]->Divide(pi0_pp[9]);

	//5.запись в файл
	std::string name_out = argv[4]; name_out.append("Rfactors_"+name[std::stoi(argv[2])]+"_"+argv[0]+"_pSet"+argv[3]+".root");
	TFile* fout = new TFile(name_out.c_str(), "RECREATE");
	fout->cd(); for (int i=0; i<10; ++i) {phi_AB[i]->Write();} for (int i=0; i<10; ++i) {pi0_AB[i]->Write();}

	//6.очистка памяти
	fout->Close(); delete fout;

	return 0;
}


//******************************************************************************
//RMS calculates RMS using different parton distributions 
//

double **RMS (std::string argv[]) {
	//1.path to input files
	vector<std::string> path_AB{argv[4], argv[4], argv[4], argv[4]}; 	//std::string path_pp = argv[4];
	vector<std::string> name{"dAu", "pAu", "CuAu", "He3Au"}; vector<std::string> sets{"8", "7", "10", "11"};
	path_AB[0].append("Rfactors_"+name[0]+"_"+argv[0]+"_pSet"+argv[3]+".root");
	for (int i=1; i<4; ++i) {path_AB[i].append("mistake/Rfactors_"+name[0]+"_"+argv[0]+"_pSet"+sets[i]+".root"); std::cout<<argv[2]<<std::endl;}
	vector<TFile*> input_AB(4);
	for (int i=0; i<4; ++i) input_AB[i] = new TFile(path_AB[i].c_str(), "READ"); //TFile* input_pp = new TFile(path_pp.c_str(), "READ");

	//2.histograms reading
	vector<TH1D*> phi_AB(10), pi0_AB(10), tphi_AB(10), tpi0_AB(10);
	vector<const char*> phi{"phi_pT_0_20", "phi_pT_20_40", "phi_pT_40_60", "phi_pT_60_100", "phi_pT_0_100", "phi_eta_0_20", "phi_eta_20_40", "phi_eta_40_60", "phi_eta_60_100", "phi_eta_0_100"};
	vector<const char*> pi0{"pi0_pT_0_20", "pi0_pT_20_40", "pi0_pT_40_60", "pi0_pT_60_100", "pi0_pT_0_100", "pi0_eta_0_20", "pi0_eta_20_40", "pi0_eta_40_60", "pi0_eta_60_100", "pi0_eta_0_100"};
	vector<const char*> title{"d+Au", "p+Au", "Cu+Au", "{ю}^{3}He+Au"};
	vector<const char*> centries{"0-20%", "20-40%", "40-60%", "60-100%"};
	for (int k=0; k<4; ++k) {
		input_AB[k]->cd();
		tphi_AB[k] = (TH1D*) input_AB[k]->Get(phi[4]); phi_AB[k] = (TH1D*) tphi_AB[k]->Clone(); phi_AB[k]->SetDirectory(0); std::cout<<"done"<<std::endl;
		tpi0_AB[k] = (TH1D*) input_AB[k]->Get(pi0[4]); pi0_AB[k] = (TH1D*) tpi0_AB[k]->Clone(); pi0_AB[k]->SetDirectory(0);

		tphi_AB[k+4] = (TH1D*) input_AB[k]->Get(phi[9]); phi_AB[k+4] = (TH1D*) tphi_AB[k+4]->Clone(); phi_AB[k+4]->SetDirectory(0); std::cout<<"done"<<std::endl;
		tpi0_AB[k+4] = (TH1D*) input_AB[k]->Get(pi0[9]); pi0_AB[k+4] = (TH1D*) tpi0_AB[k+4]->Clone(); pi0_AB[k+4]->SetDirectory(0);
	}
	
	//3.intermediate memory cleanup
	for (int i=0; i<4; ++i) delete input_AB[i];

	//4.RMS calculation
	double ** rms = new double * [2];
	for ( size_t i = 0; i < 2; ++i ) {
		rms[i] = new double [40];
	} 
	
	for (int i=1; i<21; ++i) { 
		rms[0][i-1] = sqrt((pow(phi_AB[0]->GetBinContent(i)-phi_AB[1]->GetBinContent(i),2)+pow(phi_AB[0]->GetBinContent(i)-phi_AB[2]->GetBinContent(i),2)+pow(phi_AB[0]->GetBinContent(i)-phi_AB[3]->GetBinContent(i),2))/12);///phi_AB[0]->GetBinContent(i); 
		//if (isnan(rms[0][i-1])) rms[0][i-1] = 0.; if (isinf(rms[0][i-1])) rms[0][i-1] = 1.; if (rms[0][i-1]>1.) rms[0][i-1] = 1.; 
		rms[1][i-1] = sqrt((pow(pi0_AB[0]->GetBinContent(i)-pi0_AB[1]->GetBinContent(i),2)+pow(pi0_AB[0]->GetBinContent(i)-pi0_AB[2]->GetBinContent(i),2)+pow(pi0_AB[0]->GetBinContent(i)-pi0_AB[3]->GetBinContent(i),2))/12);///pi0_AB[0]->GetBinContent(i);
		//if (isnan(rms[1][i-1])) rms[1][i-1] = 0.; if (isinf(rms[1][i-1])) rms[1][i-1] = 1.; if (rms[1][i-1]>1.) rms[1][i-1] = 1.;
		//std::cout<< phi_AB[3]->GetBinContent(i) << std::endl;
		rms[0][i+19] = sqrt((pow(phi_AB[4]->GetBinContent(i)-phi_AB[5]->GetBinContent(i),2)+pow(phi_AB[4]->GetBinContent(i)-phi_AB[6]->GetBinContent(i),2)+pow(phi_AB[4]->GetBinContent(i)-phi_AB[7]->GetBinContent(i),2))/12);///phi_AB[4]->GetBinContent(i);
		//if (isnan(rms[0][i+19])) rms[0][i+19] = 0.; if (isinf(rms[0][i+19])) rms[0][i+19] = 1.; if (rms[0][i+19]>1.) rms[0][i+19] = 1.; 
		rms[1][i+19] = sqrt((pow(pi0_AB[4]->GetBinContent(i)-pi0_AB[5]->GetBinContent(i),2)+pow(pi0_AB[4]->GetBinContent(i)-pi0_AB[6]->GetBinContent(i),2)+pow(pi0_AB[4]->GetBinContent(i)-pi0_AB[7]->GetBinContent(i),2))/12);//pi0_AB[4]->GetBinContent(i);
		//if (isnan(rms[1][i+19])) rms[1][i+19] = 0.; if (isinf(rms[1][i+19])) rms[1][i+19] = 1.; if (rms[1][i+19]>1.) rms[1][i+19] = 1.;
	}
	
	//5.printing RMS 
	std::cout << "pT" << "	" << "RMS for phi%" << "	" << "RMS for pi0%" << std::endl;	
	for (int i=0; i<40; ++i) {
		std::cout << (double)(i+1)/2 << "	&	" << round(rms[0][i]*1000)/1000 << "	&	" << round(rms[1][i]*1000)/1000 << "	\\\\" << std::endl;
	}

	return rms;
}

//Supporting function for memory cleanup
//after RMS is not needed anymore

void Free_rms( double ** rms) {
    for ( size_t i = 0; i < 2; ++i ) {
        delete [] rms[i];
    }
    delete [] rms;
}


//******************************************************************************
//hist3D_channels creates and writes 3D histograms of the desired collisions 
//for phi and pi0 separately as hist3D but particles are picked by
//decay channels, i.e. phi->K+K- and pi0->yy (check out Particle Data Group website). 
//The histogram of charged particles is written to the same file.

int hist3D_channels (std::string argv[]) {
	//1.инициализация пути
	std::string path = argv[4];
	vector<std::string> name{"dAu_pT_eta_", "pAu_pT_eta_", "CuAu_pT_eta_", "He3Au_pT_eta_"}; 
	path.append(name[std::stoi(argv[2])]+argv[0]+"_pSet"+argv[3]+".root");
	TFile* outFile = new TFile(path.c_str(), "RECREATE");

	//2.гистограммы для записи 
	TH3D *h3d_phi = new TH3D("h3d_phi", "hist for projections of phi", 500, 0, 500, 100, 0, 10, 100, -5, 5);
	TH3D *h3d_pi0 = new TH3D("h3d_pi0", "hist for projections of pi0", 500, 0, 500, 100, 0, 10, 100, -5, 5);
	TH1D *sumch = new TH1D("sumch", "Dist. of charged part.", 500, 0, 500);
	TH1D *phi_mass = new TH1D("phi_mass", "invariant mass of phi", 3000, 0.985, 1.15);
	TH1D *pi0_mass = new TH1D("pi0_mass", "invariant mass of pi0", 3000, 0., 0.3);
	TH1D *bckgr_phi = new TH1D("bckgr_phi", "combinatorial background fot \\phi", 3000, 0.985, 1.15);
	TH1D *bckgr_pi0 = new TH1D("bckgr_pi0", "combinatorial background fot \\pi^0", 3000, 0., 0.3);

	//3.инициализация вариантов настроек для PYTHIA
	vector<std::string>proj{"1000010020", "2212", "1000290630", "1000020030"}; //{dAu, pAu, CuAu, 3HeAu}
	vector<std::string>SigFitDefPar{"11.54,1.66,0.32,0.0,0.0,0.0,0.0,0.0",     //dAu
					"11.54,1.66,0.32,0.0,0.0,0.0,0.0,0.0",     //pAu
					"11.34,1.66,0.32,0.0,0.0,0.0,0.0,0.0",     //CuAu
					"11.82,1.48,0.32,0.0,0.0,0.0,0.0,0.0"};    //3HeAU
	std::string beam1 = "Beams:idA = ";
	beam1.append(proj[std::stoi(argv[2])]);
	std::string SFDP = "HeavyIon:SigFitDefPar = ";
	SFDP.append(SigFitDefPar[std::stoi(argv[2])]);
	std::string random_seed_command = "Random:seed = ";
	random_seed_command.append(argv[0]);
	std:: string pSet = "PDF:pSet = ";

	//4.настройки PYTHIA
	Pythia pythia;
	pythia.readString(beam1); 
	pythia.readString("Beams:idB = 1000791970"); //Au
	pythia.readString("Beams:eA = 99.9"); 
	pythia.readString("Beams:eB = 100.");
	pythia.readString("Beams:frameType = 2");
	pythia.readString("Random:setSeed = on");
	pythia.readString(random_seed_command);
	pythia.readString("HeavyIon:SigFitErr = " 
			"0.02,0.02,0.1,0.05,0.05,0.0,0.1,0.0");
	pythia.readString("HeavyIon:SigFitDefPar = " +
			SigFitDefPar[std::stoi(argv[2])]);
	pythia.readString(pSet+argv[3]);
	pythia.readString("SoftQCD:all=on");
	pythia.readString("MultipartonInteractions:Kfactor = 0.5");
	pythia.readString("HeavyIon:SigFitNGen = 20");
	pythia.init();

	//5.запуск событий
	std::vector<TLorentzVector> phi_Kpos, phi_Kneg, pi0, bck_Kpos, bck_Kneg, bck_Ypos, bck_Yneg;
	TLorentzVector vphi, vpi0, vbck;
	int sumcharge=0; int count_pi0 = 0; int count_phi = 0;
	for (int i=0; i<std::stoi(argv[1]); ++i) {
		if (!pythia.next()) continue;
		sumcharge=0; phi_Kpos.clear(); phi_Kneg.clear(); pi0.clear();
		if (i%2 == 0) {bck_Kpos.clear(); bck_Kneg.clear(); bck_Ypos.clear(); bck_Yneg.clear();}
		for (int k=0; k<pythia.event.size(); ++k) {
			Particle & p = pythia.event[k];
			if (p.isFinal() && abs(p.eta())<4 && abs(p.eta())>3) sumcharge+=abs(p.charge());
			if (p.id()==321 && p.isFinal()) {vphi.SetPxPyPzE(p.px(), p.py(), p.pz(), p.e()); phi_Kpos.push_back(vphi);}
			if (p.id()==321 && p.isFinal() && !(i%2==0)) {vphi.SetPxPyPzE(p.px(), p.py(), p.pz(), p.e()); bck_Kpos.push_back(vphi);}
			if (p.id()==-321 && p.isFinal()) {vphi.SetPxPyPzE(p.px(), p.py(), p.pz(), p.e()); phi_Kneg.push_back(vphi);}
			if (p.id()==-321 && p.isFinal() && i%2==0) {vphi.SetPxPyPzE(p.px(), p.py(), p.pz(), p.e()); bck_Kneg.push_back(vphi);}
			if (p.id()==22 && p.isFinal()) {vpi0.SetPxPyPzE(p.px(), p.py(), p.pz(), p.e()); pi0.push_back(vpi0);} 
			if (p.id()==22 && p.isFinal() && i%2==0) {vpi0.SetPxPyPzE(p.px(), p.py(), p.pz(), p.e()); bck_Yneg.push_back(vpi0);}
			if (p.id()==22 && p.isFinal() && !(i%2==0)) {vpi0.SetPxPyPzE(p.px(), p.py(), p.pz(), p.e()); bck_Ypos.push_back(vpi0);}
		}
		sumch->Fill(sumcharge); 
		for (int k=0; k<phi_Kpos.size(); ++k) {
			for (int j=k; j<phi_Kneg.size(); ++j) {
				//count_phi+=1; if (count_phi>1e10) count_phi=2e9; if (count_phi>1e9) break;
				h3d_phi->Fill(sumcharge, (phi_Kpos[k]+phi_Kneg[j]).Pt(), (phi_Kpos[k]+phi_Kneg[j]).Eta());
				phi_mass->Fill((phi_Kpos[k]+phi_Kneg[j]).M());}
		}
		for (int k=0; k<pi0.size(); ++k) {
			for (int j=k; j<pi0.size(); ++j) {
				//count_pi0+=1; if (count_pi0>1e10) count_pi0=2e9; if (count_pi0>1e9) break; 
				h3d_pi0->Fill(sumcharge, (pi0[k]+pi0[j]).Pt(), (pi0[k]+pi0[j]).Eta());
				pi0_mass->Fill((pi0[k]+pi0[j]).M());
				}
		}
		if (!(i%2==0)) {
			for (int m=0; m<bck_Kpos.size(); ++m) {
				for (int c=0; c<bck_Kneg.size(); ++c) {bckgr_phi->Fill((bck_Kpos[m]+bck_Kneg[c]).M());}
			}
			for (int m=0; m<bck_Ypos.size(); ++m) {
				for (int c=m; c<bck_Yneg.size(); ++c) {bckgr_pi0->Fill((bck_Ypos[m]+bck_Yneg[c]).M());}
			}
		}
	}

	//6.запись в файл и очистка памяти
	h3d_phi->Write(); h3d_pi0->Write(); sumch->Write(); phi_mass->Write(); pi0_mass->Write(); bckgr_phi->Write(); bckgr_pi0->Write();
	outFile->Close(); delete outFile;

	return 0;
}


//******************************************************************************
//hist3D_pp_channels - same as hist3D_channels but for proton beams

int hist3D_pp_channels (std::string argv[]) {
	//1.инициализация пути
	std::string path = argv[4];
	path.append("pp_pT_eta_"+argv[0]+"_pSet"+argv[3]+".root");
	TFile* outFile = new TFile(path.c_str(), "RECREATE");

	//2.гистограммы для записи 
	TH3D *h3d_phi = new TH3D("h3d_phi", "hist for projections of phi", 500, 0, 500, 100, 0, 10, 100, -5, 5);
	TH3D *h3d_pi0 = new TH3D("h3d_pi0", "hist for projections of pi0", 500, 0, 500, 100, 0, 10, 100, -5, 5);
	TH1D *sumch = new TH1D("sumch", "Dist. of charged part.", 500, 0, 500);
	TH1D *phi_mass = new TH1D("phi_mass", "invariant mass of phi", 3000, 0.985, 1.15);
	TH1D *pi0_mass = new TH1D("pi0_mass", "invariant mass of pi0", 3000, 0., 0.3);
	TH1D *bckgr_phi = new TH1D("bckgr_phi", "combinatorial background fot \\phi", 3000, 0.985, 1.15);
	TH1D *bckgr_pi0 = new TH1D("bckgr_pi0", "combinatorial background fot \\pi^0", 3000, 0., 0.3);

	//3.инициализация вариантов настроек для PYTHIA
	std::string random_seed_command = "Random:seed = ";
	random_seed_command.append(argv[0]);
	std:: string pSet = "PDF:pSet = ";

	//4.настройки PYTHIA
	Pythia pythia;
	pythia.readString("Beams:idA = 2212"); 
	pythia.readString("Beams:idB = 2212"); 
	pythia.readString("Beams:eCM = 200."); 
	pythia.readString("Random:setSeed = on");
	pythia.readString(random_seed_command);
	pythia.readString(pSet+argv[3]);
	pythia.readString("SoftQCD:all=on");
	pythia.readString("MultipartonInteractions:Kfactor = 0.5");
	pythia.init();

	//5.запуск событий
	std::vector<TLorentzVector> phi_Kpos, phi_Kneg, pi0, bck_Kpos, bck_Kneg, bck_Ypos, bck_Yneg;
	TLorentzVector vphi, vpi0, vbck;
	int sumcharge=0; int count_pi0 = 0; int count_phi = 0; 
	for (int i=0; i<std::stoi(argv[1]); ++i) {
		if (!pythia.next()) continue;
		sumcharge=0; phi_Kpos.clear(); phi_Kneg.clear(); pi0.clear();
		if (i%2 == 0) {bck_Kpos.clear(); bck_Kneg.clear(); bck_Ypos.clear(); bck_Yneg.clear();}
		for (int k=0; k<pythia.event.size(); ++k) {
			Particle & p = pythia.event[k];
			if (p.isFinal() && abs(p.eta())<4 && abs(p.eta())>3) sumcharge+=abs(p.charge());
			if (p.id()==321 && p.isFinal()) {vphi.SetPxPyPzE(p.px(), p.py(), p.pz(), p.e()); phi_Kpos.push_back(vphi);}
			if (p.id()==321 && p.isFinal() && !(i%2==0)) {vphi.SetPxPyPzE(p.px(), p.py(), p.pz(), p.e()); bck_Kpos.push_back(vphi);}
			if (p.id()==-321 && p.isFinal()) {vphi.SetPxPyPzE(p.px(), p.py(), p.pz(), p.e()); phi_Kneg.push_back(vphi);}
			if (p.id()==-321 && p.isFinal() && i%2==0) {vphi.SetPxPyPzE(p.px(), p.py(), p.pz(), p.e()); bck_Kneg.push_back(vphi);}
			if (p.id()==22 && p.isFinal()) {vpi0.SetPxPyPzE(p.px(), p.py(), p.pz(), p.e()); pi0.push_back(vpi0);} 
			if (p.id()==22 && p.isFinal() && i%2==0) {vpi0.SetPxPyPzE(p.px(), p.py(), p.pz(), p.e()); bck_Yneg.push_back(vpi0);}
			if (p.id()==22 && p.isFinal() && !(i%2==0)) {vpi0.SetPxPyPzE(p.px(), p.py(), p.pz(), p.e()); bck_Ypos.push_back(vpi0);}
		}
		sumch->Fill(sumcharge); 
		for (int k=0; k<phi_Kpos.size(); ++k) {
			for (int j=k; j<phi_Kneg.size(); ++j) {
				//count_phi+=1; if (count_phi>1e10) count_phi=2e9; if (count_phi>1e9) break;
				h3d_phi->Fill(sumcharge, (phi_Kpos[k]+phi_Kneg[j]).Pt(), (phi_Kpos[k]+phi_Kneg[j]).Eta());
				phi_mass->Fill((phi_Kpos[k]+phi_Kneg[j]).M());}
		}
		for (int k=0; k<pi0.size(); ++k) {
			for (int j=k; j<pi0.size(); ++j) {
				//count_pi0+=1; if (count_pi0>1e10) count_pi0=2e9; if (count_pi0>1e9) break;
				h3d_pi0->Fill(sumcharge, (pi0[k]+pi0[j]).Pt(), (pi0[k]+pi0[j]).Eta());
				pi0_mass->Fill((pi0[k]+pi0[j]).M());
			}
		}
		if (!(i%2==0)) {
			for (int m=0; m<bck_Kpos.size(); ++m) {
				for (int c=0; c<bck_Kneg.size(); ++c) {bckgr_phi->Fill((bck_Kpos[m]+bck_Kneg[c]).M());}
			}
			for (int m=0; m<bck_Ypos.size(); ++m) {
				for (int c=m; c<bck_Yneg.size(); ++c) {bckgr_pi0->Fill((bck_Ypos[m]+bck_Yneg[c]).M());}
			}
		}
	}

	//6.запись в файл и очистка памяти
	h3d_phi->Write(); h3d_pi0->Write(); sumch->Write(); phi_mass->Write(); pi0_mass->Write(); bckgr_phi->Write(); bckgr_pi0->Write();
	outFile->Close(); delete outFile;

	return 0;
}


//******************************************************************************
//******************************************************************************
//Functions below are not finished and are not working.

void drawing (std::string argv[]) {
	//1.инициализация путей
	/*std::string path_AB = argv[4]; 	std::string path_pp = argv[4];
	vector<std::string> name{"dAu", "pAu", "CuAu", "He3Au"}; path_AB.append(name[std::stoi(argv[2])]+"_pT_eta_"+argv[0]+"_pSet"+argv[3]+"_proj.root");
	path_pp.append("pp_pT_eta_"+argv[0]+"_pSet"+argv[3]+"_proj.root"); 
	TFile* input_AB = new TFile(path_AB.c_str(), "READ"); TFile* input_pp = new TFile(path_pp.c_str(), "READ");

	//2.считывание гистограмм
	vector<TH1D*> phi_AB(10), pi0_AB(10), phi_pp(10), pi0_pp(10), tphi_AB(10), tpi0_AB(10), tphi_pp(10), tpi0_pp(10); 
	vector<const char*> phi{"phi_pT_0_20", "phi_pT_20_40", "phi_pT_40_60", "phi_pT_60_100", "phi_pT_0_100", "phi_eta_0_20", "phi_eta_20_40", "phi_eta_40_60", "phi_eta_60_100", "phi_eta_0_100"};
	vector<const char*> pi0{"pi0_pT_0_20", "pi0_pT_20_40", "pi0_pT_40_60", "pi0_pT_60_100", "pi0_pT_0_100", "pi0_eta_0_20", "pi0_eta_20_40", "pi0_eta_40_60", "pi0_eta_60_100", "pi0_eta_0_100"};
	for (int i=0; i<10; ++i) {
		tphi_AB[i] = (TH1D*) input_AB->Get(phi[i]); (TH1D*)phi_AB[i] = tphi_AB[i]->Get(phi[i]); phi_AB[i]->SetDirectory(0); std::cout<<"done"<<std::endl;
		tpi0_AB[i] = (TH1D*) input_AB->Get(pi0[i]); (TH1D*)pi0_AB[i] = tpi0_AB[i]->Get(pi0[i]); pi0_AB[i]->SetDirectory(0);
		tphi_pp[i] = (TH1D*) input_pp->Get(phi[i]); (TH1D*)phi_pp[i] = tphi_pp[i]->Get(phi[i]); phi_pp[i]->SetDirectory(0);
		tpi0_pp[i] = (TH1D*) input_pp->Get(pi0[i]); (TH1D*)pi0_pp[i] = tpi0_pp[i]->Get(pi0[i]); pi0_pp[i]->SetDirectory(0);
	}

	//3.промежуточная очистка памяти 
	input_AB->Close(); delete input_AB;
	input_pp->Close(); delete input_pp;*/

	//4.рисование гистограмм
	TCanvas *c1 = new TCanvas("c1","Canvas Example",200,10,800,800); gBenchmark->Start("canvas");
	TPad *pad1 = new TPad("pad1","This is pad1",0.05,0.52,0.95,0.97);
	TPad *pad2 = new TPad("pad2","This is pad2",0.05,0.02,0.95,0.47);
	pad1->Draw();
	pad2->Draw();
	pad2->cd();
	TPad *pad21 = new TPad("pad21","First subpad of pad2",0.02,0.05,0.48,0.95,17,3);
	TPad *pad22 = new TPad("pad22","Second subpad of pad2",0.52,0.05,0.98,0.95,17,3);
	pad21->Draw();
	pad22->Draw();
	pad1->cd();
	float xt1 = 0.5;
	float yt1 = 0.1;
	TText *t1 = new TText(0.5,yt1,"ROOT");
	t1->SetTextAlign(22);
	t1->SetTextSize(0.05);
	t1->Draw();
	TLine *line1 = new TLine(0.05,0.05,0.80,0.70);
	line1->SetLineWidth(8);
	line1->SetLineColor(2);
	line1->Draw();
	line1->DrawLine(0.6,0.1,0.9,0.9);
	TLine *line2 = new TLine(0.05,0.70,0.50,0.10);
	line2->SetLineWidth(4);
 	line2->SetLineColor(5);
   	line2->Draw();
 
   	pad21->cd();
	TText *t21 = new TText(0.05,0.8,"test1");
  	t21->SetTextSize(0.1);
  	t21->Draw();
  	float xp2 = 0.5;
  	float yp2 = 0.4;
  	TPavesText *paves = new TPavesText(0.1,0.1,xp2,yp2);
  	paves->AddText("piu");
  	paves->AddText("а русский пишет или нет");
  	paves->AddText("Text formatting is automatic");
  	paves->SetFillColor(43);
  	paves->Draw();
  	pad22->cd();
  	TText *t22 = new TText(0.05,0.8,"test2");
  	t22->SetTextSize(0.1);
  	t22->Draw();
  	float xlc = 0.01;
  	float ylc = 0.01;
  	TPaveLabel *label = new TPaveLabel(xlc, ylc, xlc+0.8, ylc+0.1,"This is a PaveLabel");
  	label->SetFillColor(24);
  	label->Draw();
	c1->Update();
	gBenchmark->Show("canvas");
	
	
}


//******************************************************************************
//minus_background вычитает 
//комбинаторный фон


int minus_background (std::string argv[]) {
	//1.инициализация путей и настроек
	std::string path = argv[4];
	vector<std::string> name{"dAu_pT_eta_", "pAu_pT_eta_", "CuAu_pT_eta_", "He3Au_pT_eta_"}; 
	path.append(name[std::stoi(argv[2])]+argv[0]+"_pSet"+argv[3]+".root");
	TFile* fin = new TFile(path.c_str(), "READ");

	//2.чтение гистограмм
	int num = 2;
	vector<TH1D*> mass(num), back(num), tempm(num), tempb(num); 
	vector<const char*> part = {"phi_mass", "pi0_mass"};
	vector<const char*> fon = {"bckgr_phi", "bckgr_pi0"};

	for (int i=0; i<num; ++i) {
		tempm[i] = (TH1D*)fin->Get(part[i]); mass[i] = (TH1D*)tempm[i]->Clone(); mass[i]->SetDirectory(0); std::cout << "done" << std::endl;
		tempb[i] = (TH1D*)fin->Get(fon[i]); back[i] = (TH1D*)tempb[i]->Clone(); back[i]->SetDirectory(0);
	}

	//3.промежуточная очистка памяти
	fin->Close(); delete fin;

	//4.вычет комбинаторного фона
	double val_phi, val_pi0;
	for (int i=0; i<num; ++i) {
		mass[i]->Sumw2(true); back[i]->Sumw2(true);
		back[i]->Scale(2.);
		mass[i]->Add(back[i], -1); 
	} 
	//5.запись в новый файл
	std::string path_out = argv[4];
	vector<std::string> n{"dAu", "pAu", "CuAu", "He3Au"};
	path_out.append(n[std::stoi(argv[2])]+"_inv_masses_"+argv[0]+"_pSet"+argv[3]+".root");
	TFile* fout = new TFile(path_out.c_str(), "RECREATE"); 
	for (int i=0; i<num; ++i) mass[i]->Write();
	
	//6.очистка памяти
	fout->Close(); delete fout;

	return 0;
}



