from ROOT import *
defped=[31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31]

def fitped(run,tdc,vthmin,vthmax,old=defped):
  ped=[]
  for i in range(32):
    ped.append(0)
  f82=TFile("/tmp/toto%d.root" % run);
  f82.cd("/run%d/TDC%d" % (run,tdc));
  c1=TCanvas();
  fout=open("summary_pedestal_%d_tdc%d.txt" % (run,tdc),"w");
  fout.write("+--+-----+-----+-----+ \n");
  gStyle.SetOptFit();
  hmean=TH1F("hmean","Summary %d %d " %(run,tdc),vthmax-vthmin+1,vthmin,vthmax)
  hnoise=TH1F("hnoise","Summary noise %d %d " %(run,tdc),100,0.,30.)
  scfit=TF1("scfit","[0]*TMath::Erfc((x-[1])/[2])",vthmin+1,vthmax);
  for ip in range(0,16):
    
      hs=f82.Get("/run%d/TDC%d/vth%d" % (run,tdc,ip));
      if (hs.GetEntries()==0):
        continue
      #hs.Rebin(2)
      for i in range(1,hs.GetNbinsX()):
          if (hs.GetBinContent(i)==0):
              if (hs.GetBinContent(i-1)!=0 and hs.GetBinContent(i+1)!=0):
                  hs.SetBinContent(i,(hs.GetBinContent(i-1)+hs.GetBinContent(i+1))/2.)
      scfit.SetParameter(0,60);
      scfit.SetParameter(1,hs.GetMean()+5);
      scfit.SetParameter(2,10.);



      hs.GetXaxis().SetRangeUser(vthmin-1,vthmax);
      hs.Fit("scfit","Q","",vthmin+2,vthmax);
      hs.GetXaxis().SetRangeUser(vthmin-1,scfit.GetParameter(1)+60);
      hs.Draw()
      fout.write("|%2d|%5.1f|%5.1f|%5.2f| \n" % (ip,scfit.GetParameter(0),scfit.GetParameter(1),scfit.GetParameter(2)));
      ipr=0
      if (ip%2==1):
        ipr=ip/2
      else:
        ipr=31-ip/2

      ped[ipr]=scfit.GetParameter(1)
      print ip,ipr,ped[ipr]
      hmean.Fill(scfit.GetParameter(1))
      hnoise.Fill(scfit.GetParameter(2))
      c1.SaveAs("Run%d_Strip%d.png" % (run,ip));
      val = raw_input()
  c1.cd()
  hmean.Draw()
  c1.Update()
  c1.SaveAs("Summary_%d_TDC%d.png" % (run,tdc));
  val = raw_input()
  hnoise.Draw()
  c1.Update()
  c1.SaveAs("Summary_Noise_%d_TDC%d.png" % (run,tdc));

  fout.write("+--+-----+-----+-----+ \n");
  fout.close()
  print ped
  val = raw_input()
  med=525
  dac=ped
  for i in range(32):
    if (ped[i]==0):
      continue;
    dac[i]=int(round(old[i]+(med-ped[i])/2.97))
  print dac
  return dac
