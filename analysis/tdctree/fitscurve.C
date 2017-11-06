{
  f82=TFile("/tmp/toto737294.root");
  f82.cd("/run737294/TDC5");
  TCanvas c1;
  TF1 *myfit = new TF1("myfit","[0]*TMath::Erfc((x-[1])/[2])",529,700);
  myfit->SetParameter(0,120);
  myfit->SetParameter(1,530);
  myfit->SetParameter(2,2);
  //vth11.Rebin(2);
  //vth6.Rebin(2);
  //vth2.Rebin(2);
  vth2->Fit("myfit","Q","",511,540);
  printf("|2| %f| %f| %f| \n",myfit->GetParameter(0),myfit->GetParameter(1),myfit->GetParameter(2));
  myfit->SetParameter(0,120);
  myfit->SetParameter(1,530);
  myfit->SetParameter(2,2);

  vth6->Fit("myfit","Q","",510,690);
  printf("|6| %f| %f| %f| \n",myfit->GetParameter(0),myfit->GetParameter(1),myfit->GetParameter(2));

  myfit->SetParameter(0,120);
  myfit->SetParameter(1,530);
  myfit->SetParameter(2,2);

  vth11->Fit("myfit","Q","",511,690);
  printf("|11| %f| %f| %f| \n",myfit->GetParameter(0),myfit->GetParameter(1),myfit->GetParameter(2));

  vth11.GetXaxis().SetRangeUser(511,700.);
  vth6.GetXaxis().SetRangeUser(511,700.);
  vth2.GetXaxis().SetRangeUser(511,700.);
  gStyle->SetOptFit();
  vth2.Draw();
  c1.SaveAs("Run737294_Strip2.png");
  gStyle->SetOptFit();
  vth6.Draw();
  c1.SaveAs("Run737294_Strip6.png");
  gStyle->SetOptFit();
  vth11.Draw();
  c1.SaveAs("Run737294_Strip11.png");
  
}
