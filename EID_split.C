#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TObjString.h"
#include "TSystem.h"
#include "TROOT.h"

//#include "TMVAGui.C"

#if not defined(__CINT__) || defined(__MAKECINT__)
// needs to be included when makecint runs (ACLIC)
#include "TMVA/Factory.h"
#include "TMVA/Tools.h"
//#include "TMVA/Category.h"
#endif

void EID_split(TString location, TString ptrange, TString EXTRA) {

  TMVA::Tools::Instance();
  (TMVA::gConfig().GetVariablePlotting()).fNbinsXOfROCCurve = 400;
  // OutPut File
  cout << "begin?" << endl;
  TString outfileName = "out/TMVA_"+location+"_"+ptrange+"_"+EXTRA+".root";
  cout << "---> Working with OutfileName = " << outfileName << endl;
  
  TFile* outputFile = TFile::Open(outfileName, "RECREATE");

  TMVA::Factory *factory = new TMVA::Factory( "EIDmva_"+location+"_"+ptrange+"_"+EXTRA, outputFile,
					      "!V:!Silent:Color:DrawProgressBar=False:Transformations=I;D;P;G:AnalysisType=Classification");

  factory->AddVariable("ele_oldsigmaietaieta",'F');
  factory->AddVariable("ele_oldsigmaiphiiphi",'F');
  factory->AddVariable("ele_oldcircularity",'F');
  factory->AddVariable("ele_oldr9",'F');
   
  //
   factory->AddVariable("ele_scletawidth",'F');
   factory->AddVariable("ele_sclphiwidth",'F');
   //
   factory->AddVariable("ele_oldhe",'F');
   //
   // Tracks Variables

   factory->AddVariable("ele_kfhits", 'I');
   factory->AddVariable("ele_kfchi2",'F');    // chi2 KF
   factory->AddVariable("ele_gsfchi2",'F'); // ele_chi2_hits chi2 GSF
   factory->AddVariable("ele_fbrem",'F');

   factory->AddVariable("ele_gsfhits", 'I');
   factory->AddVariable("ele_expected_inner_hits", 'I');
   factory->AddVariable("ele_conversionVertexFitProbability", 'F');

   //
   // E-p matching
   factory->AddVariable("ele_ep",'F');
   factory->AddVariable("ele_eelepout",'F');
   factory->AddVariable("ele_IoEmIop",'F');
   factory->AddVariable("ele_deltaetain",'F');
   factory->AddVariable("ele_deltaphiin",'F');
   factory->AddVariable("ele_deltaetaseed", 'F');
   //factory->AddVariable("ele_pT", 'F');
 
   // New variables
   if(EXTRA.Contains("nsub")>0)  factory->AddVariable("ele_sclNclus",'F');

   
   if(location.Contains("EE")>0) factory->AddVariable("ele_psEoverEraw",'F');
   if(EXTRA.Contains("fbrem")>0 && EXTRA.Contains("Nclus2")>0)   { factory->AddVariable("ele_fbremasym",'F'); }

   // Spectators
   
   factory->AddSpectator("ele_pt"); //, 'F');
   factory->AddSpectator("ele_isEE"); //, 'I');
   factory->AddSpectator("ele_isEB"); //, 'I');
   factory->AddSpectator("ele_isEBEtaGap"); //, 'I');
   factory->AddSpectator("ele_isEBPhiGap"); //, 'I');
   factory->AddSpectator("ele_isEBEEGap"); //, 'I');
   factory->AddSpectator("ele_isEERingGap"); //, 'I');
   factory->AddSpectator("ele_isEEDeeGap"); //, 'I');
   factory->AddSpectator("ele_isEE"); //, 'I');
   factory->AddSpectator("scl_eta"); 
   factory->AddSpectator("ele_eClass");
   //factory->AddSpectator("ele_pfRelIso");
   //factory->AddSpectator("ele_expected_inner_hits");
   //factory->AddSpectator("ele_vtxconv");
   //factory->AddSpectator("mc_event_weight");
   factory->AddSpectator("mc_ele_matchedFromCB");
    
//   factory->AddSpectator("PUweight_ouflow");

   
   // -----------------------------
   //  Input File & Tree
   // -----------------------------
   TString input_signal_name = "/data/DATA/temp_pigard/eID/new_ntup_v1_DY_ext/new_ntup_v1_DY_ext_REG_signal.root";

   TString input_back_name   = "/data/DATA/temp_pigard/eID/new_ntup_v1_DY_ext/new_ntup_v1_DY_ext_REG_background.root";


   
   TFile* inputSignal = TFile::Open(input_signal_name);
   TFile* inputBkg    = TFile::Open(input_back_name);
  
   TTree *signal     = (TTree*)inputSignal->Get("tree");
   TTree *background = (TTree*)inputBkg->Get("tree");
   
  // global event weights per tree (see below for setting event-wise weights)
  Double_t signalWeight     = 1.0;
  Double_t backgroundWeight = 1.0;
  
  // You can add an arbitrary number of signal or background trees
  factory->AddSignalTree    ( signal);
  factory->AddBackgroundTree( background);
 

  // Remember to always set both weights - one does not work! 
  //factory->SetSignalWeightExpression("total_weight");
  //factory->SetBackgroundWeightExpression("mc_event_weight");
 
  // ---------------------------
  //  Training
  // ---------------------------
  // Apply additional cuts on the signal and background samples (can be different)
  TCut addcut1 = "";
  TCut addcut2 = "";
  TCut addcut3 = "";

  if(location=="EB")  addcut1 = " ele_isEB == 1";
  if(location=="EB1") addcut1 = " ele_isEB == 1 && fabs(scl_eta)<0.8" ;
  if(location=="EB2") addcut1 = " ele_isEB == 1 && fabs(scl_eta)>=0.8";
  
  if(location=="EE")  addcut1 =  " ele_isEE == 1";
  if(location=="EE1")  addcut1 = " ele_isEE == 1 && fabs(scl_eta)<2.0";
  if(location=="EE2")  addcut1 = " ele_isEE == 1 && fabs(scl_eta)>=2.0";

  if(ptrange=="5")    addcut2 = " ele_pt > 5 && ele_pt<=10";
  if(ptrange=="10")   addcut2 = " ele_pt > 10";// && ele_pT <=20";
  if(ptrange=="20")   addcut2 = " ele_pt > 20";

  // Split By Ncluster
  if(EXTRA.Contains("Nclus1")>0) addcut3 = " ele_sclNclus == 1";
  if(EXTRA.Contains("Nclus2")>0) addcut3 = " ele_sclNclus > 1";


  TCut mycut = addcut1 + addcut2 + addcut3;


  Int_t sigEvents = 0;
  Int_t bkgEvents = 0;


  if(location=="EB1" && ptrange=="5") {
    sigEvents = 4956;
    bkgEvents = 28078;
  }

  if(location=="EB1" && ptrange=="10") {
    sigEvents = 38743 * 0;
    bkgEvents = 22380;
  }

  if(location=="EB2" && ptrange=="5") {
    sigEvents = 3454;
    bkgEvents = 24818;
  }

  if(location=="EB2" && ptrange=="10") {
    sigEvents = 27402;
    bkgEvents = 23944;
  }

  if(location=="EE" && ptrange=="5") {
    sigEvents = 3207;
    bkgEvents = 33262;
  }

  if(location=="EE" && ptrange=="10") {
    sigEvents = 23861;
    bkgEvents = 30858;
  }

  sigEvents = 200000;
  bkgEvents = 200000;

  TString sigEventsStr, bkgEventsStr;

  sigEventsStr.Form("%i", sigEvents);
  bkgEventsStr.Form("%i", bkgEvents);

  cout << "sigEvents: " << sigEvents << " string: " << sigEventsStr <<endl;
  cout << "bkgEvents: " << bkgEvents << " string: " << bkgEventsStr <<endl;

  TString prepare_nevents = "nTrain_Signal=" + sigEventsStr + ":nTrain_Background=" + bkgEventsStr + ":nTest_Signal=" + sigEventsStr + ":nTest_Background=" + bkgEventsStr +":SplitMode=Random:NormMode=NumEvents:!V";

 
  //TString prepare_nevents = "nTrain_Signal=0:nTrain_Background=0:nTest_Signal=0:nTest_Background=0:SplitMode=Random:NormMode=NumEvents:!V";
  if(EXTRA.Contains("test")>0) prepare_nevents = "nTrain_Signal=1000:nTrain_Background=1000:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V";

  factory->PrepareTrainingAndTestTree(mycut, mycut, 
				      prepare_nevents);

  factory->BookMethod(TMVA::Types::kBDT, "BDT", 
//"!H:!V:NTrees=2000:BoostType=Grad:Shrinkage=0.10:!UseBaggedGrad:nCuts=2000:nEventsMin=100:NNodesMax=5:UseNvars=4:PruneStrength=5:PruneMethod=CostComplexity:MaxDepth=6:CreateMVAPdfs");
"!H:!V:NTrees=2000:BoostType=Grad:Shrinkage=0.10:!UseBaggedGrad:nCuts=2000:MinNodeSize=0.1%:PruneStrength=5:PruneMethod=CostComplexity:MaxDepth=6:CreateMVAPdfs");

  //:NegWeightTreatment=PairNegWeightsGlobal" );

  factory->TrainAllMethods();
  factory->TestAllMethods();
  factory->EvaluateAllMethods();
  
  // Save the output
  outputFile->Close();

  std::cout << "==> Wrote root file: " << outputFile->GetName() << std::endl;
  std::cout << "==> TMVAClassification is done!" << std::endl;

  delete factory;
}
