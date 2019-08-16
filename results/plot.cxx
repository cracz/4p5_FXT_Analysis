void plot(TString jobID)
{
  TString fileName = jobID + ".picoDst.result.combined.root";

  TFile *file = TFile::Open(fileName);
  if(!file) {cout << "Wrong file!" << endl; return;}

  //cout << "Drawing histograms from " << fileName << endl;
  
  TH1F  *hist;
  TString name;
  TIter next(file->GetListOfKeys());
  TKey  *key;

  TCanvas *canvas = new TCanvas("canvas", "Canvas");
  canvas->SetCanvasSize(875, 675);

  while ((key = (TKey*)next()))
    {
      hist = (TH1F*)key->ReadObj();
      name = hist->GetName();

      if (hist->GetEntries() == 0) { continue; }

      canvas->SetLogy(0);    //Reset y axis to linear
      canvas->SetGrid();     //Draw grid lines
      gStyle->SetOptStat(1); //Reset stat box
      gStyle->SetPalette();  //Reset color palette

      Bool_t errors = false; //Draw error bars
      
      TClass *cl = gROOT->GetClass(key->GetClassName());

      if (cl->InheritsFrom("TH2"))                       //Keep this if/else order!! A TH2 still inherits from TH1.
	{
	  canvas->SetRightMargin(0.13);
	  gStyle->SetOptStat(11);
	  gStyle->SetPalette(53); //kDarkBodyRadiator
	  canvas->SetLogz();

	  if (name.Contains("dEdx"))
	    {
	      hist->GetZaxis()->SetRangeUser(1,10000);
	    }
	  else if (name.Contains("TPC"))
	    {
	      hist->GetZaxis()->SetRangeUser(1,100000);
	    }
	  else if (name.Contains("vtx"))
	    {
	      hist->GetZaxis()->SetRangeUser(1,10000);
	    }
	  else if (name.Contains("MvsY"))
	    {
	      canvas->SetLogz(0);
	    }

	  hist->Draw("COLZ");
	  canvas->Update();
	}
      else if (cl->InheritsFrom("TH1"))
	{
	  hist->SetTitleOffset(1.2, "y");

	  if (name.Contains("vtx"))
	    {
	      canvas->SetLogy();
	    }
	  else if (name.Contains("TOF_beta"))
	    {
	      canvas->SetLogy();
	      hist->GetYaxis()->SetRangeUser(0.1,10E+7);
	      gStyle->SetOptStat(11);
	    }
	  else if (name.Contains("check"))
	    {
	      hist->SetFillColorAlpha(4,0.6);
	    }
	  else if (name.Contains("dndy")) 
	    { 
	      continue;
	    }
	  else if (name.Contains("dndm")) 
	    { 
	      hist->SetMinimum(0.1);
	      canvas->SetLogy();
	      errors = true;
	    }


	  if (errors) 
	    hist->Draw("E1");
	  else 
	    hist->Draw();

	  canvas->Update();
	}
      else
	continue;
      
      canvas->SaveAs(jobID+"."+name+".png");
    }

  canvas->Close();
  delete file;

  cout << "Done!" << endl;
}
