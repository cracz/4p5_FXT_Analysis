// This is needed for calling standalone classes (not needed on RACF)
//#define _VANILLA_ROOT_

// C++ headers
#include <iostream>
#include <vector>

// ROOT headers
#include "TROOT.h"
#include "TObject.h"
#include "TFile.h"
#include "TChain.h"
#include "TH1.h"
#include "TH2.h"
#include "TString.h"

// PicoDst headers
#include "StRoot/StPicoEvent/StPicoDstReader.h"
#include "StRoot/StPicoEvent/StPicoDst.h"
#include "StRoot/StPicoEvent/StPicoEvent.h"
#include "StRoot/StPicoEvent/StPicoTrack.h"
#include "StRoot/StPicoEvent/StPicoBTofHit.h"
#include "StRoot/StPicoEvent/StPicoBTowHit.h"
#include "StRoot/StPicoEvent/StPicoEmcTrigger.h"
#include "StRoot/StPicoEvent/StPicoBTofPidTraits.h"
//#include "StRoot/StPicoEvent/StPicoTrackCovMatrix.h"
//#include "StPicoEvent/StPicoPhysicalHelix.h"
//#include "StRoot/StPicoEvent/StPicoHelix.h"


// Load libraries (for ROOT_VERSTION_CODE >= 393215)
//#if ROOT_VERSION_CODE >= ROOT_VERSION(6,0,0)
//R__LOAD_LIBRARY(StPicoEvent/libStPicoDst)
//#endif

// inFile - is a name of name.picoDst.root file or a name
//          of a name.lis(t) files that contains a list of
//          name1.picoDst.root files


//Functions
Double_t rapidity(Double_t px, Double_t py, Double_t pz, Double_t mass);
Double_t transMass(Double_t px, Double_t py, Double_t mass);
void fillRawSpect(Double_t px, Double_t py, Double_t pz, Double_t mass, TH1D *dndy, TH1D *dndm, TH2D *MvsY);



//Main
void picoAnalyzer(TString inFile, TString outFile)
{
  std::cout << "Initializing..." << std::endl;

  StPicoDstReader* picoReader = new StPicoDstReader(inFile);
  picoReader->Init();
  picoReader->SetStatus("*",0);
  picoReader->SetStatus("Event",1);
  picoReader->SetStatus("Track",1);
  picoReader->SetStatus("BTofHit",1);
  picoReader->SetStatus("BTofPidTraits",1);

  if (!picoReader->chain()) { std::cout << "No chain found." << std::endl; }

  Long64_t eventsInTree = picoReader->tree()->GetEntries();
  Long64_t events2read  = picoReader->chain()->GetEntries();
    
  std::cout << "Number of events to read: " << events2read << std::endl;
    
  outFile.Append(".picoDst.result.root");
  TFile *outputFile = new TFile(outFile,"recreate");


  /* Histogramming and analysis */
  std::cout << "Starting analysis..." << std::endl;

  double d_xvtx = -9999.0;
  double d_yvtx = -9999.0;
  double d_zvtx = -9999.0;

  std::vector <unsigned int> triggerIDs;


  // pp = pi+
  // pm = pi-
  // kp = K+
  // km = K-
  // pr = Proton

  TH1D *h_event_check = new TH1D("h_event_check","Event number after each cut;;Events", 3, 0, 3);
  const char *eventSections[3] = {"No cuts", "Trigger cut", "Vertex cut"};  
  h_event_check->SetStats(0);

  TH1D *h_track_check = new TH1D("h_track_check","Track number after each cut;;Tracks", 4, 0, 4);
  const char *trackSections[4] = {"Event cuts only", "TOF #beta cut", "QA cuts", "PID cuts"};  
  h_track_check->SetStats(0);

  TH1D *h_nhits      = new TH1D("h_nhits", "nHits;Number of hits;Tracks", 50, 0, 50);
  TH1D *h_nhits_fit  = new TH1D("h_nhits_fit","nHitsFit;Number of hits;Tracks", 50, 0, 50);
  TH1D *h_nhits_dEdx = new TH1D("h_nhits_dEdx","nHitsdEdx;Number of hits;Tracks", 50, 0, 50);

  TH1D *h_TOF_beta = new TH1D("h_TOF_beta","TOF #beta;#beta;Tracks", 100, 0, 1);

  TH1D *h_xvtx = new TH1D("h_xvtx","Primary Vertex Position in x;Distance (cm);Events", 100, -5, 5);
  TH1D *h_yvtx = new TH1D("h_yvtx","Primary Vertex Position in y;Distance (cm);Events", 100, -5, 5);
  TH1D *h_zvtx = new TH1D("h_zvtx","Primary Vertex Position in z;Distance (cm);Events", 100, 200, 220);

  TH1D *h_pi_TPC_nsig = new TH1D("h_pi_TPC_nsig", "#pi TPC n#sigma;n#sigma;Tracks",    200, -10, 10);
  TH1D *h_ka_TPC_nsig = new TH1D("h_ka_TPC_nsig", "K TPC n#sigma;n#sigma;Tracks",      200, -10, 10);
  TH1D *h_pr_TPC_nsig = new TH1D("h_pr_TPC_nsig", "Proton TPC n#sigma;n#sigma;Tracks", 200, -10, 10);

  TH1D *h_pp_dndm = new TH1D("h_pp_dndm", "#pi^{+} Raw m_{T} Spectrum;m_{T}-m_{0} (GeV);dN/dm_{T}", 60, 0, 3);//30, 0, 3);
  TH1D *h_pm_dndm = new TH1D("h_pm_dndm", "#pi^{-} Raw m_{T} Spectrum;m_{T}-m_{0} (GeV);dN/dm_{T}", 60, 0, 3);//30, 0, 3);
  TH1D *h_kp_dndm = new TH1D("h_kp_dndm", "K^{+} Raw m_{T} Spectrum;m_{T}-m_{0} (GeV);dN/dm_{T}",   60, 0, 3);//30, 0, 3);
  TH1D *h_km_dndm = new TH1D("h_km_dndm", "K^{-} Raw m_{T} Spectrum;m_{T}-m_{0} (GeV);dN/dm_{T}",   60, 0, 3);//30, 0, 3);
  TH1D *h_pr_dndm = new TH1D("h_pr_dndm", "Proton Raw m_{T} Spectrum;m_{T}-m_{0} (GeV);dN/dm_{T}",  60, 0, 3);//30, 0, 3);

  TH1D *h_pp_dndy = new TH1D("h_pp_dndy", "#pi^{+} Raw Rapidity Spectrum;y;dN/dy", 50, -2, 0.5);//25, -2, 0.5);
  TH1D *h_pm_dndy = new TH1D("h_pm_dndy", "#pi^{-} Raw Rapidity Spectrum;y;dN/dy", 50, -2, 0.5);//25, -2, 0.5);
  TH1D *h_kp_dndy = new TH1D("h_kp_dndy", "K^{+} Raw Rapidity Spectrum;y;dN/dy",   50, -2, 0.5);//25, -2, 0.5);
  TH1D *h_km_dndy = new TH1D("h_km_dndy", "K^{-} Raw Rapidity Spectrum;y;dN/dy",   50, -2, 0.5);//25, -2, 0.5);
  TH1D *h_pr_dndy = new TH1D("h_pr_dndy", "Proton Raw Rapidity Spectrum;y;dN/dy",  50, -2, 0.5);//25, -2, 0.5);

  TH1D *h_pp_pT = new TH1D("h_pp_pT", "#pi^{+} p_{T};p_{T} (GeV);", 100, 0, 5);
  TH1D *h_pm_pT = new TH1D("h_pm_pT", "#pi^{-} p_{T};p_{T} (GeV);", 100, 0, 5);
  TH1D *h_kp_pT = new TH1D("h_kp_pT", "K^{+} p_{T};p_{T} (GeV);",   100, 0, 5);
  TH1D *h_km_pT = new TH1D("h_km_pT", "K^{-} p_{T};p_{T} (GeV);",   100, 0, 5);
  TH1D *h_pr_pT = new TH1D("h_pr_pT", "Proton p_{T};p_{T} (GeV);",  100, 0, 5);

  TH1D *h_pp_mom = new TH1D("h_pp_mom", "#pi^{+} Total Momentum;|p| (GeV);", 100, 0, 5);
  TH1D *h_pm_mom = new TH1D("h_pm_mom", "#pi^{-} Total Momentum;|p| (GeV);", 100, 0, 5);
  TH1D *h_kp_mom = new TH1D("h_kp_mom", "K^{+} Total Momentum;|p| (GeV);",   100, 0, 5);
  TH1D *h_km_mom = new TH1D("h_km_mom", "K^{-} Total Momentum;|p| (GeV);",   100, 0, 5);
  TH1D *h_pr_mom = new TH1D("h_pr_mom", "Proton Total Momentum;|p| (GeV);",  100, 0, 5);



  TH2D *h2_y_vs_x_vtx = new TH2D("h2_y_vs_x_vtx","Transverse Position of Primary Vertex;x (cm);y (cm)", 500, -5, 5, 500, -5, 5);

  TH2D *h2_pi_m2_vs_TPC_nsig = new TH2D("h2_pi_m2_vs_TPC_nsig", "m^{2} vs #pi TPC n#sigma;n#sigma_{#pi};m^{2} (GeV^{2})", 500, -5, 5, 500, -0.1, 1.2);
  TH2D *h2_ka_m2_vs_TPC_nsig = new TH2D("h2_ka_m2_vs_TPC_nsig", "m^{2} vs K TPC n#sigma;n#sigma_{K};m^{2} (GeV^{2})", 500, -5, 5, 500, -0.1, 1.2);
  TH2D *h2_pr_m2_vs_TPC_nsig = new TH2D("h2_pr_m2_vs_TPC_nsig", "m^{2} vs Proton TPC n#sigma;n#sigma_{pro};m^{2} (GeV^{2})", 500, -5, 5, 500, -0.1, 1.2);

  TH2D *h2_pp_MvsY  = new TH2D("h2_pp_MvsY", "#pi^{+} m_{T} vs. Rapidity (Weighted);y;m_{T} - m_{0}", 50, -2, 0.5, 60, 0, 3);//25, -2, 0.5, 30, 0, 3);
  TH2D *h2_pm_MvsY  = new TH2D("h2_pm_MvsY", "#pi^{-} m_{T} vs. Rapidity (Weighted);y;m_{T} - m_{0}", 50, -2, 0.5, 60, 0, 3);//25, -2, 0.5, 30, 0, 3);
  TH2D *h2_kp_MvsY  = new TH2D("h2_kp_MvsY", "K^{+} m_{T} vs. Rapidity (Weighted);y;m_{T} - m_{0}",   50, -2, 0.5, 60, 0, 3);//25, -2, 0.5, 30, 0, 3);
  TH2D *h2_km_MvsY  = new TH2D("h2_km_MvsY", "K^{-} m_{T} vs. Rapidity (Weighted);y;m_{T} - m_{0}",   50, -2, 0.5, 60, 0, 3);//25, -2, 0.5, 30, 0, 3);
  TH2D *h2_pr_MvsY  = new TH2D("h2_pr_MvsY", "Proton m_{T} vs. Rapidity (Weighted);y;m_{T} - m_{0}",  50, -2, 0.5, 60, 0, 3);//25, -2, 0.5, 30, 0, 3);

  TH2D *h2_m2_vs_pTq  = new TH2D("h2_m2_vs_pTq", "m^{2} vs p_{T}/q;p_{T}/q (GeV);m^{2} (GeV^{2})", 500, -3, 3, 500, -0.1, 1.2);
  TH2D *h2_dEdx_vs_pq = new TH2D("h2_dEdx_vs_pq", "dE/dx vs |p|/q;|p|/q (GeV);dE/dx (keV/cm)", 400, -2, 2, 500, 0, 10);


  // EVENT LOOP
  for (Long64_t ievent = 0; ievent < events2read; ievent++)
    {
      Bool_t readEvent = picoReader->readPicoEvent(ievent);
      if( !readEvent ) { std::cout << "Event could not be read; aborting analysis." << std::endl; break; }
        
      StPicoDst *dst = picoReader->picoDst();
      StPicoEvent *event = dst->event();
      if( !event ) { std::cout << "No event found; aborting analysis." << std::endl; break; }

      h_event_check->Fill(eventSections[0], 1);

      //=========================================================
      //          Trigger Selection
      //=========================================================
      // loop for the trigger ids and see if any == 1

      triggerIDs.clear();
      triggerIDs = event->triggerIds();
      Bool_t b_bad_trig = true;

      for (UInt_t i = 0; i < triggerIDs.size(); i++) { if (triggerIDs[i] == 1) {b_bad_trig = false;} }

      if (b_bad_trig) continue;
      //=========================================================
      //      END Trigger Selection
      //=========================================================

      h_event_check->Fill(eventSections[1], 1);
      
      //=========================================================
      //          Z-VTX Selection
      //=========================================================
      // Fill vertex coordinates and check the z-vertex position

      TVector3 pVtx = event->primaryVertex();

      d_xvtx = pVtx.x();
      d_yvtx = pVtx.y();
      d_zvtx = pVtx.z();

      h_xvtx->Fill(d_xvtx);
      h_yvtx->Fill(d_yvtx);
      h_zvtx->Fill(d_zvtx);

      Bool_t b_bad_xvtx = ( (d_xvtx < -1.2) || (d_xvtx > 0.4)  );
      Bool_t b_bad_yvtx = ( (d_yvtx < -2.5) || (d_yvtx > -1.5) );
      Bool_t b_bad_zvtx = ( (d_zvtx < 210)  || (d_zvtx > 212)  );

      if (/*b_bad_xvtx || b_bad_yvtx || */b_bad_zvtx) continue;

      h2_y_vs_x_vtx->Fill(d_xvtx, d_yvtx);

      if (b_bad_xvtx || b_bad_yvtx) continue;
      //=========================================================
      //      END Z-VTX Selection
      //=========================================================
      
      h_event_check->Fill(eventSections[2], 1);

      /*
      //=========================================================
      //      Track Cut Settings
      //=========================================================
      // Mins and maxes for the track selection

      //int nHits_min = 9;  //minimum number of hits
        
      Double_t d_pT_min  = 0.1;
      Double_t d_pT_max  = 10.0;
      Double_t d_mom_min = 0.1;
      Double_t d_mom_max = 10.0;

      //double d_SigmaCutLevel = 4.0;  //nsigmas must be less than this value
      
      //=========================================================
      //      END Track Cut Settings
      //=========================================================
      */

      // TRACK LOOP OVER PRIMARY TRACKS
      Int_t nTracks = dst->numberOfTracks();
      for(Int_t iTrk = 0; iTrk < nTracks; iTrk++)
	{            
	  // Retrieve i-th pico track and make sure it's a primary vertex
	  StPicoTrack *picoTrack = dst->track(iTrk);            
	  if(picoTrack == NULL) continue;
	  if(!picoTrack->isPrimary()) continue;

	  h_track_check->Fill(trackSections[0], 1);

	  //=========================================================
	  //          TOF Beta Cuts
	  //=========================================================

	  if (!picoTrack->isTofTrack()) continue;                      // Only TOF tracks

	  StPicoBTofPidTraits *trait = dst->btofPidTraits(picoTrack->bTofPidTraitsIndex());

	  Double_t d_tofBeta = trait->btofBeta();

	  if (d_tofBeta < 0.005 || d_tofBeta >= 1) continue;
	  
	  h_TOF_beta->Fill(d_tofBeta);

	  //=========================================================
	  //          End TOF Beta Cuts
	  //=========================================================

	  h_track_check->Fill(trackSections[1], 1);


	  //=========================================================
	  //          Track QA Cuts
	  //=========================================================

	  h_nhits->Fill(picoTrack->nHits());
	  h_nhits_fit->Fill(picoTrack->nHitsFit());
	  h_nhits_dEdx->Fill(picoTrack->nHitsDedx());

	  unsigned short nHits = picoTrack->nHits();
	  
	  bool b_bad_dEdx     = ( picoTrack->nHitsDedx() <= 0 );
	  bool b_bad_tracking = ( ((double)picoTrack->nHitsFit() / (double)picoTrack->nHitsPoss()) < 0.52 );

	  if (b_bad_dEdx || b_bad_tracking) continue;

	  //=========================================================
	  //          End Track QA Cuts
	  //=========================================================

	  h_track_check->Fill(trackSections[2], 1);

	  Double_t d_TPCnSigmaPion   = picoTrack->nSigmaPion();
	  Double_t d_TPCnSigmaProton = picoTrack->nSigmaProton();
	  Double_t d_TPCnSigmaKaon   = picoTrack->nSigmaKaon();

	  h_pi_TPC_nsig->Fill(d_TPCnSigmaPion);
	  h_ka_TPC_nsig->Fill(d_TPCnSigmaProton);
	  h_pr_TPC_nsig->Fill(d_TPCnSigmaKaon);


	  Double_t d_charge = picoTrack->charge();
	  Double_t d_dEdx  = picoTrack->dEdx();
	  Double_t d_px    = picoTrack->pMom().x();
	  Double_t d_py    = picoTrack->pMom().y();
	  Double_t d_pz    = picoTrack->pMom().z();
	  Double_t d_pT    = picoTrack->pPt();
	  Double_t d_mom   = sqrt(d_pT*d_pT + d_pz*d_pz);
	  Double_t d_m2    = d_mom*d_mom*( (1 / (d_tofBeta*d_tofBeta)) - 1);

	  h2_pi_m2_vs_TPC_nsig->Fill(d_TPCnSigmaPion, d_m2);
	  h2_ka_m2_vs_TPC_nsig->Fill(d_TPCnSigmaKaon, d_m2);
	  h2_pr_m2_vs_TPC_nsig->Fill(d_TPCnSigmaProton, d_m2);

	  h2_m2_vs_pTq->Fill(d_pT/d_charge, d_m2);
	  h2_dEdx_vs_pq->Fill(d_mom/d_charge, d_dEdx);

	  //=========================================================
	  //          PID Cuts
	  //=========================================================

	  Bool_t pion   = ( (d_TPCnSigmaPion > -2)      && (d_TPCnSigmaPion < 2)      && (d_m2 > 0.005) && (d_m2 < 0.03) );    // PARTICLE TAGGING
	  Bool_t kaon   = ( (d_TPCnSigmaKaon > -2.25)   && (d_TPCnSigmaKaon < 2.25)   && (d_m2 > 0.2)   && (d_m2 < 0.28) );
	  Bool_t proton = ( (d_TPCnSigmaProton > -2.25) && (d_TPCnSigmaProton < 2.25) && (d_m2 > 0.75)  && (d_m2 < 1)    );

	  if (!pion && !kaon && !proton) continue;

	  if (pion && kaon) continue;   //Sanity check
	  if (pion && proton) continue;
	  if (kaon && proton) continue;

	  //=========================================================
	  //          PID Cuts
	  //=========================================================

	  h_track_check->Fill(trackSections[3], 1);

	  Double_t d_m0_pi = 0.1396;   //Rest masses
	  Double_t d_m0_ka = 0.4937;
	  Double_t d_m0_pr = 0.9383;

	  if (pion)
	    {
	      if (d_charge > 0) 
		{
		  fillRawSpect(d_px, d_py, d_pz, d_m0_pi, h_pp_dndy, h_pp_dndm, h2_pp_MvsY);
		  h_pp_pT->Fill(d_pT);
		  h_pp_mom->Fill(d_mom);
		}
	      else  
		{
		  fillRawSpect(d_px, d_py, d_pz, d_m0_pi, h_pm_dndy, h_pm_dndm, h2_pm_MvsY);
		  h_pm_pT->Fill(d_pT);
		  h_pm_mom->Fill(d_mom);
		}
	    }
	  else if (kaon)
	    {
	      if (d_charge > 0) 
		{
		  fillRawSpect(d_px, d_py, d_pz, d_m0_ka, h_kp_dndy, h_kp_dndm, h2_kp_MvsY);
		  h_kp_pT->Fill(d_pT);
		  h_kp_mom->Fill(d_mom);
		}
	      else  
		{
		  fillRawSpect(d_px, d_py, d_pz, d_m0_ka, h_km_dndy, h_km_dndm, h2_km_MvsY);
		  h_km_pT->Fill(d_pT);
		  h_km_mom->Fill(d_mom);
		}
	    }
	  else if (proton)
	    {
	      if (d_charge > 0) 
		{
		  fillRawSpect(d_px, d_py, d_pz, d_m0_pr, h_pr_dndy, h_pr_dndm, h2_pr_MvsY);
		  h_pr_pT->Fill(d_pT);
		  h_pr_mom->Fill(d_mom);
		}
	    }
	}//End track loop
    }//End event loop

  outputFile->cd();

  h_event_check ->Write();
  h_track_check ->Write();
  h_TOF_beta    ->Write();
  h_nhits       ->Write();
  h_nhits_fit   ->Write();
  h_nhits_dEdx  ->Write();
  h_xvtx        ->Write();
  h_yvtx        ->Write();
  h_zvtx        ->Write();
  h_pi_TPC_nsig ->Write();
  h_ka_TPC_nsig ->Write();
  h_pr_TPC_nsig ->Write();
  h_pp_dndm     ->Write();
  h_pm_dndm     ->Write();
  h_kp_dndm     ->Write();
  h_km_dndm     ->Write();
  h_pr_dndm     ->Write();
  h_pp_dndy     ->Write();
  h_pm_dndy     ->Write();
  h_kp_dndy     ->Write();
  h_km_dndy     ->Write();
  h_pr_dndy     ->Write();
  h_pp_pT       ->Write();
  h_pm_pT       ->Write();
  h_kp_pT       ->Write();
  h_km_pT       ->Write();
  h_pr_pT       ->Write();
  h_pp_mom      ->Write();
  h_pm_mom      ->Write();
  h_kp_mom      ->Write();
  h_km_mom      ->Write();
  h_pr_mom      ->Write();

  h2_pp_MvsY           ->Write();
  h2_pm_MvsY           ->Write();
  h2_kp_MvsY           ->Write();
  h2_km_MvsY           ->Write();
  h2_pr_MvsY           ->Write();
  h2_y_vs_x_vtx        ->Write();
  h2_pi_m2_vs_TPC_nsig ->Write();
  h2_ka_m2_vs_TPC_nsig ->Write();
  h2_pr_m2_vs_TPC_nsig ->Write();
  h2_m2_vs_pTq         ->Write();
  h2_dEdx_vs_pq        ->Write();


  gROOT->GetListOfFiles()->Remove(outputFile);
  outputFile->Close();

  picoReader->Finish();

  std::cout << "Done!" << std::endl;
}//End picoAnalyzer()





////////
//   Using px, py, pz, and rest mass, return rapidity
////////
Double_t rapidity(Double_t px, Double_t py, Double_t pz, Double_t mass)
{
  Double_t rapidity, energy, momentum = 0;
  momentum = TMath::Sqrt(px*px + py*py + pz*pz);
  energy   = TMath::Sqrt(momentum*momentum + mass*mass);
  rapidity = TMath::ATanH(pz/energy);
  return rapidity;
}

////////
//   Using px, py, pz, and rest mass, return transverse mass
////////
Double_t transMass(Double_t px, Double_t py, Double_t mass) {return TMath::Sqrt(mass*mass + px*px + py*py);}


////////
//   Using px, py, pz, and rest mass, fill histograms raw of dN/dy,
// dN/dmT (shifted left by m0), and a 2D histogram of mT-m0 vs y.
////////
void fillRawSpect(Double_t px, Double_t py, Double_t pz, Double_t mass, TH1D *dndy, TH1D *dndm, TH2D *MvsY)
{
  Double_t y  = rapidity(px, py, pz, mass);
  Double_t mT = transMass(px, py, mass);
  Double_t M  = mT - mass;
  dndy->Fill(y);
  dndm->Fill(M);
  MvsY->Fill(y,M, 1/(TMath::TwoPi() * mT));
}
