#include <TH1F.h>
#include <TTree.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TFile.h>
#include <TCanvas.h>
#include <unistd.h>
#include <TAxis.h>
#include <TMath.h>

#define N_ADC 16384 //14 bit
#define N_ADC_corr 16284 //14 bit
#define nPar 10

//TString filename = "600mV/HPGe_Co60.root";
//TString filename = "600mV/HPGe_Na22.root";
//TString filename = "HPGe_Bi207.root";
//TString filename = "HPGe_Co57.root";
//TString filename = "HPGe_Eu152.root";
//TString filename = "HPGe_Na22.root";
//TString filename = "Run1102_eventsCSPEC.root";
TString filename = "Calib_Pulser/1400mV/Run639_eventsCSPEC.root";

//TString Background_file = "/eli1/home/RunControlUser/RunControl/Analisi/HPGe/AnalisiDt5780_BKG.root";
TString Background_file = "Bkg1400mV_eventsCSPEC.root";


//Sottrazione background
double sign_min_P, sign_max_P,sign_min_bkg_P, sign_max_bkg_P;
//Peak Potassio 600 mV
/*
sign_min_P = sign_min_bkg_P = 8800;
sign_max_P = sign_max_bkg_P = 9000;
*/
//Peak Potassio 1400 mV
sign_min_P = sign_min_bkg_P = 6100;
sign_max_P = sign_max_bkg_P = 6200;

bool fit = true;
bool plot = true;
bool onepeak = true;

double sign_min;
double sign_max;
double min_fit;
double max_fit;
double sign_min_1;
double sign_max_1;
double min_fit_1;
double max_fit_1;

//Co60
min_fit = 2200;
max_fit = 2300;
sign_min = 2260;
sign_max = 2270;

min_fit_1 = 7500;
max_fit_1 = 8000;
sign_min_1 = 7750;
sign_max_1 = 7780;


