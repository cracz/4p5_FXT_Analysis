void combine(/*TString fitType,*/TString jobId, Int_t fileNumber, Int_t keyJumpIndex = 1) 
{
  // output file
  char name[1000];

  TString outputPicoFileName = jobId;
  outputPicoFileName.Append(".picoDst.result.combined.root");
  TFile *outputPicoFile = new TFile(outputPicoFileName,"recreate");

  // input file
  TFile *tmpFile[1000]; //Array of root files
  TString tmpFilename;
  TObjArray *ObjArray = new TObjArray(500); 
  Int_t Nkeys = 0;
  Int_t Nhist = 0;

  for(Int_t i=0; i < fileNumber; i++) 
    {
      sprintf(name,"_%d.picoDst.result.root",i);
      tmpFilename = jobId;
      tmpFilename.Append(name);
      tmpFile[i] = new TFile(tmpFilename,"read");

      if(tmpFile[i]->IsZombie()) continue;

      if(i == 0) Nkeys = tmpFile[i]->GetNkeys();
      if(Nkeys == 0) { std::cout << "No keys!" << std::endl; return; }

      TList *list = tmpFile[i]->GetListOfKeys();
      TIter next((TList*)list);
      TKey  *key;
      Int_t histCounter = 0;
      Int_t keyJumper  = 0;

      while ( (key = (TKey*)next()) ) 
	{
	  keyJumper++;
	  if(keyJumper < keyJumpIndex) continue;

	  TObject *obj = tmpFile[i]->Get(key->GetName());
	  if(!obj) { std::cout << "Could not get object out of file " << i << std::endl; return; }
	  if(obj->IsA() == TDirectory::Class()) continue;
	  if(obj->InheritsFrom(TH1::Class())) 
	    {
	      if(i == 0) ObjArray->Add((TH1*)obj);
	      if(i >= 1) ((TH1*)ObjArray->At(histCounter))->Add((TH1*)obj); //Add like histograms together
	      histCounter++;
	    }
	}

      if(i == 0) Nhist = histCounter;

      // clean up
      if(i >= 1)
	{
	  tmpFile[i]->Close();
	  delete tmpFile[i];
	}

      std::cout << "File " << i+1 << " read in." << std::endl;
    }

  // write merged histograms
  std::cout << "Writing merged histograms." << std::endl;

  for(Int_t k=0; k <= Nhist-keyJumpIndex; k++) 
    {
      outputPicoFile->cd();
      ((TH1*)ObjArray->At(k))->Write();
    }

  ObjArray->Clear();
  tmpFile[0]->Close();
  delete tmpFile[0];


  /*
  if (!fitType.EqualTo("E") && !fitType.EqualTo("L") && !fitType.EqualTo("B")) { return 0; }

  TString outputPysFileName = jobId;
  outputPysFileName.Append(".pys_fit"+fitType+".result.combined.root");
  TFile *outputPysFile = new TFile(outputPysFileName, "RECREATE");

  //Combine Particle Yield Spectra plots
  for(Int_t j=0; j < fileNumber; j++) 
    {
      TString indexStr;
      indexStr.Form("%d",j);

      TString suffix = "_"+indexStr+".pys_fit"+fitType+".result.root"; 
      //sprintf(name, suffix, j);
      tmpFilename = jobId;
      tmpFilename.Append(suffix);
      tmpFile[j] = new TFile(tmpFilename,"read");

      if(j == 0) Nkeys = tmpFile[j]->GetNkeys();
      if(Nkeys == 0) { std::cout << "No keys!" << std::endl; return; }

      TList *list = tmpFile[j]->GetListOfKeys();
      TIter next((TList*)list);
      TKey  *key;
      Int_t histCounter = 0;
      Int_t keyJumper  = 0;

      while ( (key = (TKey*)next()) ) 
	{
	  keyJumper++;
	  if(keyJumper < keyJumpIndex) continue;

	  TObject *obj = tmpFile[j]->Get(key->GetName());
	  if(!obj) { std::cout << "Could not get object out of file " << i << std::endl; return; }
	  if(obj->IsA() == TDirectory::Class()) continue;
	  if(obj->InheritsFrom(TH1::Class())) 
	    {
	      if(j == 0) ObjArray->Add((TH1*)obj);
	      if(j >= 1) ((TH1*)ObjArray->At(histCounter))->Add((TH1*)obj); //Add like histograms together
	      histCounter++;
	    }
	}

      if(j == 0) Nhist = histCounter;

      // clean up
      if(j >= 1)
	{
	  tmpFile[j]->Close();
	  delete tmpFile[j];
	}

      std::cout << "PYS file " << j+1 << " read in" << std::endl;      
    }

  // write merged histograms
  std::cout << "Writing PYS histograms." << std::endl;

  for(Int_t k=0; k <= Nhist-keyJumpIndex; k++) 
    {
      outputPysFile->cd();
      ((TH1*)ObjArray->At(k))->Write();
    }

  ObjArray->Clear();
  tmpFile[0]->Close();
  delete tmpFile[0];
  */
  std::cout << "Done!" << std::endl;
}
