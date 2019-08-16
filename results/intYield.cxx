void usage();
void fillIntYield(TH2D *MvsY, TH1D *dndy, TString fitType, Float_t mass, TFile *outputFile);
Double_t getViewRange(Double_t maxValue);


void intYield(TString jobID, TString fitType)
{
  if (jobID.EqualTo("")) { usage(); return; }
  if (!fitType.EqualTo("E") && !fitType.EqualTo("L") && !fitType.EqualTo("B")) { usage(); return; }
  
  TString fileName = jobID + ".picoDst.result.combined.root";
  TFile *file = TFile::Open(fileName, "read");
  if (!file) { std::cout << "File not found!" << std::endl; return; }

  TString outputName = jobID + ".picoDst.result."+fitType+"yields.root";
  TFile *output = new TFile(outputName, "recreate");

  TH2D *pp_MvsY = (TH2D*)file->Get("h2_pp_MvsY");
  TH2D *pm_MvsY = (TH2D*)file->Get("h2_pm_MvsY");
  TH2D *kp_MvsY = (TH2D*)file->Get("h2_kp_MvsY");
  TH2D *km_MvsY = (TH2D*)file->Get("h2_km_MvsY");
  TH2D *pr_MvsY = (TH2D*)file->Get("h2_pr_MvsY");

  // Adapt histogram sizes to match those retrieved
  Int_t nbins  = pp_MvsY->GetNbinsX();
  Double_t min = pp_MvsY->GetXaxis()->GetXmin();
  Double_t max = pp_MvsY->GetXaxis()->GetXmax();

  TH1D *pp_yield = new TH1D("pp_yield", "#pi^{+} Rapidity (Integrated);y;N", nbins, min, max);
  TH1D *pm_yield = new TH1D("pm_yield", "#pi^{-} Rapidity (Integrated);y;N", nbins, min, max);
  TH1D *kp_yield = new TH1D("kp_yield", "K^{+} Rapidity (Integrated);y;N",   nbins, min, max);
  TH1D *km_yield = new TH1D("km_yield", "K^{-} Rapidity (Integrated);y;N",   nbins, min, max);
  TH1D *pr_yield = new TH1D("pr_yield", "Proton Rapidity (Integrated);y;N",  nbins, min, max);

  Double_t m0_pi = 0.1396;   //Rest masses
  Double_t m0_ka = 0.4937;
  Double_t m0_pr = 0.9383;

  fillIntYield(pp_MvsY, pp_yield, fitType, m0_pi, output);
  fillIntYield(pm_MvsY, pm_yield, fitType, m0_pi, output);
  fillIntYield(kp_MvsY, kp_yield, fitType, m0_ka, output);
  fillIntYield(km_MvsY, km_yield, fitType, m0_ka, output);
  fillIntYield(pr_MvsY, pr_yield, fitType, m0_pr, output);

  output->cd();
  
  pp_yield->Write();
  pm_yield->Write();
  kp_yield->Write();
  km_yield->Write();
  pr_yield->Write();

  output->Close();
  file->Close();

  std::cout << "Integrated yields saved to " + outputName + "." << std::endl;
  /*
  std::cout << "Plotting results..." << std::endl;

  gROOT->ProcessLine(".x overlay.cxx(\""+jobID+"\",\""+fitType+"\")");
  */
}//End intYield()


void usage()
{
  std::cout << "Usage: root -l -b -q 'intYield(\"[job ID]\",\"[fit type]\")'" << std::endl
	    << "Fit types: E = exponential, L = Levy, B = Blast-wave" << std::endl;
}


Double_t getViewRange(Double_t maxValue)
{
  // Get string form of the number, then cut off the decimals
  TString maxValStr;
  maxValStr.Form("%f", maxValue);
  maxValStr.Form("%d", maxValStr.Atoi());

  // Get the number of digits 
  Int_t digits = maxValStr.Length();

  // Get next factor needed to get above maxValue
  // maxValue is at 10E+(digits-2), we need the next one up
  TString nextHighestFactor;
  nextHighestFactor.Form("%d", digits-1);

  // Return Double_t form of the max view range
  TString maxViewRange = "10E+" + nextHighestFactor;
  return maxViewRange.Atof();
}

////////
//   Letting M = mT - m0
//   For a given rapidity bin, a projection is taken of the 2D
// histogram (M vs y) to get the M histogram at rapidity bin dy. According
// to the fitType, that M is fitted (rebinned and refitted if it fails), 
// the fit function is integrated up through the last non-empty bin, 
// and that integration is filled into the dndy histogram as the dN at the
// bin dy. Error in integration is set as the error in dN.
////////
void fillIntYield(TH2D *MvsY, TH1D *dndy, TString fitType, Float_t mass, TFile *outputFile)
{
  TString yieldName = dndy->GetName();

  Double_t maxViewRange = getViewRange(MvsY->GetMaximum());

  for (Int_t ybin = 1; ybin < MvsY->GetNbinsX(); ybin++)
    {
      // String of bin number
      TString ybinStr;
      ybinStr.Form("%d",ybin);

      // Corresponding names for successfully fitted histograms/functions
      TString ithHistName = yieldName + "_bin" + ybinStr;
      TString ithFuncName = yieldName + "_func" + ybinStr;

      // Strings of rapidity range for this fit
      TString minYR, maxYR;
      minYR.Form("%3.2f", MvsY->GetXaxis()->GetBinLowEdge(ybin));
      maxYR.Form("%3.2f", MvsY->GetXaxis()->GetBinLowEdge(ybin) + MvsY->GetXaxis()->GetBinWidth(ybin));

      // Get the projection at 'ybin', fix size and titles, add rapidity range to the title
      TH1D *MatY = MvsY->ProjectionY(ithHistName, ybin, ybin);
      MatY->GetYaxis()->SetTitle("d^{2}N/2#pi m_{T}dm_{T}dy");
      MatY->GetYaxis()->SetRangeUser(1, maxViewRange);
            
      TString origTitle = MatY->GetTitle();
      TString newTitle( origTitle(0, origTitle.Index("m_{T} vs. Rapidity (Weighted)")) );         //construct new title and cut off that portion shown
      newTitle.Append(" Invariant Spectrum (" + minYR + " #leq y < " + maxYR + ")");           //add on the rapidity range of this particular fit
      MatY->SetTitle(newTitle);


      if(MatY->GetEntries() == 0)
	{
	  dndy->SetBinContent(ybin, 0);
	  delete MatY;
	  continue;
	}


      Double_t minR    = MatY->GetXaxis()->GetXmin();    //Min and max range for function
      Double_t maxR    = MatY->GetXaxis()->GetXmax();
      Double_t minIntR = MatY->GetXaxis()->GetXmin();    //Min and max range for integration
      Double_t maxIntR = MatY->GetXaxis()->GetXmax();
      Double_t minFitR = 0.5;                            //Min and max range for fitting
      Double_t maxFitR = 2.0;
  
      // Determine the function requested and starting parameters
      TF1 *func;   // For fitting to get the needed parameters
      TF1 *ifunc;  // For integrating, this includes the (2*pi*mT) factor needed for integration

      if (fitType.EqualTo("E"))
	{
	  func = new TF1(ithFuncName, "[0]*TMath::Exp(-[1]*(x+[2]))", minR, maxR);
	  func->SetParameter(0, MatY->GetMaximum()); func->SetParName(0, "Scale");
	  func->SetParameter(1, 1); func->SetParName(1, "1/T");
	  func->FixParameter(2, mass); func->SetParName(2, "m_{0}");

	  ifunc = new TF1("iexpo", "TMath::TwoPi()*(x+[2])*[0]*TMath::Exp(-[1]*(x+[2]))", minR, maxR);
	  ifunc->SetParameter(0, MatY->GetMaximum());
	  ifunc->SetParameter(1, 1);
	  ifunc->FixParameter(2, mass);
	}
      else if (fitType.EqualTo("L"))
	{
	  // DON'T USE YET
	  func = new TF1("levy", "[0]*TMath::Sqrt([1]/TMath::TwoPi())*(TMath::Exp(-[1]/(2*(x-[2])))/TMath::Power((x-[2]),(3/2)))", minR, maxR);
	  func->SetParameter(0, MatY->GetMaximum());
	  func->SetParameter(1, 0.5);
	  func->SetParameter(2, 0);
	}
      else
	{
	  minFitR = 0.2;
	  //maxFitR = 1.0;
	  minIntR = 0.00001;
	  minR = 0.00001;

	  //Up to order (beta*r^(1/2))^6
	  func = new TF1(ithFuncName, "([0]*(x+[3])/(768*[2]*[2]*[2]*[2]))*((8*[2]*[2]*[2]*[2]*(48+16*[1]*[1]+9*[1]*[1]*[1]*[1])+3*[1]*[1]*[1]*[1]*TMath::Power((x+[3])*(x+[3])-[3]*[3],4)+8*[2]*[2]*[1]*[1]*((x+[3])*(x+[3])*(x+[3])*(x+[3])*(8+9*[1]*[1])+[3]*[3]*[3]*[3]*(8+9*[1]*[1])+(x+[3])*(x+[3])*(3*[1]*[1]-2*(8+9*[1]*[1])*[3]*[3])))*TMath::BesselK1((x+[3])/[2])-8*(x+[3])*[2]*[1]*[1]*(2*[2]*[2]*(8+3*[1]*[1])+3*[1]*[1]*TMath::Power((x+[3])*(x+[3])-[3]*[3],2))*TMath::BesselK(2,(x+[3])/[2]))", minR, maxR);

	  func->SetParameter(0, 4 * MatY->GetMaximum()); func->SetParName(0, "Scale");
	  func->SetParameter(1, 0.25); func->SetParName(1, "#beta_{S}");
	  func->SetParameter(2, 0.5);  func->SetParName(2, "T");
	  func->FixParameter(3, mass); func->SetParName(3, "m_{0}");

	  func->SetParLimits(1, 0, 1);
	  func->SetParLimits(2, 0.01, 10);


	  ifunc = new TF1("iblast", "TMath::TwoPi()*(x+[3])*([0]*(x+[3])/(768*[2]*[2]*[2]*[2]))*((8*[2]*[2]*[2]*[2]*(48+16*[1]*[1]+9*[1]*[1]*[1]*[1])+3*[1]*[1]*[1]*[1]*TMath::Power((x+[3])*(x+[3])-[3]*[3],4)+8*[2]*[2]*[1]*[1]*((x+[3])*(x+[3])*(x+[3])*(x+[3])*(8+9*[1]*[1])+[3]*[3]*[3]*[3]*(8+9*[1]*[1])+(x+[3])*(x+[3])*(3*[1]*[1]-2*(8+9*[1]*[1])*[3]*[3])))*TMath::BesselK1((x+[3])/[2])-8*(x+[3])*[2]*[1]*[1]*(2*[2]*[2]*(8+3*[1]*[1])+3*[1]*[1]*TMath::Power((x+[3])*(x+[3])-[3]*[3],2))*TMath::BesselK(2,(x+[3])/[2]))", minR, maxR);

	  ifunc->SetParameter(0, 4 * MatY->GetMaximum());
	  ifunc->SetParameter(1, 0.25);
	  ifunc->SetParameter(2, 0.5);
	  ifunc->FixParameter(3, mass);

	  ifunc->SetParLimits(1, 0, 1);
	  ifunc->SetParLimits(2, 0.01, 10);
	}

      // Check result of the fit, rebin by 2 and refit if it failed
      TFitResultPtr result1 = MatY->Fit(func->GetName(), "QES0", "", minFitR, maxFitR);
      TFitResultPtr result2;
      Int_t NDF = func->GetNDF();
  
      if (/*result1 == -1 ||*/ !result1->IsValid())    // UNCOMMENT IF USING ROOT 6
	{
	  MatY    = (TH1D*)MatY->Rebin();
	  result2 = MatY->Fit(func->GetName(), "QES0", "", minFitR, maxFitR);
	  NDF = func->GetNDF();
	}

      // Set the now known parameters of the integration function
      ifunc->SetParameters(func->GetParameters());
      ifunc->SetParErrors(func->GetParErrors());

      // Reset max range to remove empty bins from the integration
      Int_t lastBin      = MatY->GetNbinsX();
      Int_t lastNonEmpty = MatY->FindLastBinAbove(0,1);   // = -1 if the histogram is empty
  
      if (lastNonEmpty < lastBin && lastNonEmpty != -1)
	{ 
	  maxIntR = MatY->GetBinLowEdge(lastNonEmpty + 1);
	  func->SetRange(minR, maxIntR);
	  ifunc->SetRange(minR, maxIntR);
	}

      // Fill the rapidity bin with the integrated dN
      Double_t binWidth = MatY->GetXaxis()->GetBinWidth(1);
      Float_t dN;
      Float_t err;

      if (NDF >= 10) 
	{
	  dN  = ifunc->Integral(minIntR,maxIntR) / binWidth;  
	  err = ifunc->IntegralError(minIntR,maxIntR) / binWidth;

	  dndy->SetBinContent(ybin, dN);
	  dndy->SetBinError(ybin, err);

	  // Write each successfully fitted histogram and corresponding function
	  outputFile->cd();
	  MatY->Write();
	  func->Write();
	}
      else
	{ dndy->SetBinContent(ybin, 0); }       //Don't trust any bins with NDF < 10

      delete func;
      delete ifunc;
      delete MatY;
    }//End for (ybins)
}//End fillIntYield()
