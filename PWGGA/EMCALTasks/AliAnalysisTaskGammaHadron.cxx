//
// Task to estimate the number of gamma-hadron
// statistic available in the Pb+Pb run.
//
// Author: E. Epple, based on code by  B. Sahlmueller and C. Loizides

#include <Riostream.h>
#include <TClonesArray.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <THnSparse.h>
#include <TList.h>
#include <TLorentzVector.h>

#include "AliAnalysisTaskGammaHadron.h"

#include "AliParticleContainer.h"
#include "AliClusterContainer.h"
#include "AliAnalysisManager.h"
#include "AliCentrality.h"
#include "AliVCluster.h"
#include "AliVParticle.h"
#include "AliVTrack.h"
#include "AliAODTrack.h"
#include "AliVEventHandler.h"
#include "AliInputEventHandler.h"
#include "AliAODEvent.h"
#include "AliExternalTrackParam.h"
#include "AliTrackerBase.h"
#include "AliLog.h"
#include "AliEMCALGeometry.h"
#include "AliEMCALGeoParams.h"
#include "AliPicoTrack.h"
#include "AliVVZERO.h"
#include "AliESDUtils.h"
#include "AliEventPoolManager.h"
#include "AliMCEvent.h"

#include <memory>

using std::cout;
using std::endl;

ClassImp(AliAnalysisTaskGammaHadron)
//
//  This class inherits from AliAnalysisTaskEmcal() -->fcent,fVertex,fNVertCont,fVertexSPD,fNVertSPDCont,fTriggers is defined in there
//  And AliAnalysisTaskEmcal() inherits from AliAnalysisTaskSE()
//________________________________________________________________________
AliAnalysisTaskGammaHadron::AliAnalysisTaskGammaHadron():
AliAnalysisTaskEmcal("AliAnalysisTaskGammaHadron", kTRUE),

fGammaOrPi0(0),fDoMixing(0),fMCorData(0),fDebug(0),fSavePool(0),
fEventCuts(0),fHistEffGamma(0x0),fHistEffHadron(0x0),
fRtoD(0),
fClShapeMin(0),fClShapeMax(10),fMaxNLM(10),fRmvMTrack(0),
fMixBCent(0),fMixBZvtx(),fPoolMgr(0x0),fTrackDepth(0),fPoolSize(0),fEventPoolOutputList(0),
fTriggerType(AliVEvent::kINT7), fMixingEventType(AliVEvent::kINT7),fCurrentEventTrigger(0),
fParticleLevel(kFALSE),fIsMC(kFALSE),
fEventCutList(0),fOutputList1(0),fOutputListTrAs(0),fOutputListGamma(0),fOutputListXi(0),fOutputListZeta(0),

fHistNoClusPt(0),fHistPi0(0),fHistEvsPt(0),fHistClusPairInvarMasspT(0),fHistClusPairInvarMasspTMIX(0),fHistClusPairInvarMasspTMIXolap(0),fMAngle(0),fMAngleMIX(0),fPtAngle(0),fPtAngleMIX(0),fHistBinCheckPt(0),fHistBinCheckZt(0),fHistBinCheckXi(0),

fHistDEtaDPhiGammaQA(0),fHistDEtaDPhiTrackQA(0),
fHistCellsCluster(0),fHistClusterShape(0),fHistClusterShape0(0),fHistClusterShape1(0),fHistClusterShape2(0),fHistClusterShape3(0),fHistClusterShape4(0),fHistClusterTime(0),

//fAODfilterBits(0),fHistptAssHadronG(0),fHistptAssHadronZt(0),fHistptAssHadronXi(0),fHistDEtaDPhiG(0),fHistDEtaDPhiZT(0),fHistDEtaDPhiXI(0)
//fHistptTriggG(),fHistptTriggZt(),fHistptTriggXi(),

 thisEvent(),

fHPoolReady(0x0)
{
	//..Initialize by defult for
	//..AliAnalysisTaskGammaHadron(0,0);
	InitArrays();

	for(int i=0;i<nMulClass;i++){
	  for(int j=0;j<nZClass;j++){
	    for(int k=0;k<nPtClass;k++){
	      iEvt[i][j][k] = 0;
	      for(int l=0;l<nEvt;l++){
		EmcEventList[i][j][k][l].SetGlobalInfo(0,0.,0.);
	      }
	    }
	  }
	}
}
//________________________________________________________________________
AliAnalysisTaskGammaHadron::AliAnalysisTaskGammaHadron(Bool_t InputGammaOrPi0,Bool_t InputDoMixing, Bool_t InputMCorData):
AliAnalysisTaskEmcal("AliAnalysisTaskGammaHadron", kTRUE),

fGammaOrPi0(0),fDoMixing(0),fMCorData(0),fDebug(0),fSavePool(0),
fEventCuts(0),fHistEffGamma(0x0),fHistEffHadron(0x0),
fRtoD(0),
fClShapeMin(0),fClShapeMax(10),fMaxNLM(10),fRmvMTrack(0),
fMixBCent(0),fMixBZvtx(),fPoolMgr(0x0),fTrackDepth(0),fPoolSize(0),fEventPoolOutputList(0),
fTriggerType(AliVEvent::kINT7), fMixingEventType(AliVEvent::kINT7),fCurrentEventTrigger(0),
fParticleLevel(kFALSE),fIsMC(kFALSE),
fEventCutList(0),fOutputList1(0),fOutputListTrAs(0),fOutputListGamma(0),fOutputListXi(0),fOutputListZeta(0),

fHistNoClusPt(0),fHistPi0(0),fHistEvsPt(0),fHistClusPairInvarMasspT(0),fHistClusPairInvarMasspTMIX(0),fHistClusPairInvarMasspTMIXolap(0),fMAngle(0),fMAngleMIX(0),fPtAngle(0),fPtAngleMIX(0),fHistBinCheckPt(0),fHistBinCheckZt(0),fHistBinCheckXi(0),

fHistDEtaDPhiGammaQA(0),fHistDEtaDPhiTrackQA(0),
fHistCellsCluster(0),fHistClusterShape(0),fHistClusterShape0(0),fHistClusterShape1(0),fHistClusterShape2(0),fHistClusterShape3(0),fHistClusterShape4(0),fHistClusterTime(0),

//fAODfilterBits(0),fHistptAssHadronG(0),fHistptAssHadronZt(0),fHistptAssHadronXi(0),fHistDEtaDPhiG(0),fHistDEtaDPhiZT(0),fHistDEtaDPhiXI(0)
//fHistptTriggG(),fHistptTriggZt(),fHistptTriggXi(),
thisEvent(),

fHPoolReady(0x0)
{
	InitArrays();
	//..set input variables
	fGammaOrPi0        =InputGammaOrPi0;
	fDoMixing          =InputDoMixing;
	fMCorData          =InputMCorData;

	for(int i=0;i<nMulClass;i++){
	  for(int j=0;j<nZClass;j++){
	    for(int k=0;k<nPtClass;k++){
	      iEvt[i][j][k] = 0;
	      for(int l=0;l<nEvt;l++){
		EmcEventList[i][j][k][l].SetGlobalInfo(0,0.,0.);
	      }
	    }
	  }
	}
}
//________________________________________________________________________
void AliAnalysisTaskGammaHadron::InitArrays()
{
	//..Initialize by defult for
	//..AliAnalysisTaskGammaHadron(0,1);

	//..set input variables
	fGammaOrPi0        =0; //= 0 ( Gamma analysis ), 1 (pi0 analysis)
	fDoMixing          =0; //= 0 (do only same event analyis with correct triggers), =1 (do event mixing)
	fMCorData          =0; // 0->MC, 1->Data

	fDebug             =0; //set only 1 for debugging
	fSavePool          =0; //= 0 do not save the pool by default. Use the set function to do this.

	//..These two items are set in AliAnalysisTaskEmcal::RetrieveEventObjects()
	//fCent, zVertex

	//..Initialize the arrays to 0
	for(Int_t i=0; i<kNIdentifier;i++)
	{
		fHistptAssHadronG[i] =0;
		fHistptAssHadronZt[i]=0;
		fHistptAssHadronXi[i]=0;
		fHistptTriggG[i] =0;
		fHistptTriggZt[i]=0;
		fHistptTriggXi[i]=0;

		for(Int_t j=0; j<kNIdentifier;j++)
		{
			if(j<kNoGammaBins+1)fHistDEtaDPhiG[i][j]  = 0;
			if(j<kNoZtBins+1)   fHistDEtaDPhiZT[i][j] = 0;
			if(j<kNoXiBins+1)   fHistDEtaDPhiXI[i][j] = 0;
		}
	}

	fRtoD=180.0/TMath::Pi();

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//   Define bins in which the 2D histograms are plotted
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	Double_t fZtStep =1.0/(7-1.0);  // Bin width for the zT histograms
	Double_t fXiStep =2.5/(8-1.0);  // Bin width for the Xi histograms

	Double_t fArray_G_BinsValue[kNoGammaBins+1] ={5,7,9,11,14,17,22,30,60,90};
	Double_t fArray_ZT_BinsValue[kNoZtBins+1]   ={0,fZtStep,2*fZtStep,3*fZtStep,4*fZtStep,5*fZtStep,6*fZtStep,20};
	Double_t fArray_XI_BinsValue[kNoXiBins+1]   ={-100,0,fXiStep,2*fXiStep,3*fXiStep,4*fXiStep,5*fXiStep,6*fXiStep,10};

	memcpy (fArray_G_Bins,  fArray_G_BinsValue,  sizeof (fArray_G_Bins));
	memcpy (fArray_ZT_Bins, fArray_ZT_BinsValue, sizeof (fArray_ZT_Bins));
	memcpy (fArray_XI_Bins, fArray_XI_BinsValue, sizeof (fArray_XI_Bins));
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//   Define vertex and centrality bins for the ME background
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//..if desired one can add a set function to set these values in the add task function
	Double_t centmix[kNcentBins+1] = {0.0, 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 80.0, 100.0};
	fMixBCent = new TAxis(kNcentBins,centmix);

	//static const Int_t NvertBins=8;
	//Double_t zvtxmix[NvertBins+1] = {-10,-6,-4,-2,0,2,4,6,10};
	Double_t zvtxmix[kNvertBins+1] = {-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10};
	memcpy (fArrayNVertBins, zvtxmix, sizeof (fArrayNVertBins));
	fMixBZvtx = new TAxis(kNvertBins,zvtxmix);

	//..Raymond/Megan gives more mixed event yield - don't know about the quality though
	//fTrackDepth     = 100;      //Hanseul sets it to 100! Q:: is this good? Maximum number of tracks??
	fTrackDepth     = 50000;    //Raymonds/Megans value

	//..!!
	//.. fPoolSize is an input that is ignored in the PoolManager Anyway
	fPoolSize       = 1;     //1000 - Raymond/Megan value, says it is ignored anyway

	//..member function of AliAnalysisTaskEmcal
	SetMakeGeneralHistograms(kTRUE);
}
//________________________________________________________________________
AliAnalysisTaskGammaHadron::~AliAnalysisTaskGammaHadron()
{
	// Destructor

	//Copied from chris yaldo. Ask Salvatore about it!
	// Destructor. Clean-up the output list, but not the histograms that are put inside
	// (the list is owner and will clean-up these histograms). Protect in PROOF case.
	if (fOutput && !AliAnalysisManager::GetAnalysisManager()->IsProofMode())
	{
		delete fOutputList1;
	}
	//copied from hanseul
	/*if (fPoolMgr)
	{
		delete fPoolMgr;
	}*/
}
//________________________________________________________________________
void AliAnalysisTaskGammaHadron::UserCreateOutputObjects()
{
	if(fDebug==1)cout<<"Inside of: AliAnalysisTaskGammaHadron::UserCreateOutputObjects()"<<endl;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//   Create fOutput list and histograms (fHistZVertex, fHistEventRejection, fHistEventRejection, fHistEventCount, fHistCentrality, fHistEventPlane)
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	AliAnalysisTaskEmcal::UserCreateOutputObjects();

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//   Add output for AliEventCuts
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	fEventCutList = new TList();
	fEventCutList ->SetOwner();
	fEventCutList ->SetName("EventCutOutput");

	fEventCuts.OverrideAutomaticTriggerSelection(fOffTrigger);
	Bool_t UseManualEventCuts=0;
	if(UseManualEventCuts==1)
	{
		/*
	     This is not possible because these are private functions!
	    //..Enable manual mode.
		//..this just means that the automatic cut settings
	    //..are not loaded every time the event is checked
	    fEventCuts.SetManualMode();
	//..nevertheless we set now the standard settings
	    //..for the respective period and then overwrite
	    //..some cuts with the set values in the Emcal task.
	    fEventCuts.fCurrentRun = fRunNumber;
	    fEventCuts.AutomaticSetup();
		 */
		//..overwrite the manual set cuts with
		//..some of our own values
		fEventCuts.fTriggerMask = fOffTrigger;
		fEventCuts.fMinVtz = fMinVz;
		fEventCuts.fMaxVtz = fMaxVz;
		fEventCuts.fRequireTrackVertex = true;
		fEventCuts.fMaxDeltaSpdTrackAbsolute=fZvertexDiff;
		fEventCuts.fTrackletBGcut = fTklVsClusSPDCut; //(false by default for 15o)
		fEventCuts.fMinCentrality = fMinCent;
		fEventCuts.fMaxCentrality = fMaxCent;
		//++fRejectPileup (IsPileupFromSPD)= true (fixed in code)
		//+remove multi vertexer pile up (false - not activated yet)
		//+spd vertex resolution etc
		//+some cent. resolution cuts
		//+some variable correlatios - fixed to false
	}

	fEventCuts.AddQAplotsToList(fEventCutList);
	fOutput->Add(fEventCutList);
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//   Create mixed event pools
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	if(fDoMixing==1 || fPoolMgr) //do this for either a mixed event analysis or when an external pool is given
	{
		InitEventMixer();
	}
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//   Define sublists/folders for a better organisation of the figures
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	fOutputList1    = new TList();
	fOutputList1    ->SetOwner();
	fOutputList1    ->SetName("pT_distributions_of_the_gamma");
	fOutputListTrAs = new TList();
	fOutputListTrAs ->SetOwner();
	fOutputListTrAs ->SetName("TriggAndAssoc");
	fOutputListGamma= new TList();
	fOutputListGamma->SetOwner();
	fOutputListGamma->SetName("Different_Gamma_2DHistograms");
	fOutputListXi   = new TList();
	fOutputListXi   ->SetOwner();
	fOutputListXi   ->SetName("Different_Xi_2DHistograms");
	fOutputListZeta = new TList();
	fOutputListZeta ->SetOwner();
	fOutputListZeta ->SetName("Different_Zt_2DHistograms");
	fOutputListQA   = new TList();
	fOutputListQA   ->SetOwner();
	fOutputListQA   ->SetName("QA_histograms");

	//common bins for the histograms
	Int_t nbins[6] = {0};
	Double_t min[6] = {0};
	Double_t max[6] = {0};

	//settings for p_t cluster distributon
	nbins[0] = 500;
	min[0] = 0;
	max[0] = 100;
	//settings for p_t hadron distribution
	nbins[1] = 60;  //do 1/2 GeV bins so that you can see the 0.5 cut to set as a minimum pT to combine hadron and gamma
	min[1] = 0;
	max[1] = 30;
	//settings for delta phi (g-h) distribution
	nbins[2] = 50;
	min[2] = -90;
	max[2] = 270;
	//settings for delta eta (g-h) distribution
	nbins[3] = 80;
	min[3] = -2;
	max[3] = 2;
	//settings for phi distribution for QA
	nbins[4] = 76;
	min[4] = -10;
	max[4] = 370;
	//settings for eta distribution for QA
	nbins[5] = 80;
	min[5] = -1;
	max[5] = 1;
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	//   Create Histograms
	//
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//    Histograms for common use
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	//..Initialize
	fHistBinCheckPt       = new TH1*[kNIdentifier];
	fHistBinCheckZt       = new TH1*[kNIdentifier];
	fHistBinCheckXi       = new TH1*[kNIdentifier];
	fHistDEtaDPhiGammaQA  = new TH2*[kNIdentifier+2]; //Why +2??
	fHistDEtaDPhiTrackQA  = new TH2*[kNIdentifier+2];
	fHistCellsCluster     = new TH2*[kNIdentifier+2];
	fHistClusterShape     = new TH2*[kNIdentifier+2];
	fHistClusterShape0     = new TH2*[kNIdentifier+2];
	fHistClusterShape1     = new TH2*[kNIdentifier+2];
	fHistClusterShape2     = new TH2*[kNIdentifier+2];
	fHistClusterShape3     = new TH2*[kNIdentifier+2];
	fHistClusterShape4     = new TH2*[kNIdentifier+2];
	fHistClusterTime      = new TH2*[kNIdentifier+2];

	for(Int_t i=0; i<kNIdentifier; i++)
	{
		fHistptAssHadronG[i] = new TH1*[kNoGammaBins];
		fHistptAssHadronZt[i]= new TH1*[kNoZtBins];
		fHistptAssHadronXi[i]= new TH1*[kNoXiBins];
		fHistptTriggG[i]     = new TH1*[kNoGammaBins];
		fHistptTriggZt[i]    = new TH1*[kNoZtBins];
		fHistptTriggXi[i]    = new TH1*[kNoXiBins];

		for(Int_t j=0; j<kNoGammaBins;j++)
		{
			if(j<kNoGammaBins+1)fHistDEtaDPhiG[i][j] = new TH2*[kNvertBins+1];
			if(j<kNoZtBins+1)   fHistDEtaDPhiZT[i][j]= new TH2*[kNvertBins+1];
			if(j<kNoXiBins+1)   fHistDEtaDPhiXI[i][j]= new TH2*[kNvertBins+1];
		}
	}
	//.................................
	//..p_T^{Cluster} distribution under different conditions

	//..all clusters
	fHistNoClusPt = new TH1F(Form("fHistNoClusPt_Id%0d",1),Form("fHistNoClusPt_Id%0d",1), 31,0, 31);
	fHistNoClusPt->GetXaxis()->SetTitle("p_{T}^{Calo Cluster}");
	fHistNoClusPt->GetYaxis()->SetTitle(Form("No. of Clusters [counts/%0.1f GeV/c]",fHistNoClusPt->GetBinWidth(0)));
	fOutputList1->Add(fHistNoClusPt);

	fHistClusPairInvarMasspT= new TH2F("fHistClusPairInvarMasspT","fHistClusPairInvarMasspT", 500, 0, 0.5, 250, 0, 50);
	fHistClusPairInvarMasspT->GetXaxis()->SetTitle("M_{#gamma#gamma}");
	fHistClusPairInvarMasspT->GetYaxis()->SetTitle("p_{T}_{#pi^{0}}");
	fOutput->Add(fHistClusPairInvarMasspT);

	fHistClusPairInvarMasspTMIX= new TH2F("fHistClusPairInvarMasspTMIX","fHistClusPairInvarMasspTMIX", 500, 0, 0.5, 250, 0, 50);
	fHistClusPairInvarMasspTMIX->GetXaxis()->SetTitle("M_{#gamma#gamma}");
	fHistClusPairInvarMasspTMIX->GetYaxis()->SetTitle("p_{T}_{#pi^{0}}");
	fOutput->Add(fHistClusPairInvarMasspTMIX);

	fHistClusPairInvarMasspTMIXolap= new TH2F("fHistClusPairInvarMasspTMIXolap","fHistClusPairInvarMasspTMIXolap", 500, 0, 0.5, 250, 0, 50);
	fHistClusPairInvarMasspTMIXolap->GetXaxis()->SetTitle("M_{#gamma#gamma}");
	fHistClusPairInvarMasspTMIXolap->GetYaxis()->SetTitle("p_{T}_{#pi^{0}}");
	fOutput->Add(fHistClusPairInvarMasspTMIXolap);

	fMAngleMIX= new TH2F("fMAngleMIX","fMAngleMIX", 2000, 0, 3, 500, 0, 3.14159);
	fMAngleMIX->GetXaxis()->SetTitle("M_{#gamma#gamma}");
	fMAngleMIX->GetYaxis()->SetTitle("Opening Angle [rad]");
	fOutput->Add(fMAngleMIX);

	fMAngle= new TH2F("fMAngle","fMAngle", 2000, 0, 3, 500, 0, 3.14159);
	fMAngle->GetXaxis()->SetTitle("M_{#gamma#gamma}");
	fMAngle->GetYaxis()->SetTitle("Opening Angle [rad]");
	fOutput->Add(fMAngle);

	fPtAngleMIX= new TH2F("fPtAngleMIX","fPtAngleMIX", 250, 0, 50, 500, 0, 3.14159);
	fPtAngleMIX->GetXaxis()->SetTitle("p_{T}_{#pi^{0}}");
	fPtAngleMIX->GetYaxis()->SetTitle("Opening Angle [rad]");
	fOutput->Add(fPtAngleMIX);

	fPtAngle= new TH2F("fPtAngle","fPtAngle", 250, 0, 50, 500, 0, 3.14159);
	fPtAngle->GetXaxis()->SetTitle("p_{T}_{#pi^{0}}");
	fPtAngle->GetYaxis()->SetTitle("Opening Angle [rad]");
	fOutput->Add(fPtAngle);

	//..by the identifier different histograms can be filled under different cut conditions
	//..Can eg. later be modified to contain certain delta phi or centrality bins
	for(Int_t identifier=0;identifier<kNIdentifier;identifier++)
	{
		fHistBinCheckPt[identifier] = new TH1F(Form("fHistBinCheckPt_%0d",identifier),Form("fHistBinCheckPt_%0d",identifier), nbins[0], min[0], max[0]);
		fHistBinCheckPt[identifier]->GetXaxis()->SetTitle("p_{T}^{#gamma}");
		fHistBinCheckPt[identifier]->GetYaxis()->SetTitle("Entries");
		fOutput->Add(fHistBinCheckPt[identifier]);

		fHistBinCheckZt[identifier] = new TH1F(Form("fHistBinCheckZt_%0d",identifier),Form("fHistBinCheckZt_%0d",identifier), 1500, 0, 60);
		fHistBinCheckZt[identifier]->GetXaxis()->SetTitle("z_{T}^{#gamma-h}");
		fHistBinCheckZt[identifier]->GetYaxis()->SetTitle("Entries");
		fOutput->Add(fHistBinCheckZt[identifier]);

		fHistBinCheckXi[identifier] = new TH1F(Form("fHistBinCheckXi_%0d",identifier),Form("fHistBinCheckXi_%0d",identifier), 500, -20, 20);
		fHistBinCheckXi[identifier]->GetXaxis()->SetTitle("#xi^{#gamma-h}");
		fHistBinCheckXi[identifier]->GetYaxis()->SetTitle("Entries");
		fOutput->Add(fHistBinCheckXi[identifier]);

		for(Int_t i=0; i<kNoGammaBins; i++)
		{
			for(Int_t j=0; j<kNvertBins; j++)
			{
				fHistDEtaDPhiG[identifier][i][j] = new TH2F(Form("fHistDEtaDPhiG%d_Id%d_V%d",i,identifier,j),Form("fHistDEtaDPhiG%d_Id%d_V%d",i,identifier,j),nbins[2],min[2],max[2],nbins[3],min[3],max[3]);
				fHistDEtaDPhiG[identifier][i][j]->GetXaxis()->SetTitle(Form("#Delta #varphi^{#gamma-h} %0.1f<p_{T}^{#gamma}<%0.1f",fArray_G_Bins[i],fArray_G_Bins[i+1]));
				fHistDEtaDPhiG[identifier][i][j]->GetYaxis()->SetTitle("#Delta #eta^{#gamma-h}");
				fOutputListGamma->Add(fHistDEtaDPhiG[identifier][i][j]);
			}
			fHistptAssHadronG[identifier][i] = new TH1F(Form("fHistPtAssHadronG%d_Id%d",i,identifier),Form("fHistPtAssHadronG%d_Id%d",i,identifier), nbins[1], min[1], max[1]);
			fHistptAssHadronG[identifier][i]->GetXaxis()->SetTitle(Form("p_{T}^{assoc. h} %0.1f<p_{T}^{#gamma}<%0.1f",fArray_G_Bins[i],fArray_G_Bins[i+1]));
			fHistptAssHadronG[identifier][i]->GetYaxis()->SetTitle(Form("dN^{assoc. h}/dp_{T}^{h} [counts/%0.1f GeV/c]",fHistptAssHadronG[identifier][i]->GetBinWidth(1)));
			fOutputListTrAs->Add(fHistptAssHadronG[identifier][i]);

			fHistptTriggG[identifier][i] = new TH1F(Form("fHistptTriggG%d_Id%d",i,identifier),Form("fHistptTriggG%d_Id%d",i,identifier), nbins[0], min[0], max[0]);
			fHistptTriggG[identifier][i]->GetXaxis()->SetTitle(Form("p_{T}^{cluster} %0.1f<p_{T}^{#gamma}<%0.1f",fArray_G_Bins[i],fArray_G_Bins[i+1]));
			fHistptTriggG[identifier][i]->GetYaxis()->SetTitle(Form("dN^{assoc. h}/dp_{T}^{h} [counts/%0.1f GeV/c]",fHistptAssHadronG[identifier][i]->GetBinWidth(1)));
			fOutputListTrAs->Add(fHistptTriggG[identifier][i]);
		}
		for(Int_t i=0; i<kNoZtBins; i++)
		{
			for(Int_t j=0; j<kNvertBins; j++)
			{
				fHistDEtaDPhiZT[identifier][i][j] = new TH2F(Form("fHistDEtaDPhiZT%d_Id%d_V%d",i,identifier,j),Form("fHistDEtaDPhiZT%d_Id%d_V%d",i,identifier,j),nbins[2],min[2],max[2],nbins[3],min[3],max[3]);
				fHistDEtaDPhiZT[identifier][i][j]->GetXaxis()->SetTitle(Form("#Delta #varphi^{#gamma-h} %0.1f<z_{T}<%0.1f",fArray_ZT_Bins[i],fArray_ZT_Bins[i+1]));
				fHistDEtaDPhiZT[identifier][i][j]->GetYaxis()->SetTitle("#Delta #eta^{#gamma-h}");
				fOutputListZeta->Add(fHistDEtaDPhiZT[identifier][i][j]);
			}
			fHistptAssHadronZt[identifier][i] = new TH1F(Form("fHistPtAssHadronZt%d_Id%d",i,identifier),Form("fHistPtAssHadronZt%d_Id%d",i,identifier), nbins[1], min[1], max[1]);
			fHistptAssHadronZt[identifier][i]->GetXaxis()->SetTitle(Form("p_{T}^{assoc. h} %0.1f<z_{T}<%0.1f",fArray_ZT_Bins[i],fArray_ZT_Bins[i+1]));
			fHistptAssHadronZt[identifier][i]->GetYaxis()->SetTitle(Form("dN^{assoc. h}/dp_{T}^{h} [counts/%0.1f GeV/c]",fHistptAssHadronZt[identifier][i]->GetBinWidth(1)));
			fOutputListTrAs->Add(fHistptAssHadronZt[identifier][i]);

			fHistptTriggZt[identifier][i] = new TH1F(Form("fHistptTriggZt%d_Id%d",i,identifier),Form("fHistptTriggZt%d_Id%d",i,identifier), nbins[0], min[0], max[0]);
			fHistptTriggZt[identifier][i]->GetXaxis()->SetTitle(Form("p_{T}^{cluster} %0.1f<z_{T}<%0.1f",fArray_ZT_Bins[i],fArray_ZT_Bins[i+1]));
			fHistptTriggZt[identifier][i]->GetYaxis()->SetTitle(Form("dN^{assoc. h}/dp_{T}^{h} [counts/%0.1f GeV/c]",fHistptTriggZt[identifier][i]->GetBinWidth(1)));
			fOutputListTrAs->Add(fHistptTriggZt[identifier][i]);
		}
		for(Int_t i=0; i<kNoXiBins; i++)
		{
			for(Int_t j=0; j<kNvertBins; j++)
			{
				fHistDEtaDPhiXI[identifier][i][j] = new TH2F(Form("fHistDEtaDPhiXI%d_Id%d_V%d",i,identifier,j),Form("fHistDEtaDPhiXI%d_Id%d_V%d",i,identifier,j),nbins[2],min[2],max[2],nbins[3],min[3],max[3]);
				fHistDEtaDPhiXI[identifier][i][j]->GetXaxis()->SetTitle(Form("#Delta #varphi^{#gamma-h} %0.1f<#xi<%0.1f",fArray_XI_Bins[i],fArray_XI_Bins[i+1]));
				fHistDEtaDPhiXI[identifier][i][j]->GetYaxis()->SetTitle("#Delta #eta^{#gamma-h}");
				fOutputListXi->Add(fHistDEtaDPhiXI[identifier][i][j]);
			}
			fHistptAssHadronXi[identifier][i] = new TH1F(Form("fHistPtAssHadronXi%d_Id%d",i,identifier),Form("fHistPtAssHadronXi%d_Id%d",i,identifier), nbins[1], min[1], max[1]);
			fHistptAssHadronXi[identifier][i]->GetXaxis()->SetTitle(Form("p_{T}^{assoc. h} %0.1f<#xi<%0.1f",fArray_XI_Bins[i],fArray_XI_Bins[i+1]));
			fHistptAssHadronXi[identifier][i]->GetYaxis()->SetTitle(Form("dN^{assoc. h}/dp_{T}^{h} [counts/%0.1f GeV/c]",fHistptAssHadronXi[identifier][i]->GetBinWidth(1)));
			fOutputListTrAs->Add(fHistptAssHadronXi[identifier][i]);

			fHistptTriggXi[identifier][i] = new TH1F(Form("fHistptTriggXi%d_Id%d",i,identifier),Form("fHistptTriggXi%d_Id%d",i,identifier), nbins[0], min[0], max[0]);
			fHistptTriggXi[identifier][i]->GetXaxis()->SetTitle(Form("p_{T}^{cluster} %0.1f<#xi<%0.1f",fArray_XI_Bins[i],fArray_XI_Bins[i+1]));
			fHistptTriggXi[identifier][i]->GetYaxis()->SetTitle(Form("dN^{assoc. h}/dp_{T}^{h} [counts/%0.1f GeV/c]",fHistptTriggXi[identifier][i]->GetBinWidth(1)));
			fOutputListTrAs->Add(fHistptTriggXi[identifier][i]);
		}
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//   Tyler's Special Histograms
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//   Eliane's Special Histograms
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	fHistEvsPt = new TH2F("fHistEvsPt","fHistEvsPt", nbins[0], min[0], max[0], 250, 0, 50);
	fHistEvsPt->GetXaxis()->SetTitle("p_{T}_{#gamma}");
	fHistEvsPt->GetYaxis()->SetTitle("E_{#gamma}");
	fOutput->Add(fHistEvsPt);

	//test!!
	fHistPi0 = new TH1F(Form("fHistPi0_%0d",1),Form("fHistPi0_%0d",1), 500, 0, 0.5);
	fHistPi0->GetXaxis()->SetTitle("M_{#gamma#gamma}");
	fHistPi0->GetYaxis()->SetTitle("Entries");
	fOutput->Add(fHistPi0);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//   Special QA histograms (also to get more info what is going on in mixed event for trigger data)
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	for(Int_t identifier=0;identifier<kNIdentifier+2;identifier++)
	{
		//..geometrical hit distribution of clusters
		fHistDEtaDPhiGammaQA[identifier] = new TH2F(Form("fHistDEtaDPhiGammaQA%d_Id%d",0,identifier),Form("fHistDEtaDPhiGammaQA%d_Id%d",0,identifier),nbins[4],min[4],max[4],nbins[5],min[5],max[5]);
		fHistDEtaDPhiGammaQA[identifier]->GetXaxis()->SetTitle("#varphi^{#gamma}");
		fHistDEtaDPhiGammaQA[identifier]->GetYaxis()->SetTitle("#eta^{#gamma}");
		fOutputListQA->Add(fHistDEtaDPhiGammaQA[identifier]);

		//..geometrical hit distribution of tracks
		fHistDEtaDPhiTrackQA[identifier] = new TH2F(Form("fHistDEtaDPhiTrackQA%d_Id%d",0,identifier),Form("fHistDEtaDPhiTrackQA%d_Id%d",0,identifier),nbins[4],min[4],max[4],nbins[5],min[5],max[5]);
		fHistDEtaDPhiTrackQA[identifier]->GetXaxis()->SetTitle("#varphi^{hadron}");
		fHistDEtaDPhiTrackQA[identifier]->GetYaxis()->SetTitle("#eta^{hadron}");
		fOutputListQA->Add(fHistDEtaDPhiTrackQA[identifier]);

	    //..Cluster Info
		fHistCellsCluster[identifier] = new TH2F(Form("fHistCellsCluster%d_Id%d",0,identifier),Form("fHistCellsCluster%d_Id%d",0,identifier),240,0,30,50,0,50);
		fHistCellsCluster[identifier]->GetXaxis()->SetTitle("E^{cluster}");
		fHistCellsCluster[identifier]->GetYaxis()->SetTitle("N_{cells}");
		fOutputListQA->Add(fHistCellsCluster[identifier]);

		fHistClusterShape[identifier] = new TH2F(Form("fHistClusterShape%d_Id%d",0,identifier),Form("fHistClusterShape%d_Id%d",0,identifier),240,0,30,750,0,4);
		fHistClusterShape[identifier]->GetXaxis()->SetTitle("E^{cluster}");
		fHistClusterShape[identifier]->GetYaxis()->SetTitle("#lambda_{0}");
		fOutputListQA->Add(fHistClusterShape[identifier]);

		fHistClusterShape0[identifier] = new TH2F(Form("fHistClusterShapeA%d_Id%d",0,identifier),Form("fHistClusterShapeA%d_Id%d",0,identifier),240,0,30,750,0,4);
		fHistClusterShape0[identifier]->GetXaxis()->SetTitle("E^{cluster}");
		fHistClusterShape0[identifier]->GetYaxis()->SetTitle("#lambda_{0}");
		fOutputListQA->Add(fHistClusterShape0[identifier]);

		fHistClusterShape1[identifier] = new TH2F(Form("fHistClusterShapeB%d_Id%d",0,identifier),Form("fHistClusterShapeB%d_Id%d",0,identifier),240,0,30,750,0,4);
		fHistClusterShape1[identifier]->GetXaxis()->SetTitle("E^{cluster}");
		fHistClusterShape1[identifier]->GetYaxis()->SetTitle("#lambda_{0}");
		fOutputListQA->Add(fHistClusterShape1[identifier]);

		fHistClusterShape2[identifier] = new TH2F(Form("fHistClusterShapeC%d_Id%d",0,identifier),Form("fHistClusterShapeC%d_Id%d",0,identifier),240,0,30,750,0,4);
		fHistClusterShape2[identifier]->GetXaxis()->SetTitle("E^{cluster}");
		fHistClusterShape2[identifier]->GetYaxis()->SetTitle("#lambda_{0}");
		fOutputListQA->Add(fHistClusterShape2[identifier]);

		fHistClusterShape3[identifier] = new TH2F(Form("fHistClusterShapeD%d_Id%d",0,identifier),Form("fHistClusterShapeD%d_Id%d",0,identifier),240,0,30,750,0,4);
		fHistClusterShape3[identifier]->GetXaxis()->SetTitle("E^{cluster}");
		fHistClusterShape3[identifier]->GetYaxis()->SetTitle("#lambda_{0}");
		fOutputListQA->Add(fHistClusterShape3[identifier]);

		fHistClusterShape4[identifier] = new TH2F(Form("fHistClusterShapeE%d_Id%d",0,identifier),Form("fHistClusterShapeE%d_Id%d",0,identifier),240,0,30,750,0,4);
		fHistClusterShape4[identifier]->GetXaxis()->SetTitle("E^{cluster}");
		fHistClusterShape4[identifier]->GetYaxis()->SetTitle("#lambda_{0}");
		fOutputListQA->Add(fHistClusterShape4[identifier]);

		//..Time information
		fHistClusterTime[identifier] = new TH2F(Form("fHistClusterTime%d_Id%d",0,identifier),Form("fHistClusterTime%d_Id%d",0,identifier),20000,-100,100,200,0,40);
		fHistClusterTime[identifier]->GetXaxis()->SetTitle("time [ns]");
		fHistClusterTime[identifier]->GetYaxis()->SetTitle("pT");
		fOutputListQA->Add(fHistClusterTime[identifier]);

		//..
	}



	//..The END
	fOutput->Add(fOutputList1);
	fOutput->Add(fOutputListTrAs);
	fOutput->Add(fOutputListGamma);
	fOutput->Add(fOutputListZeta);
	fOutput->Add(fOutputListXi);
	fOutput->Add(fOutputListQA);

	PostData(1, fOutput); // Post data for ALL output slots >0 here, to get at least an empty histogram
}
//________________________________________________________________________
void AliAnalysisTaskGammaHadron::InitEventMixer()
{
	if(fDebug==1)cout<<"Inside of: AliAnalysisTaskGammaHadron::InitEventMixer()"<<endl;
	//--The effective pool size in events is set by trackDepth, so more
	//--low-mult events are required to maintain the threshold than
	//--high-mult events. Centrality pools are indep. of data histogram
	//--binning, no need to match.

	//..Centrality pools
	Int_t nCentBins=fMixBCent->GetNbins();
	Double_t centBins[nCentBins+1];
	centBins[0] = fMixBCent->GetBinLowEdge(1);
	for(Int_t i=1; i<=nCentBins; i++)
	{
		centBins[i] = fMixBCent->GetBinUpEdge(i);
	}

	//..Z-vertex pools
	Int_t nZvtxBins=fMixBZvtx->GetNbins();
	Double_t zvtxbin[nZvtxBins+1];
	zvtxbin[0] = fMixBZvtx->GetBinLowEdge(1);
	for(Int_t i=1; i<=nZvtxBins; i++)
	{
		zvtxbin[i] = fMixBZvtx->GetBinUpEdge(i);
	}

	//..in case no external pool is provided create one here
	if(!fPoolMgr)
	{
		cout<<"....  Pool Manager Created ...."<<endl;
		fPoolMgr = new AliEventPoolManager(fPoolSize, fTrackDepth, nCentBins, centBins, nZvtxBins, zvtxbin);
		fPoolMgr->SetTargetValues(fTrackDepth, 0.1, 5);  //pool is ready at 0.1*fTrackDepth = 5000 or events =5
		//save this pool by default
	}
	else
	{
		//..lock all pools
		//..clears empty pools and sets them locked
		//..(is only possible because all save flags are ture in my case  - NASTY NASTY)
		fPoolMgr->ClearPools();
		cout<<"....  Pool Manager Provided From File ...."<<endl;
	}

	//..Check binning of pool manager (basic dimensional check for the time being) to see whether external pool fits the here desired one??
	if( (fPoolMgr->GetNumberOfMultBins() != nCentBins) || (fPoolMgr->GetNumberOfZVtxBins() != nZvtxBins) )
	{
		AliFatal("Binning of given pool manager not compatible with binning of correlation task!");
	}
	//if you want to save the pool:
	// If some bins of the pool should be saved, fEventPoolOutputList must be given
	// using AddEventPoolToOutput() (to increase size of fEventPoolOutputList)
	// Note that this is in principle also possible, if an external poolmanager was given
//	if(fEventPoolOutputList.size())

	if(fSavePool==1)
	{
		//? is this an option to save only specific pools instead of the full pool manager?
		//for(Int_t i = 0; i < fEventPoolOutputList.size(); i++)
		{
			/*Double_t minCent = fEventPoolOutputList[i][0];
			Double_t maxCent = fEventPoolOutputList[i][1];
			Double_t minZvtx = fEventPoolOutputList[i][2];
			Double_t maxZvtx = fEventPoolOutputList[i][3];
			Double_t minPt   = fEventPoolOutputList[i][4];
			Double_t maxPt   = fEventPoolOutputList[i][5];
            */
		    //If the pool fulfills the given criteria the saveflag is set to true
			//the flag is used in the ClearPools function to not delete the pool content
			fPoolMgr->SetSaveFlag(-1, 10000, -10000, 100000, 0, 0, -1, 10000000);

			/*
			In case you don't want to store all pools but only the ones specified above
			you have to rund at the very end of filling these lines.

			// Clear unnecessary pools before saving and locks the pool
			fPoolMgr->ClearPools();
			*/
		}
		fOutput->Add(fPoolMgr);
	}

	//..Basic checks and printing of pool properties
	fPoolMgr->Validate();
}
//____________________________________________________________________
void AliAnalysisTaskGammaHadron::AddEventPoolsToOutput(Double_t minCent, Double_t maxCent,  Double_t minZvtx, Double_t maxZvtx, Double_t minPt, Double_t maxPt)
{
	//..This allows you to add only specific pools and not the full pool manager to the output file
	std::vector<Double_t> binVec;
	binVec.push_back(minCent);
	binVec.push_back(maxCent);
	binVec.push_back(minZvtx);
	binVec.push_back(maxZvtx);
	binVec.push_back(minPt);
	binVec.push_back(maxPt);
	fEventPoolOutputList.push_back(binVec);
}
//________________________________________________________________________
void AliAnalysisTaskGammaHadron::ExecOnce()
{
	if(fDebug==1)cout<<"Inside of: AliAnalysisTaskGammaHadron::ExecOnce()"<<endl;

	//..This function does...
	AliAnalysisTaskEmcal::ExecOnce();

}
///Overwrites the AliAnalysisTaskEmcal::IsEventSelected()
///function
///
//________________________________________________________________________
Bool_t AliAnalysisTaskGammaHadron::IsEventSelected()
{
	//..checks: rejects DAQ incompletes, magnetic field selection, offline trigger
	//..vertex selection, SPD pile-up (if enabled), centrality cuts
	if (!fEventCuts.AcceptEvent(InputEvent()))
	{
		PostData(1, fOutput);
		return kFALSE;
	}

	//ELIANE for the moment...
	//..check that they are the same
	if(fCent!=fEventCuts.GetCentrality())	  cout<<"Difference in centralities -> classic centr.: "<<fCent<<", new centr from evt cuts: "<<fEventCuts.GetCentrality()<<endl; /// Centrality calculated with the default estimator (V0M for LHC15o)
	if(fVertex[2]!=fEventCuts.GetPrimaryVertex()->GetZ())  cout<<"Difference in centralities -> classic vtx.: "<<fVertex[2]<<", new vertex from evt cuts: "<<fEventCuts.GetPrimaryVertex()->GetZ()<<endl; /// Centrality calculated with the default estimator (V0M for LHC15o)


	//.. .. .. ..
	//..Start of copy part from AliAnalysisTaskEmcal
	if (!fTrigClass.IsNull())
	{
		TString fired;

		const AliAODEvent *aev = dynamic_cast<const AliAODEvent*>(InputEvent());
		if (aev)
		{
			fired = aev->GetFiredTriggerClasses();
		}
		else cout<<"Error analysis only for AODs"<<endl;

		if (!fired.Contains("-B-"))
		{
			if (fGeneralHistograms) fHistEventRejection->Fill("trigger",1);
			return kFALSE;
		}

		std::unique_ptr<TObjArray> arr(fTrigClass.Tokenize("|"));
		if (!arr)
		{
			if (fGeneralHistograms) fHistEventRejection->Fill("trigger",1);
			return kFALSE;
		}
		Bool_t match = 0;
		for (Int_t i=0;i<arr->GetEntriesFast();++i)
		{
			TObject *obj = arr->At(i);
			if (!obj)
				continue;

			//Check if requested trigger was fired
			TString objStr = obj->GetName();
			if(fEMCalTriggerMode == kOverlapWithLowThreshold &&
					(objStr.Contains("J1") || objStr.Contains("J2") || objStr.Contains("G1") || objStr.Contains("G2"))) {
				// This is relevant for EMCal triggers with 2 thresholds
				// If the kOverlapWithLowThreshold was requested than the overlap between the two triggers goes with the lower threshold trigger
				TString trigType1 = "J1";
				TString trigType2 = "J2";
				if(objStr.Contains("G"))
				{
					trigType1 = "G1";
					trigType2 = "G2";
				}
				if(objStr.Contains(trigType2) && fired.Contains(trigType2.Data()))
				{ //requesting low threshold + overlap
					match = 1;
					break;
				}
				else if(objStr.Contains(trigType1) && fired.Contains(trigType1.Data()) && !fired.Contains(trigType2.Data())) { //high threshold only
					match = 1;
					break;
				}
			}
			else
			{
				// If this is not an EMCal trigger, or no particular treatment of EMCal triggers was requested,
				// simply check that the trigger was fired
				if (fired.Contains(obj->GetName()))
				{
					match = 1;
					break;
				}
			}
		}
		if (!match)
		{
			if (fGeneralHistograms) fHistEventRejection->Fill("trigger",1);
			return kFALSE;
		}
	}
	if (fTriggerTypeSel != kND)
	{
		if (!HasTriggerType(fTriggerTypeSel))
		{
			if (fGeneralHistograms) fHistEventRejection->Fill("trigTypeSel",1);
			return kFALSE;
		}
	}
    /*
	//.. Maybe these two as well .. .. ..
	if (fSelectPtHardBin != -999 && fSelectPtHardBin != fPtHardBin)  {
		if (fGeneralHistograms) fHistEventRejection->Fill("SelPtHardBin",1);
		return kFALSE;
	}

	// Reject filter for MC data
	if (!CheckMCOutliers()) return kFALSE;
    */
	//..End of copy part from AliAnalysisTaskEmcal
	//.. .. .. ..

	//..call it explicitly to check
	//..whether additional event cuts are applied that should be included
	//..should be DELETED!!! later
	AliAnalysisTaskEmcal::IsEventSelected();

	return kTRUE;
}
//________________________________________________________________________
Bool_t AliAnalysisTaskGammaHadron::Run()
{
	//..This function is called in AliAnalysisTaskEmcal::UserExec.
	if(fDebug==1)cout<<"Inside of: AliAnalysisTaskGammaHadron::Run()"<<endl;
	//..Determine the trigger for the current event
	fCurrentEventTrigger = ((AliInputEventHandler*)(AliAnalysisManager::GetAnalysisManager()->GetInputEventHandler()))->IsEventSelected();

	//..check here some basic properties of the event
	//is centrality and zvertex in range? - see if this is actually done in IsSelected in the EMCal Task

	/*if(fCurrentEventTrigger & fTriggerType)
	{
		if(fCurrentEventTrigger & fMixingEventType)cout<<"*********************contains both triggers!!"<<endl;
	}*/

	//..for same event only analyse events when there is a cluster inside
	//..and when the event has the correct trigger
	if (fDoMixing==0 && !fCaloClusters)                         return kFALSE;
	if (fDoMixing==0 && !(fCurrentEventTrigger & fTriggerType)) return kFALSE;

	return kTRUE;
}
//________________________________________________________________________
Bool_t AliAnalysisTaskGammaHadron::FillHistograms()
{
	//..This function is called in AliAnalysisTaskEmcal::UserExec.
	if(fDebug==1)cout<<"Inside of: AliAnalysisTaskGammaHadron::FillHistograms()"<<endl;

	// 1. First get an event pool corresponding in mult (cent) and
	//    zvertex to the current event. Once initialized, the pool
	//    should contain nMix (reduced) events. This routine does not
	//    pre-scan the chain. The first several events of every chain
	//    will be skipped until the needed pools are filled to the
	//    specified depth. If the pool categories are not too rare, this
	//    should not be a problem. If they are rare, you could lose
	//    statistics.

	// 2. Collect the whole pool's content of tracks into one TObjArray
	//    (bgTracks), which is effectively a single background super-event.

	// 3. The reduced and bgTracks arrays must both be passed into
	//    FillCorrelations(). Also nMix should be passed in, so a weight
	//    of 1./nMix can be applied.

	//..Get pool containing tracks from other events like this one
	Double_t zVertex = fVertex[2];
	AliParticleContainer* tracks =0x0;
	tracks   = GetParticleContainer(0);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//    Mixed event section
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	if(fDoMixing==1)
	{
		AliEventPool* pool = 0x0;
		pool = fPoolMgr->GetEventPool(fCent, zVertex);
		if (!pool)
		{
			AliWarning(Form("No pool found. Centrality %f, ZVertex %f",fCent, zVertex));
			return kFALSE;
		}

        //. . . . . . . . . . . . .
		//..Start combining triggers (from fTriggerType events)
		//..with a pool filled with tracks (from fMixingEventType)
		if(pool->IsReady() && (fCurrentEventTrigger & fTriggerType))
		{
			//..get number of current events in pool
			Int_t nMix = pool->GetCurrentNEvents();

			//cout<<"number of events in pool: "<<nMix<<endl;
			for(Int_t jMix=0; jMix<nMix; jMix++)
			{
				TObjArray* bgTracks=0x0;
				bgTracks = pool->GetEvent(jMix);

				if(!bgTracks)
				{
					cout<<"could not retrieve TObjArray from EventPool!"<<endl;
				}
				//..Loop over clusters and fill histograms
				if(fGammaOrPi0==0) CorrelateClusterAndTrack(0,bgTracks,0,1.0/nMix);//correlate with mixed event
				else               CorrelatePi0AndTrack(0,bgTracks,0,1.0/nMix);    //correlate with mixed event
			}
		}
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		//    Update the pool
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		//..update pool only with tracks from event type fMixingEventType,
		//.. and do NOT add tracks from GA triggered events (BIT(15))
		if((fCurrentEventTrigger & fMixingEventType) && ((fCurrentEventTrigger & 1000000000000000)==0))
		{
			TObjArray* tracksClone=0x0;
			tracksClone = CloneToCreateTObjArray(tracks);

			//..if there is no track object or the pool is locked do not update
			if(tracksClone && !pool->GetLockFlag())
			{
				pool->UpdatePool(tracksClone);
			}
		}
	}
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//    Same event section
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//..Loop over clusters and fill histograms
	//..Do this only for events that are of fTriggerType
	if(fCurrentEventTrigger & fTriggerType)
	{
		if(fGammaOrPi0==0) CorrelateClusterAndTrack(tracks,0,1,1);//correlate with same event
		else               CorrelatePi0AndTrack(tracks,0,1,1);    //correlate with same event
	}



	                                                   //<<<<<><<<<<<<<<><<<<<<<<<<><<<<<<<<<<<<><<<<<<<<<<<<<<><<<<<<<<<<<<<<<<<<<<<>
	AliVCluster* cluster = 0;

	AliClusterContainer* clusters  = GetClusterContainer(0);  //how do I know which cells are selected
	if (!clusters) return 0;
	Int_t NoOfClustersInEvent =clusters->GetNClusters();

	// if(thisEvent.nHits>0){
	//   for(Int_t nua = 0; nua< thisEvent.nHits; nua++){
	//     if(thisEvent.hit[nua].NCells>0){
	//       for(Int_t jho=0; jho<thisEvent.hit[nua].NCells; jho++){
	// 	thisEvent.hit[nua].CellIDArray[jho]=0;
	//       }
	//     }
	//   }
	// }
	 thisEvent.Reset();
	Int_t NAccClus=0;
	Float_t Max_Phi=0;
	Float_t Max_Eta=0;

	for(Int_t NoCluster0 = 0; NoCluster0 < NoOfClustersInEvent; NoCluster0++ )
	  {
	    cluster=(AliVCluster*) clusters->GetAcceptCluster(NoCluster0); //->GetCluster(NoCluster1);
	    if(!cluster || !AccClusterForAna(clusters,cluster) || cluster->E()<=2)continue; //check if the cluster is a good cluster
	    NAccClus++;
	  }
	thisEvent.SetGlobalInfo(NAccClus,Max_Phi,Max_Eta);

Int_t AccClus=0;
	for(Int_t NoCluster1 = 0; NoCluster1 < NoOfClustersInEvent; NoCluster1++ )
	  {
	  cluster=(AliVCluster*) clusters->GetAcceptCluster(NoCluster1); //->GetCluster(NoCluster1);
	  if(!cluster || !AccClusterForAna(clusters,cluster) || cluster->E()<=2)continue; //check if the cluster is a good cluster
	  AccClus++;
	    TLorentzVector CaloClusterVec;
	    clusters->GetMomentum(CaloClusterVec, cluster);
	    thisEvent.hit[AccClus-1].thishit=CaloClusterVec;
	    thisEvent.hit[AccClus-1].NCells=cluster->GetNCells();

	    for(Int_t c = 0; c<cluster->GetNCells(); c++){
	      thisEvent.hit[AccClus-1].CellRay.push_back(cluster->GetCellsAbsId()[c]);
	      //std::cout<<cluster->GetCellsAbsId()[c]<<" vector size: "<<thisEvent.hit[AccClus-1].CellRay.size()<<" front: "<<thisEvent.hit[AccClus-1].CellRay.front()<<" back: "<<thisEvent.hit[AccClus-1].CellRay.back()<<std::endl;
	      }

	    // for(int n : thisEvent.hit[AccClus-1].CellRay){
	    //   std::cout<<"compare: "<<n<<std::endl;
	    // }
	    // thisEvent.hit[AccClus-1].SetCellIDArray(cluster->GetCellsAbsId());
	    //thisEvent.hit[AccClus-1].CellIDArray=cluster->GetCellsAbsId();

	    //std::cout<<Form("Cluster:%f,%f,%f,%f,%f", CaloClusterVec.M(), CaloClusterVec.P(), CaloClusterVec.Eta(),CaloClusterVec.Phi(), CaloClusterVec.E() )<<std::endl;
	    //thisEvent.hit[NoOfClustersInEvent-1].hittype=100;
	    //thisEvent.hit[NoOfClustersInEvent-1].weight=1.;
	    //thisEvent.hit[NoOfClustersInEvent-1].imo=1;
	    //thisEvent.hit[NoOfClustersInEvent-1].smno=1; //incorrect! could add modno function
	  }
	//thisEvent.Print();  //QA

	// TLorentzVector clusterVec1;
	// for (Int_t i = 0; i<NAccClus; ++i) {
	//   clusterVec1 = thisEvent.hit[i].thishit;
	//   std::cout<<Form("ClusterREAD: %f,%f,%f,%f,%f", clusterVec1.M(), clusterVec1.P(), clusterVec1.Eta(),clusterVec1.Phi(), clusterVec1.E() )<<std::endl;
	// }



	 Int_t nclusthis = thisEvent.nHits;
	 //std::cout<<Form("Summary of Hits: %d, %d", NoOfClustersInEvent, nclusthis)<<std::endl;


	Int_t vtxClass = 1;
	Int_t MulClass = 4;

	//////////GetMulClassPi0(MulClass);

	Float_t phitrig = 0;
	Float_t thetatrig = 0;
	Double_t pt_max = 0;
	Int_t ptClass = 0;

	//if(MulClass < nMulClass && vtxClass < nZClass && ptClass < nPtClass){
	AddMixEventPi0(MulClass, vtxClass, ptClass, iEvt[MulClass][vtxClass][ptClass], Max_Phi, Max_Eta);
	  //	}
	  //	else{
	  // AliWarning("event outside defined classes");
	  //	}

                                                    //<<<<<><<<<<<<<<><<<<<<<<<<><<<<<<<<<<<<><<<<<<<<<<<<<<><<<<<<<<<<<<<<<<<<<<<>

	if( fMCorData==0)	ProcessMC();

	return kTRUE;
}
//________________________________________________________________________
TObjArray* AliAnalysisTaskGammaHadron::CloneToCreateTObjArray(AliParticleContainer* tracks)
{
	if(fDebug==1)cout<<"Inside of: AliAnalysisTaskGammaHadron::CloneToCreateTObjArray()"<<endl;
	//..clones a track list
	if(!tracks)                            return 0;
	if(tracks->GetNAcceptedParticles()==0) return 0;

	TObjArray* tracksClone = new TObjArray;
	tracksClone->SetOwner(kTRUE);

	Int_t NoOfTracksInEvent =tracks->GetNParticles();
	AliVParticle* track=0;

	for(Int_t NoTrack = 0; NoTrack < NoOfTracksInEvent; NoTrack++)
	{
		track = (AliVParticle*)tracks->GetAcceptParticle(NoTrack);
		if(!track)continue; //check if the track is a good track
		//tracksClone->Add((AliVParticle*)track);  //only add accepted tracks
		tracksClone->Add(new AliPicoTrack(track->Pt(), track->Eta(), track->Phi(), track->Charge(), 0, 0, 0, 0));
	}

	if(tracksClone->GetEntries()!=tracks->GetNAcceptedParticles())cout<<"!!!!!!! Major error!!!! "<<"Accepted tracks in event: "<<tracks->GetNAcceptedParticles()<<", Tracks in TObjArray: "<<tracksClone->GetEntries()<<endl;

	return tracksClone;
}
//________________________________________________________________________
Int_t AliAnalysisTaskGammaHadron::CorrelateClusterAndTrack(AliParticleContainer* tracks,TObjArray* bgTracksArray,Bool_t SameMix, Double_t InputWeight)
{
	if(fDebug==1)cout<<"Inside of: AliAnalysisTaskGammaHadron::CorrelateClusterAndTrack()"<<endl;

	//...........................................
	//..Do cluster loop.
	AliClusterContainer* clusters  = GetClusterContainer(0);  //how do I know which cells are selected
	if (!clusters) return 0;
	Int_t NoOfClustersInEvent =clusters->GetNClusters();
	Double_t EffWeight_Gamma;
	Double_t EffWeight_Hadron;
	Double_t Weight=1;    //weight to normalize mixed and same event distributions individually

	AliVCluster* cluster = 0;
	AliVCluster* cluster2= 0;
	AliVParticle* trackNULL=0;

	//...........................................
	//..for mixed events normalize per events in pool
	if(SameMix==0)
	{
		Weight=InputWeight;
	}
	//..for same events normalize later by counting the entries in spec. norm histograms
	if(SameMix==1)
	{
		Weight=1;
	}

	//...........................................
	//..run the loop for filling the histograms
	Int_t GammaCounter=0;
	for(Int_t NoCluster1 = 0; NoCluster1 < NoOfClustersInEvent; NoCluster1++ )
	{
		cluster=(AliVCluster*) clusters->GetAcceptCluster(NoCluster1); //->GetCluster(NoCluster1);
		if(!cluster)continue; //check if the cluster is a good cluster
		//if(!cluster)continue; //check if the cluster is a good cluster
		//clusters->GetLeadingCluster("e");

		TLorentzVector CaloClusterVec;
		clusters->GetMomentum(CaloClusterVec, cluster);
		AliTLorentzVector aliCaloClusterVec = AliTLorentzVector(CaloClusterVec); //..can acess phi from
		EffWeight_Gamma=GetEff(aliCaloClusterVec);

		//------------------------------------------------
        //..This section is for the moment to test
		//..cluster distributions without cuts
		//..switch off NLM and cluster shape for
		//..the moment
		Double_t minTemp=fClShapeMin;
		Double_t maxTemp=fClShapeMax;
		Int_t    nlmTem =fMaxNLM;
		SetM02(0,6);
		SetNLM(5);
		if(!AccClusterForAna(clusters,cluster))
		{
			SetM02(minTemp,maxTemp);
			SetNLM(nlmTem);
			continue; //check if the cluster is a good cluster
		}
		FillQAHisograms(0,clusters,cluster,trackNULL);

		//..switch it back on
		//..
		//cout<<"nlm tmp: ="<<nlmTem<<", minTemp:"<<minTemp<<", maxTpm: "<<maxTemp<<endl;
		SetM02(minTemp,maxTemp);
		SetNLM(nlmTem);
		if(!AccClusterForAna(clusters,cluster))continue; //check if the cluster is a good cluster
		//------------------------------------------------


		fHistNoClusPt->Fill(CaloClusterVec.Pt()); //the .pt only works for gammas (E=M) for other particle this is wrong
		fHistEvsPt->Fill(CaloClusterVec.Pt(),CaloClusterVec.E());

        //cout<<"Cluster eta: "<<CaloClusterVec.Eta()<<", Cluster phi:"<<aliCaloClusterVec.Phi_0_2pi()*fRtoD<<", Cluster E:"<<cluster->E()<<endl;

		//cout<<"Cluster number: "<<NoCluster1<<", pT = "<<CaloClusterVec.Pt()<<endl;
		//...........................................
		//..combine gammas with same event tracks
		GammaCounter++;
		if(SameMix==1)
		{
			if(!tracks)  return 0;
			Int_t NoOfTracksInEvent =tracks->GetNParticles();
			AliVParticle* track=0;

			Int_t trackCounter=0;
			for(Int_t NoTrack = 0; NoTrack < NoOfTracksInEvent; NoTrack++)
			{
				track = (AliVParticle*)tracks->GetAcceptParticle(NoTrack);
				if(!track)continue; //check if the track is a good track
				trackCounter++;
				//cout<<"..Track number: "<<NoTrack<<", pT = "<<track->Pt()<<endl;

				//EffWeight_Hadron=GetEff(<TLorentzVector>track);
				FillGhHisograms(1,aliCaloClusterVec,track,5,0,Weight);
				FillGhHisograms(2,aliCaloClusterVec,track,10,0,Weight);

				if(GammaCounter==1)FillQAHisograms(1,clusters,cluster,track); //fill only once per track (first gamma) - good for each track
				if(trackCounter==1)FillQAHisograms(2,clusters,cluster,track); //fill only once per gamma (first track) - good for gamma distr.
			}
			//...........................................
			//..double cluster loop for testing an anti pi0 cut
			for( Int_t NoCluster2 = 0; NoCluster2 < NoOfClustersInEvent; NoCluster2++ )
			{
				if(NoCluster1!=NoCluster2)
				{
					cluster2=(AliVCluster*) clusters->GetAcceptCluster(NoCluster2);
					if(!cluster2 || !AccClusterForAna(clusters,cluster2))continue; //check if the cluster is a good cluster

					TLorentzVector CaloClusterVec2;
					TLorentzVector CaloClusterVecpi0;
					clusters->GetMomentum(CaloClusterVec2, cluster2);
					if(cluster2->E()>2 && cluster->E()>2)
					{
						CaloClusterVecpi0=CaloClusterVec+CaloClusterVec2;
						fHistPi0->Fill(CaloClusterVecpi0.M());
						fHistClusPairInvarMasspT->Fill(CaloClusterVecpi0.M(),CaloClusterVecpi0.Pt());
					}
				}
			}
		}
		//...........................................
		//..combine gammas with mixed event tracks
		if(SameMix==0)
		{
			Int_t Nbgtrks = bgTracksArray->GetEntries();
			for(Int_t ibg=0; ibg<Nbgtrks; ibg++)
			{
				AliPicoTrack* track = static_cast<AliPicoTrack*>(bgTracksArray->At(ibg));
				if(!track) continue;

				//EffWeight_Hadron=GetEff(<TLorentzVector>track);
				FillGhHisograms(0,aliCaloClusterVec,track,5,0,Weight);
			}
		}

	}
	return GammaCounter;
}
// //<<<<<><<<<<<<<<><<<<<<<<<<><<<<<<<<<<<<><<<<<<<<<<<<<<><<<<<<<<<<<<<<<<<<<<<>
TObjArray* AliAnalysisTaskGammaHadron::CloneClustersTObjArray(AliClusterContainer* clusters)
{
	if(fDebug==1)cout<<"Inside of: AliAnalysisTaskGammaHadron::CloneClustersTObjArray()"<<endl;
	//..clones a track list
	if(!clusters)                            return 0;
	if(clusters->GetNClusters()==0) return 0;

	TObjArray* clustersCloneI = new TObjArray;
	clustersCloneI->SetOwner(kTRUE);

	Int_t NoOfClustersInEvent =clusters->GetNClusters();
	AliVCluster* cluster = 0;

	for(Int_t NoClus = 0; NoClus < NoOfClustersInEvent; NoClus++)
	{
		cluster = (AliVCluster*) clusters->GetAcceptCluster(NoClus);
		if(!cluster)continue; //check if the Cluster is good
		//tracksClone->Add((AliVParticle*)track);  //only add accepted tracks
		clustersCloneI->Add((AliVCluster*)cluster);// Add(new AliPicoTrack(track->Pt(), track->Eta(), track->Phi(), track->Charge(), 0, 0, 0, 0));
	}

	if(clustersCloneI->GetEntries()!=clusters->GetNAcceptedClusters())cout<<"!!!!!!! Major error!!!! "<<"Accepted clusters in event: "<<clusters->GetNAcceptedClusters()<<", Tracks in TObjArray: "<<clustersCloneI->GetEntries()<<endl;

	return clustersCloneI;
}
//<<<<<><<<<<<<<<><<<<<<<<<<><<<<<<<<<<<<><<<<<<<<<<<<<<><<<<<<<<<<<<<<<<<<<<<>
 void AliAnalysisTaskGammaHadron::GetMulClassPi0(Int_t& imcl)
 {
   Int_t nclus = 0;
AliClusterContainer* clustersCont  = GetClusterContainer(0);
 if (!clustersCont) return;
TObjArray* clustersClone=0x0;
clustersClone = CloneClustersTObjArray(clustersCont);
 TObjArray *clusters =clustersClone;// fEsdClusters;
 // if (!clusters)   clusters = fAodClusters;

   if (clusters)
    nclus = clusters->GetEntries();

//   //const int MultCut[8] = {5, 15, 30, 50, 80, 120, 300, 9999};
   const int MultCut[nMulClass] = {5, 12, 20 ,50, 9999};

   imcl=0;

   for (imcl=0; imcl<nMulClass; imcl++) {
     if (nclus < MultCut[imcl]) break;
   }
 }

//<<<<<><<<<<<<<<><<<<<<<<<<><<<<<<<<<<<<><<<<<<<<<<<<<<><<<<<<<<<<<<<<<<<<<<<>
void AliAnalysisTaskGammaHadron::AddMixEventPi0(const Int_t MulClass, const Int_t vtxClass, const Int_t PtClass, Int_t& iEvent, const Float_t& phitrig, const Float_t& thetatrig)
{
  if(iEvent >= nEvt){
    iEvent = 0;
  }

  Double_t vertex[3] = {0};
  InputEvent()->GetPrimaryVertex()->GetXYZ(vertex);

AliClusterContainer* clustersCont  = GetClusterContainer(0);
 if (!clustersCont) return;
TObjArray* clustersClone=0x0;
clustersClone = CloneClustersTObjArray(clustersCont);
 TObjArray *clusters =clustersClone;

  // TObjArray *clusters = fEsdClusters;
  // if (!clusters)
  //   clusters = fAodClusters;

 if (!clusters) return;
  Int_t nclus = 0;
  nclus = clusters->GetEntries();
  nclus = thisEvent.nHits;

  // if(nclus > evt.nMaxHit){
  //   AliWarning("event has more clusters than nMaxHit!");
  //   //    nclus = evt.nMaxHit;
  // }


  //cout << Form("%d, %d, %d, %d",MulClass,vtxClass,PtClass,iEvent) << endl;
  thisEvent.SetGlobalInfo(nclus,phitrig,thetatrig);
  EmcEventList[MulClass][vtxClass][PtClass][iEvent] = thisEvent;

  iEvent++;
  return;
}
//<<<<<><<<<<<<<<><<<<<<<<<<><<<<<<<<<<<<><<<<<<<<<<<<<<><<<<<<<<<<<<<<<<<<<<<>
void AliAnalysisTaskGammaHadron::ProcessMC()
{
  std::cout<<" MCWHEEEEEEEEEEEEE! "<<std::endl;
   if (fMCorData==1)
    return;

AliMCEvent *mcEvent = MCEvent();
  if (!mcEvent){
    cout << "no MC event" << endl;
    return;
  }

  // get vertex
  const AliVVertex *evtVtx = mcEvent->GetPrimaryVertex();
  if (!evtVtx)
    return;

    // read event
  mcEvent->PreReadAll();

   // get number of MC particles
  Int_t nTracks = mcEvent->GetNumberOfPrimaries();

    std::cout<<" nTracks "<<nTracks<<std::endl;

    // loop through MC particles
  for (Int_t iTrack = 0; iTrack<nTracks; ++iTrack) {
    // get particle at index iTrack
    AliMCParticle *mcP = static_cast<AliMCParticle*>(mcEvent->GetTrack(iTrack));
    if (!mcP)
      continue;

    std::cout<<" pdg "<<mcP->PdgCode()<<std::endl;

     // pion or eta meson or direct photon
    if(mcP->PdgCode() == 111) {
    } else if(mcP->PdgCode() == 221) {
    } else if(mcP->PdgCode() == 22 ) {
    } else
      continue;

     Double_t dR = TMath::Sqrt((mcP->Xv()-evtVtx->GetX())*(mcP->Xv()-evtVtx->GetX()) +
                              (mcP->Yv()-evtVtx->GetY())*(mcP->Yv()-evtVtx->GetY()));
    if(dR > 0.1)
      continue;


  }


}
//<<<<<><<<<<<<<<><<<<<<<<<<><<<<<<<<<<<<><<<<<<<<<<<<<<><<<<<<<<<<<<<<<<<<<<<>
Int_t AliAnalysisTaskGammaHadron::CorrelatePi0AndTrack(AliParticleContainer* tracks,TObjArray* bgTracksArray,Bool_t SameMix, Double_t InputWeight)
{
  if(fDebug==1)cout<<"Inside of: AliAnalysisTaskGammaHadron::CorrelatePi0AndTrack()"<<endl;


	//**This is Tyler's world
	//**initially copied from CorrelateClusterAndTrack
	//**ready to be modified and used for something fabulous

	//...........................................
	//--Do cluster loop.
	AliClusterContainer* clusters  = GetClusterContainer(0);  //how do I know which cells are selected
	if (!clusters) return 0;
	Int_t NoOfClustersInEvent =clusters->GetNClusters();
	Int_t nAccClusters = 0;
	Int_t nAccPi0Clusters = 0;
	Double_t Pi0Mass = 0.13487;
	Double_t Pi0Window = 0.02;//0.03 //can eventually modulate this based on pT of Pi0 candidate!
	Double_t EffWeight_Gamma;
	Double_t EffWeight_Hadron;
	Double_t Weight;    //weight to normalize mixed and same event distributions individually

	AliVCluster* cluster = 0;
	AliVCluster* cluster2= 0;
	AliVParticle* trackNULL=0;

	TLorentzVector OldClusterVec;               //<<<<<><<<<<<<<<><<<<<<<<<<><<<<<<<<<<<<><<<<<<<<<<<<<<><<<<<<<<<<<<<<<<<<<<<>
	TLorentzVector Pi0MixVec;
	Int_t vtxClass=1;
	Int_t PtClass=0;
	Int_t MulClass = 4;
	Int_t olapswitch=0;
	//////////GetMulClassPi0(MulClass);
	//std::cout<<"    "<<MulClass<<std::endl;   //<<<<<><<<<<<<<<><<<<<<<<<<><<<<<<<<<<<<><<<<<<<<<<<<<<><<<<<<<<<<<<<<<<<<<<<>


	//...........................................
	//do a small loop to count the triggers in this event
    //** we don't need this loop here any longer because we will
	//**normalize later in the analysis not now so we don't need the
	//** total pi0 count beforehand!!!!!
	if(SameMix==1)
	{
		for(Int_t NoCluster1 = 0; NoCluster1 < NoOfClustersInEvent; NoCluster1++ )
		{
			cluster=(AliVCluster*) clusters->GetAcceptCluster(NoCluster1); //->GetCluster(NoCluster1);
			if(!cluster || !AccClusterForAna(clusters,cluster))continue; //check if the cluster is a good cluster
			//clusters->GetLeadingCluster("e");

			TLorentzVector CaloClusterVec;
			//old framework			cluster->GetMomentum(CaloClusterVec, fVertex);
			clusters->GetMomentum(CaloClusterVec, cluster);
			//acc if pi0 candidate
			nAccClusters++;

			for (Int_t iOld=0;iOld<nEvt;iOld++){  //<<<<<><<<<<<<<<><<<<<<<<<<><<<<<<<<<<<<><<<<<<<<<<<<<<><<<<<<<<<<<<<<<<<<<<<>
			  EmcEventPi0 OldEvent = EmcEventList[MulClass][vtxClass][PtClass][iOld];
			  Int_t nclusold = OldEvent.nHits;
			  for (Int_t iii = 0; iii<nclusold; ++iii) {
			    OldClusterVec = OldEvent.hit[iii].thishit;

			    olapswitch = 0;
			    for(Int_t c = 0; c<cluster->GetNCells(); c++){
			      for(int n : OldEvent.hit[iii].CellRay){
				if(cluster->GetCellsAbsId()[c]==n){
				  olapswitch++;
				}
			      }
			    }

			    Pi0MixVec= OldClusterVec + CaloClusterVec;
			    if(cluster->E()>2){
			      if(olapswitch>0){
			      fHistClusPairInvarMasspTMIXolap->Fill(Pi0MixVec.M(),Pi0MixVec.Pt());
			      }else if(olapswitch==0){
				fHistClusPairInvarMasspTMIX->Fill(Pi0MixVec.M(),Pi0MixVec.Pt());
				fMAngleMIX->Fill(Pi0MixVec.M(), CaloClusterVec.Angle(OldClusterVec.Vect()));
				fPtAngleMIX->Fill(Pi0MixVec.Pt(), CaloClusterVec.Angle(OldClusterVec.Vect()));
			      }
			    }
			  }
			  //if(nAccClusters==1) std::cout<<Form("%d, %d", iOld, nclusold)<<std::endl;
			}                                      //<<<<<><<<<<<<<<><<<<<<<<<<><<<<<<<<<<<<><<<<<<<<<<<<<<><<<<<<<<<<<<<<<<<<<<<>



			for(Int_t NoCluster2 = 0; NoCluster2 < NoOfClustersInEvent; NoCluster2++ )
			{
				if(NoCluster1!=NoCluster2)
				{
					cluster2=(AliVCluster*) clusters->GetAcceptCluster(NoCluster2);
					if(!cluster2 || !AccClusterForAna(clusters,cluster2))continue; //check if the cluster is a good cluster

					TLorentzVector CaloClusterVec2;
					TLorentzVector CaloClusterVecpi0;

					//old framework				cluster2->GetMomentum(CaloClusterVec2, fVertex);
					clusters->GetMomentum(CaloClusterVec2, cluster2);
					if(cluster2->E()>2 && cluster->E()>2)
					{
						CaloClusterVecpi0=CaloClusterVec+CaloClusterVec2;
						fHistPi0->Fill(CaloClusterVecpi0.M());
						fHistClusPairInvarMasspT->Fill(CaloClusterVecpi0.M(),CaloClusterVecpi0.Pt(),0.5);  //eventually divide by 2
						fMAngle->Fill(CaloClusterVecpi0.M(), CaloClusterVec.Angle(CaloClusterVec2.Vect()),0.5);
						fPtAngle->Fill(CaloClusterVecpi0.Pt(), CaloClusterVec.Angle(CaloClusterVec2.Vect()),0.5);
						if((CaloClusterVecpi0.M()>=Pi0Mass-Pi0Window) && (CaloClusterVecpi0.M()<=Pi0Mass+Pi0Window)){
							nAccPi0Clusters++; //need eventually to divide by 2, otherwise double counting the pi0's
						}
					}
				}
			}
		}
	}

	//...........................................
	//..for mixed events normalize per events in pool
	if(SameMix==0)
	{
		Weight=InputWeight;
	}
	//..for same events normalize later by counting the entries in spec. norm histograms
	if(SameMix==1)
	{
		Weight=1;
	}
	//...........................................
	//run the loop for filling the histograms
	for( Int_t NoCluster1 = 0; NoCluster1 < NoOfClustersInEvent; NoCluster1++ )
	{
		cluster=(AliVCluster*) clusters->GetAcceptCluster(NoCluster1); //->GetCluster(NoCluster1);
		if(!cluster || !AccClusterForAna(clusters,cluster))continue; //check if the cluster is a good cluster
		//clusters->GetLeadingCluster("e");

		TLorentzVector CaloClusterVec;
		//old framework	cluster->GetMomentum(CaloClusterVec, fVertex);
		clusters->GetMomentum(CaloClusterVec,cluster);
		AliTLorentzVector aliCaloClusterVec = AliTLorentzVector(CaloClusterVec); //..can acess phi from

		FillQAHisograms(0,clusters,cluster,trackNULL);

		for( Int_t NoCluster2 = 0; NoCluster2 < NoOfClustersInEvent; NoCluster2++ )
		{
			if(NoCluster1!=NoCluster2)
			{
				cluster2=(AliVCluster*) clusters->GetAcceptCluster(NoCluster2);
				if(!cluster2 || !AccClusterForAna(clusters,cluster2))continue; //check if the cluster is a good cluster

				TLorentzVector CaloClusterVec2;
				TLorentzVector aliCaloClusterVecpi0;
				//old framework			cluster2->GetMomentum(CaloClusterVec2, fVertex);
				clusters->GetMomentum(CaloClusterVec2,cluster2); /// Vec+=2 2.1.17
				AliTLorentzVector aliCaloClusterVec2 = AliTLorentzVector(CaloClusterVec2); //..can acess phi from

				if(cluster2->E()>2 && cluster->E()>2)
				{
					aliCaloClusterVecpi0=aliCaloClusterVec+aliCaloClusterVec2;

					if((aliCaloClusterVecpi0.M()<Pi0Mass-Pi0Window) || (aliCaloClusterVecpi0.M()>Pi0Mass+Pi0Window)) continue; /// 2.1.17

					//here I don't really know what to do in your case
					//eff of pi0? or eff of gamma? or some mix up of the two ?
					EffWeight_Gamma=GetEff(aliCaloClusterVecpi0);//currently just assigns 1!!! need eventually to input Pi0 efficiency histogram

					fHistNoClusPt->Fill(aliCaloClusterVecpi0.Pt()); //the .pt only works for gammas (E=M) for other particle this is wrong

					//...........................................
					//..combine gammas with same event tracks
					if(SameMix==1)
					{
						//cout<<"SameMix==1"<<endl;
						if(!tracks)  return 0;
						Int_t NoOfTracksInEvent =tracks->GetNParticles();
						AliVParticle* track=0;

						for(Int_t NoTrack = 0; NoTrack < NoOfTracksInEvent; NoTrack++)
						{
							track = (AliVParticle*)tracks->GetAcceptParticle(NoTrack);
							if(!track)continue; //check if the track is a good track

							//..fill here eventually a pi0 four-vector instead of CaloClusterVec
							//EffWeight_Hadron=GetEff(TLorentzVector)track);
							FillGhHisograms(1,aliCaloClusterVecpi0,track,5,0,Weight);
							FillGhHisograms(2,aliCaloClusterVecpi0,track,10,0,Weight);
						}
					}
					//...........................................
					//..combine gammas with mixed event tracks
					if(SameMix==0)
					{
						Int_t Nbgtrks = bgTracksArray->GetEntries();
						for(Int_t ibg=0; ibg<Nbgtrks; ibg++)
						{
							AliPicoTrack* track = static_cast<AliPicoTrack*>(bgTracksArray->At(ibg));
							if(!track) continue;

							//**fill here eventually a pi0 four-vector instead of CaloClusterVec
							//EffWeight_Hadron=GetEff((TLorentzVector)track);
							FillGhHisograms(0,aliCaloClusterVecpi0,track,5,0,Weight);
						}
					}
				}
			}
		}
	}
	return nAccPi0Clusters/2;
}
//________________________________________________________________________
void AliAnalysisTaskGammaHadron::FillGhHisograms(Int_t identifier,AliTLorentzVector ClusterVec,AliVParticle* TrackVec, Double_t ClusterEcut, Double_t TrackPcut, Double_t Weight)
{
	if(fDebug==1)cout<<"Inside of: AliAnalysisTaskGammaHadron::FillGhHisograms()"<<endl;

	//..This function fills several histograms under different cut conditions.
	//..it is run within a cluster{ track{}} loop to get all combinations.

	//..A word to the weight - for mixed events it devides by the number of events in the current pool 1/nEvents
	//..                     - for same events you devide by the number of triggers
	//..                     - for both you have to take into account the efficiency of your correlated pair
	Double_t deltaEta   = ClusterVec.Eta()-TrackVec->Eta();
	Double_t deltaPhi   = DeltaPhi(ClusterVec,TrackVec);
	Double_t G_PT_Value = ClusterVec.Pt();
	//Double_t ZT_Value   = TMath::Cos(deltaPhi*1/fRtoD)*TrackVec->P()/ClusterVec.P(); //   TrackVec->Pt()/G_PT_Value;
	Double_t ZT_Value   = TrackVec->Pt()/G_PT_Value; //   TrackVec->Pt()/G_PT_Value;
	//..Careful here: usually this is done for an opening angle (hadron-jet axis) of less than 90�. Due to
	//..resolution momentum smearing (our guess - check that!) there are particles appearing at angles greater than 90�
	Double_t XI_Value=-50;
	if(ZT_Value>0)
	{
		XI_Value   = TMath::Log(1.0/ZT_Value);
	}
	Double_t zVertex = fVertex[2];

	if(G_PT_Value>=ClusterEcut && TrackVec->Pt()>=TrackPcut)
	{
		//..Histograms to test the binning
		fHistBinCheckPt[identifier] ->Fill(G_PT_Value,Weight);
		fHistBinCheckZt[identifier] ->Fill(ZT_Value,Weight);
		fHistBinCheckXi[identifier] ->Fill(XI_Value,Weight);

		//..Fill 2D Histograms for certain event conditions
		for(Int_t i=0;i<10;i++)
		{
			for(Int_t j=0;j<kNvertBins;j++)
			{
				if(i<kNoGammaBins  && G_PT_Value>=fArray_G_Bins[i] && G_PT_Value<fArray_G_Bins[i+1])
				{
					if(zVertex>=fArrayNVertBins[j] && zVertex<fArrayNVertBins[j+1])fHistDEtaDPhiG[identifier][i][j]->Fill(deltaPhi,deltaEta,Weight);
					if(j==0)fHistptAssHadronG[identifier][i]->Fill(TrackVec->Pt(),Weight);
					if(j==0)fHistptTriggG[identifier][i]    ->Fill(G_PT_Value,Weight);
				}
				if(i<kNoZtBins && ZT_Value>=fArray_ZT_Bins[i]  && ZT_Value<fArray_ZT_Bins[i+1])
				{
					if(zVertex>=fArrayNVertBins[j] && zVertex<fArrayNVertBins[j+1])fHistDEtaDPhiZT[identifier][i][j]   ->Fill(deltaPhi,deltaEta,Weight);
					if(j==0)fHistptAssHadronZt[identifier][i]->Fill(TrackVec->Pt(),Weight);
					if(j==0)fHistptTriggZt[identifier][i]    ->Fill(G_PT_Value,Weight);
				}
				if(i<kNoXiBins && XI_Value>=fArray_XI_Bins[i]  && XI_Value<fArray_XI_Bins[i+1])
				{
					if(zVertex>=fArrayNVertBins[j] && zVertex<fArrayNVertBins[j+1])fHistDEtaDPhiXI[identifier][i][j]   ->Fill(deltaPhi,deltaEta,Weight);
					if(j==0)fHistptAssHadronXi[identifier][i]->Fill(TrackVec->Pt(),Weight);
					if(j==0)fHistptTriggXi[identifier][i]    ->Fill(G_PT_Value,Weight);
				}
			}
		}
	}
}
//________________________________________________________________________
void AliAnalysisTaskGammaHadron::FillQAHisograms(Int_t identifier,AliClusterContainer* clusters,AliVCluster* caloCluster,AliVParticle* TrackVec)
{
	TLorentzVector caloClusterVec;
	clusters->GetMomentum(caloClusterVec,caloCluster);
	AliTLorentzVector aliCaloClusterVec = AliTLorentzVector(caloClusterVec); //..can acess phi from

	/*do similar test here?*/fHistDEtaDPhiGammaQA[identifier] ->Fill(aliCaloClusterVec.Phi_0_2pi()*fRtoD,caloClusterVec.Eta());
	if(TrackVec)             fHistDEtaDPhiTrackQA[identifier] ->Fill(TrackVec->Phi()*fRtoD,TrackVec->Eta());

	fHistCellsCluster[identifier] ->Fill(caloCluster->GetHadCorrEnergy(),caloCluster->GetNCells());
	fHistClusterShape[identifier] ->Fill(caloCluster->GetHadCorrEnergy(),caloCluster->GetM02());
	if(caloCluster->GetNExMax()==0)fHistClusterShape0[identifier] ->Fill(caloCluster->GetHadCorrEnergy(),caloCluster->GetM02());
	if(caloCluster->GetNExMax()==1)fHistClusterShape1[identifier] ->Fill(caloCluster->GetHadCorrEnergy(),caloCluster->GetM02());
	if(caloCluster->GetNExMax()==2)fHistClusterShape2[identifier] ->Fill(caloCluster->GetHadCorrEnergy(),caloCluster->GetM02());
	if(caloCluster->GetNExMax()==3)fHistClusterShape3[identifier] ->Fill(caloCluster->GetHadCorrEnergy(),caloCluster->GetM02());
	if(caloCluster->GetNExMax()==4)fHistClusterShape4[identifier] ->Fill(caloCluster->GetHadCorrEnergy(),caloCluster->GetM02());
	fHistClusterTime[identifier]  ->Fill(caloCluster->GetTOF()*1000000000,caloCluster->GetHadCorrEnergy());
}
//
// Accept cluster for analysis. More cuts besides in ApplyClusterCuts and ApplyKinematicCuts
//
//
//________________________________________________________________________
Bool_t AliAnalysisTaskGammaHadron::AccClusterForAna(AliClusterContainer* clusters, AliVCluster* caloCluster)
{
	TLorentzVector caloClusterVec;
	clusters->GetMomentum(caloClusterVec,caloCluster);
	Double_t deltaPhi=2;   //..phi away from detector edges.
	Double_t deltaEta=0.0; //..eta away from detector edges.
    //!!!! eventually transform to AliTLorentzvector

	//..Accepts clusters if certain conditions are fulfilled
	Bool_t Accepted=1; //..By default accepted

	//!!double check these cuts carefully with the experts!!
	//-----------------------------
	//..at least 2 cells in cluster
	if(caloCluster->GetNCells()<2)
	{
		//..Reject the cluster as a good candidate for your analysis
		return 0;
	}
	//-----------------------------
	//..number of local maxima should be 1 or 0 (for cluster splitting this has to be changed)
	if(caloCluster->GetNExMax()>fMaxNLM)
	{
		//..Reject the cluster as a good candidate for your analysis
		return 0;
	}
	//-----------------------------
	//..cut on the cluster shape
	if(fClShapeMin>0 && fClShapeMax>0
	   && (caloCluster->GetM02()<fClShapeMin || caloCluster->GetM02()>fClShapeMax))
	{
		//..Reject the cluster as a good candidate for your analysis
		return 0;
	}
	//-----------------------------
	//..remove clusters with a matched track
	if(fRmvMTrack==1 && caloCluster->GetNTracksMatched()!=0)
	{
		return 0;
	}

	//-----------------------------
	//..Do we need a distance to bad channel cut?
	//caloCluster->GetDistanceToBadChannel()


	//-----------------------------
	//..Do fiducial volume cut



	return Accepted;
}
//________________________________________________________________________
Double_t AliAnalysisTaskGammaHadron::DeltaPhi(AliTLorentzVector ClusterVec,AliVParticle* TrackVec)
{
	Double_t Phi_g = ClusterVec.Phi_0_2pi();
	Double_t Phi_h = TrackVec->Phi();

	Double_t dPhi = -999;
	Double_t pi = TMath::Pi();

	dPhi = Phi_g-Phi_h;
	//--shift the second peak over the fist peak: \--�--/   --> -�--
	//--to create a histogram that starts at -pi/2 and ends at 3/2pi
	if (dPhi <= -TMath::Pi()/2)    dPhi += 2*pi;
	if (dPhi > 3.0*TMath::Pi()/2.0)dPhi -= 2*pi;

	//--change from rad to degree:
	dPhi*= fRtoD;

	return dPhi;
}
//________________________________________________________________________
Double_t AliAnalysisTaskGammaHadron::GetEff(AliTLorentzVector ClusterVec)
{
	Double_t DetectionEff=1;

	/*
	 *
	 * Do something here with the input efficiency histograms
	 *
  THnF                      *fHistEffGamma;            // input efficiency for trigger particles
  THnF                      *fHistEffHadron;           // input efficiency for associate particles
	 *
	 *
	 */


	return DetectionEff;
}
//the end




//__________________________________________________________________________________________________
EmcEventPi0::EmcEventPi0()
: nHits(0),
TrigPhi(0),
TrigTheta(0)
{
  nHits = 0;
  TrigPhi = 0;
  TrigTheta = 0;
}

//__________________________________________________________________________________________________
EmcEventPi0::EmcEventPi0(const EmcEventPi0 &obj)
: nHits(0),
TrigPhi(0),
TrigTheta(0)
{
  nHits = obj.nHits;
  TrigPhi = obj.TrigPhi;
  TrigTheta = obj.TrigTheta;
  // copy all hits
  for(int i=0;i<nHits;i++){
    hit[i].hittype = obj.hit[i].hittype;
    hit[i].imo = obj.hit[i].imo;
    hit[i].pid = obj.hit[i].pid;
    hit[i].weight = obj.hit[i].weight;
    hit[i].thishit = obj.hit[i].thishit;
    hit[i].NCells = obj.hit[i].NCells;
    hit[i].smno = obj.hit[i].smno;
    hit[i].CellRay = obj.hit[i].CellRay;
  }
}


//__________________________________________________________________________________________________
void EmcEventPi0::SetGlobalInfo(const Int_t& Size, const Float_t& phiTrig, const Float_t& thetaTrig)
{
  //    fCenPercent = centPer;
  //    fVtx = vtxPos;
  nHits = Size;
  TrigPhi = phiTrig;
  TrigTheta = thetaTrig;
}

// void EmcHitPi0::SetCellIDArray(const UShort_t *idarray)
// {
// *CellIDArray=*idarray;
// }

void EmcEventPi0::Print()
{
  Printf("%d hits",nHits);
  for(int i=0;i<nHits;i++){
    hit[i].thishit.Print();
  }
}

//__________________________________________________________________________________________________
void EmcEventPi0::Reset()
{
  for(int i=0;i<nHits;i++){
    hit[i].hittype = 0;
    hit[i].imo = 0;
    hit[i].pid = 0;
    hit[i].weight = 1;
    hit[i].smno = 0;
    TLorentzVector lv(0,0,0,0);
    hit[i].thishit = lv;
    hit[i].NCells = 0;
    hit[i].CellRay.clear();
   // if(!hit[i].NCells) continue;
    // for(Int_t Kam = 0; Kam < hit[i].NCells; Kam++){
    //   if(!hit[i].CellIDArray[Kam]) continue;
    // 	hit[i].CellIDArray[Kam]=0;
    // }
    //    hit[i].CellIDArray = 0x0;
  }
  nHits = 0;
  //    fCenPercent = -1;
  //    fVtx = -9999;
}

//__________________________________________________________________________________________________
EmcHitPi0::EmcHitPi0()
  : thishit(),
    hittype(0),
    imo(0),
    pid(0),
    weight(1),
    bclean(1),
    NCells(0),
    CellRay()
{
}
