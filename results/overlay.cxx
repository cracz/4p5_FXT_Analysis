void usage();

void overlay(TString jobID, TString fitType)
{
  if (jobID.EqualTo("")) { usage(); return; }
  if (!fitType.EqualTo("E") && !fitType.EqualTo("L") && !fitType.EqualTo("B")) { usage(); return; }
  
  TString rawFileName = jobID + ".picoDst.result.combined.root";
  TString intFileName = jobID + ".picoDst.result."+fitType+"yields.root";

  TFile *rawFile = TFile::Open(rawFileName, "read");
  if (!rawFile) { std::cout << "File with raw spectra not found!" << std::endl; return; }

  TFile *intFile = TFile::Open(intFileName, "read");
  if (!intFile) { std::cout << "File with integrated spectra not found!" << std::endl; return; }


  TH1D *pp_raw = (TH1D*)rawFile->Get("h_pp_dndy");
  TH1D *pm_raw = (TH1D*)rawFile->Get("h_pm_dndy");
  TH1D *kp_raw = (TH1D*)rawFile->Get("h_kp_dndy");
  TH1D *km_raw = (TH1D*)rawFile->Get("h_km_dndy");
  TH1D *pr_raw = (TH1D*)rawFile->Get("h_pr_dndy");

  TH1D *pp_int = (TH1D*)intFile->Get("pp_yield");
  TH1D *pm_int = (TH1D*)intFile->Get("pm_yield");
  TH1D *kp_int = (TH1D*)intFile->Get("kp_yield");
  TH1D *km_int = (TH1D*)intFile->Get("km_yield");
  TH1D *pr_int = (TH1D*)intFile->Get("pr_yield");

  
  TObjArray *rawArray = new TObjArray();
  rawArray->Add(pp_raw);
  rawArray->Add(pm_raw);
  rawArray->Add(kp_raw);
  rawArray->Add(km_raw);
  rawArray->Add(pr_raw);
  rawArray->Compress();

  TObjArray *intArray = new TObjArray();
  intArray->Add(pp_int);
  intArray->Add(pm_int);
  intArray->Add(kp_int);
  intArray->Add(km_int);
  intArray->Add(pr_int);
  intArray->Compress();


  TCanvas *canvas = new TCanvas("canvas", "Title", 875, 675);
  canvas->SetGrid();
  canvas->SetTicks();
  gStyle->SetOptStat(0);
  
  TLegend *leg = new TLegend(0.72,0.72,0.9,0.9);


  for (Int_t i = 0; i < rawArray->GetEntriesFast(); i++)
    {
      ((TH1D*)rawArray->At(i))->SetMarkerStyle(27);
      ((TH1D*)rawArray->At(i))->SetMarkerColor(4);
      ((TH1D*)rawArray->At(i))->SetMarkerSize(2.5);
      ((TH1D*)rawArray->At(i))->SetLineColor(4);

      ((TH1D*)intArray->At(i))->SetMarkerStyle(24);
      ((TH1D*)intArray->At(i))->SetMarkerColor(2);
      ((TH1D*)intArray->At(i))->SetMarkerSize(1.5);
      ((TH1D*)intArray->At(i))->SetLineColor(2);

      ((TH1D*)intArray->At(i))->DrawClone("E1");
      ((TH1D*)rawArray->At(i))->DrawClone("E1 SAME");

      ((TH1D*)intArray->At(i))->SetMarkerStyle(1);
      ((TH1D*)rawArray->At(i))->SetMarkerStyle(1);
      ((TH1D*)intArray->At(i))->Draw("E1 SAME");
      ((TH1D*)rawArray->At(i))->Draw("E1 SAME");
  
      leg->Clear();
      leg->AddEntry(((TH1D*)rawArray->At(i))->GetName(), "Raw", "ep");
      leg->AddEntry(((TH1D*)intArray->At(i))->GetName(), "Integrated", "ep");
      leg->Draw();

      canvas->Update();
      canvas->SaveAs(jobID+"."+((TH1D*)rawArray->At(i))->GetName()+"_fit"+fitType+"_overlay.png");
      canvas->Clear();
      
    }

  canvas->Close();
  rawFile->Close();
  intFile->Close();
  delete rawArray;
  delete intArray;
  delete rawFile;
  delete intFile;
}//End overlay()



void showAllFits(TString jobID, TString fitType)
{
  if (jobID.EqualTo("")) { usage(); return; }
  if (!fitType.EqualTo("E") && !fitType.EqualTo("L") && !fitType.EqualTo("B")) { usage(); return; }
  
  TString intFileName = jobID + ".picoDst.result."+fitType+"yields.root";
  TFile *intFile = TFile::Open(intFileName, "read");
  if (!intFile) { std::cout << "File with integrated spectra not found!" << std::endl; return; }

  TH1D *pp_int = (TH1D*)intFile->Get("pp_yield");
  TH1D *pm_int = (TH1D*)intFile->Get("pm_yield");
  TH1D *kp_int = (TH1D*)intFile->Get("kp_yield");
  TH1D *km_int = (TH1D*)intFile->Get("km_yield");
  TH1D *pr_int = (TH1D*)intFile->Get("pr_yield");

  TObjArray *intArray = new TObjArray();
  intArray->Add(pp_int);
  intArray->Add(pm_int);
  intArray->Add(kp_int);
  intArray->Add(km_int);
  intArray->Add(pr_int);
  intArray->Compress();


  TCanvas *canvas = new TCanvas("canvas", "Title", 875, 675);
  canvas->SetGrid();
  canvas->SetLogy();
  canvas->SetTicks();

  gStyle->SetOptFit(1);

  for (Int_t j = 0; j < intArray->GetEntriesFast(); j++)  //Loop over particle types
    {
      TString prefix = ((TH1D*)intArray->At(j))->GetName();

      for (Int_t k = 1; k <= ((TH1D*)intArray->At(j))->GetNbinsX(); k++)  //Loop over bin numbers
	{
	  TString binStr;
	  binStr.Form("%d", k);

	  TString histName = prefix + "_bin" + binStr;
	  TString funcName = prefix + "_func" + binStr;

	  TH1D *hist = (TH1D*)intFile->Get(histName);
	  if (!hist) continue;

	  TF1 *func = (TF1*)intFile->Get(funcName);
	  func->SetLineColor(2);

	  hist->Draw("E1");
	  func->Draw("SAME");

	  canvas->Update();
	  canvas->SaveAs(histName+".png");
	  canvas->Clear();
	}
    }

  canvas->Close();
  intFile->Close();
  delete intArray;
  delete intFile;
}//End getAllFits()


void usage()
{
  std::cout << "Usage: root -l -b -q 'overlay.cxx(\"[job ID]\",\"[fit type]\")'" << std::endl
	    << "Fit types: E = exponential, L = Levy, B = Blast-wave" << std::endl;
}
