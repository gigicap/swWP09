#ifndef NRSSTree_h
#define NRSSTree_h
#include <unistd.h>
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1D.h>
#include <TSpectrum>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>

#include "../include/DecodeDT5743.h"
#include "../CAENDigitizer.h"

using std::vector;
using namespace std;

//to be parsed from config file in the final version
#define nADC 1024
#define n_peak 40
#define sigma_p 10 
#define peak_amp 0.3


double CFD_position(TH1D *,Double_t,Double_t, double, double, double);
double zeropos(TH1D *h)

class NRSSEvTree{
public :
   NRSSEvTree(TString rootfile);
   virtual ~NRSSEvTree();
   virtual void CreateTree(TString rootfile);
   virtual void FillTree();
   virtual void Store(CAEN_DGTZ_X743_EVENT_t *5743Event, double timestamp, int nev, int nrun);

 private:
   TFile* f;
   TTree* myTree;   
   //variables 
	double TimeStamp;
	int nEv;
	int nRun;

	int IsChannelActive[8];
	double TDC[8];

	TObjArray *RawSpectra;

	double baseline[8];
	int n_raw_maxima[8];

	double TrigTimeReference[8];

	TString specname[8]; 
	TString chname[8];

	TH1D *RawS[8];
};

class NRSSFCTree{
public :
   NRSSFCTree(TString rootfile);
   virtual ~NRSSFCTree();
   virtual void CreateTree(TString rootfile);
   virtual void FillTree();

 private:
   TFile* f;
   TTree* myTree;   
};

class NRSSESTree{
public :
   NRSSESTree(TString rootfile);
   virtual ~NRSSESTree();
   virtual void CreateTree(TString rootfile);
   virtual void FillTree();

 private:
   TFile* f;
   TTree* myTree;   
};

#endif

//creators and destructors 

NRSSEvTree::NRSSEvTree(TString rootfile) {
  CreateTree(rootfile);
}


NRSSEvTree::~NRSSEvTree() {
  f->cd();
  myTree->Write();
  f->Close();
}


NRSSFCTree::NRSSFCTree(TString rootfile) {
  CreateTree(rootfile);
}


NRSSFCTree::~NRSSFCTree() {
  f->cd();
  myTree->Write();
  f->Close();
}

NRSSESTree::NRSSESTree(TString rootfile) {
  CreateTree(rootfile);
}


NRSSESTree::~NRSSESTree() {
  f->cd();
  myTree->Write();
  f->Close();
}

//Other stuff

//CREATE TREE

void  NRSSEvTree::CreateTree(TString rootfile) {

  cout << "NRSS-EvTree:: Creating new tree" << endl;
  f=new TFile(rootfile,"recreate");

  myTree=new TTree("EvTree","EvTree");

  myTree->Branch("TimeStamp",&TimeStamp,"TimeStamp/D");
  myTree->Branch("nEv",&nEv,"nEv/I");
  myTree->Branch("nRun",&nRun,"nRun/I");
  myTree->Branch("IsChannelActive",IsChannelActive,"IsChannelActive[8]/I");
  myTree->Branch("TDC",TDC,"TDC[8]/D");
  myTree->Branch("baseline",baseline,"baseline[8]/D");
  myTree->Branch("n_raw_maxima",n_raw_maxima,"n_raw_maxima[8]/I");
  myTree->Branch("TrigTimeReference",TrigTimeReference,"TrigTimeReference[8]/D");

  //Define Raw Spectra
  RawSpectra = new TObjArray();
  chname[8] = {"A1","B1","A2","B2","A3","B3","4","LYSO"};
  for (int i = 0; i < 8; ++i){
  specname[i] =  "RawSpectrum_";
  pecname[i].Append(chname[i]);
  RawS[i] = new TH1D(specname[i],specname[i],nADC,0,nADC);
  RawSpectra->Add(Raws);  	
  }

return;
}

void  NRSSFCTree::CreateTree(TString rootfile) {      //TO BE IMPLEMENTED

  cout << "NRSS-FCTree:: Creating new tree" << endl;
  f=new TFile(rootfile,"recreate");

  myTree=new TTree("FCTree","FCTree");


  myTree->Branch("SumTot",&SumTot,"SumTot/D");

return;
}

void  NRSSESTree::CreateTree(TString rootfile) {   //TO BE IMPLEMENTED

  cout << "NRSS-ESTree:: Creating new tree" << endl;
  f=new TFile(rootfile,"recreate");

  myTree=new TTree("ESTree","ESTree");


  myTree->Branch("SumTot",&SumTot,"SumTot/D");


return;
}


//Fill Tree
void  NRSSEvTree::FillTree() {
  myTree->Fill();
}

void  NRSSFCTree::FillTree() {
  myTree->Fill();
}

void  NRSSESTree::FillTree() {
  myTree->Fill();
}


//Store Tree
void NRSSEvTree::Store(CAEN_DGTZ_X743_EVENT_t *5743Event, double timestamp, int nev, int nrun) {

TimeStamp = timestamp;
nEv = nev;
nRun = nrun;

for (int i = 0; i < 8; ++i)
{
	int gridx = i/2;
	IsChannelActive[i] = 5743Event->GrPresent[gridx];
	TDC[i] = (5743Event->DataGroup[gridx]).TDC;


	double base = 0;
	for (int j = 0; j < nADC; ++j)
	 {
	 double value;
	 if(i%2==0)	value = (5743Event->DataGroup[gridx]).DataChannel[0][j];
	 else value = (5743Event->DataGroup[gridx]).DataChannel[1][j];
	 RawS[i]->SetBinContent(j+1,value);
	 if(j<50) base = base + value;
	 } 
	 baseline[i] = base/50;

	//calculate maxima (TSpectrum)  <<otherwise use the other algo
	TSpectrum *spectrum = new TSpectrum(n_peak);
	//tspectrum find peaks
	Double_t xpmin = nADC*2;
	Double_t ypmin
    n_raw_maxima[i] =  s->Search(RawS[i],sigma_p,"",peak_amp);
    for (p=0;p<n_raw_maxima[i];p++) {
      Double_t xp = xpeaks[p];
      if(xp <= xpmin){
      		xpmin = xp;
      		Int_t bin = h->GetXaxis()->FindBin(xp);
      		ypmin = h->GetBinContent(bin); 
      		}    
   }

	//calculate time ref
	//TOBEIMPLEMENTED as CFD on the first peak 
	if(i<6){    //LED time ref only for the first 3 crystals
		TrigTimeReference[i] = CFD_position(RawS[i],xpmin,ypmin, sigma_p, baseline[i]);
	} 
	else TrigTimeReference[i] = 0;
}


return;
} 

//to be implemented
void NRSSFCTree::Store(){
	return;
}

//to be implemented
void NRSSESTree::Store(){
	return;
}


//other useful functions

double CFD_position(TH1D *h, Double_t x, Double_t y, double frac, double sigma0, double baseline){

	double cdfpos;
	TH1D *hn("hCFD","hCFD",(int)x+100,0,(int)x+100);

//CFD
for(int i=0; i<(int)x+100; i++){

double a0ati = 0.2*h->GetBinContent(i+1);

//shifted
int shift = (int)1.79*sigma0;


double s0 = baseline;
if(i+shift<=1023) s0 = h->GetBinContent(i+shift);
hn->SetBinContent(i+1,a0ati-s0); 

}

cdfpos = zeropos(hn);

cout<<"Time Reference position = "<<cdfpos<<endl; 


	return cdfpos;
}



double zeropos(TH1D *h){

double zero;
int zpos = start_ch;
int i=0; 

do{
i++;
}while(signbit(h->GetBinContent(i)) == signbit(h->GetBinContent(i+1)));

zpos = i;

cout<<"Zero pos: "<< zpos<<endl;

//linear interpolation
//zero = (h->GetBinContent(zpos+1)-(zpos+1)*h->GetBinContent(zpos))/(h->GetBinContent(zpos+1)-h->GetBinContent(zpos));

double y0 = h->GetBinContent(zpos);
double y1 = h->GetBinContent(zpos+1);

zero = (double)zpos - y0/(y1-y0);  

return zero;
}


