from ROOT import *
import os
import sqlite3 as sqlite
import time
from ROOT import gStyle
import math
def sumamryBad():
  f=open("summarybad.txt","w")
  for i in range(1,255):
    for j in range(1,49):
      for k in range(1,65):
        fn='DIF%d/Asic%d/channel%d' % (i,j,k)
        if (os.path.exists(fn)):
          fchan=open(fn)
          f.write( "s.SetGain(%d,%d,%d,%d)\n" % (i,j,k,int(fchan.readline())))
          fchan.close()
  f.close() 

def printBad(fn,gain):
  f=TFile(fn)
  vl=getmaindir()
  for idif in range(1,255):
    if ('DIF%d' % idif) in vl: 
      for iasic in range(1,49):
        hfreq=getth1('/DIF%d/Asic%d/Frequency' % (idif,iasic))
        if (hfreq != None):
          #print hfreq.GetName()
          for ichan in range(1,65):
            #print hfreq.GetBinContent(ichan)

            if (hfreq.GetBinContent(ichan)>15):
              gcut=gain
              if  (hfreq.GetBinContent(ichan)>30):
                gcut=gain/2
              if  (hfreq.GetBinContent(ichan)>60):
                gcut=gain/4

              fdir="DIF%d/Asic%d" % (idif,iasic)
              if (not os.path.exists(fdir)):
                try:
                  os.makedirs(fdir)
                except:
                  print ""
              fnchan=fdir+"/channel%d" % ichan
              
              curgain=128
              if os.path.exists(fnchan):
                # read current gain store
                fchan=open(fnchan,'r')
                curgain=int(fchan.read())
                fchan.close()
              print idif,iasic,ichan,curgain,gcut,hfreq.GetBinContent(ichan)
              if (gcut<curgain):
                fchan=open(fnchan,'w')
                fchan.write('%d\n' % gcut)
                fchan.close()

def FeelBad():
  hnoise=TH1F("noise","noise",10000,0.,100.)
  vl=getmaindir()
  for idif in range(1,255):
    if ('DIF%d' % idif) in vl: 
      for iasic in range(1,49):
        hfreq=getth1('/DIF%d/Asic%d/Frequency' % (idif,iasic))
        if (hfreq != None):
          #print hfreq.GetName()
          for ichan in range(1,65):
            #print hfreq.GetBinContent(ichan)
            hnoise.Fill(hfreq.GetBinContent(ichan))
  return hnoise

def getmaindir():
  vdir=[]
  for ik in range(gDirectory.GetListOfKeys().GetEntries()):
    vdir.append(gDirectory.GetListOfKeys().At(ik).GetName())
  return vdir
def getth1(fullname):
  directory=os.path.dirname(fullname)
  name=os.path.basename(fullname)
 
  try:
    gDirectory.cd(directory)
  except:
    return None
  for ik in range(gDirectory.GetListOfKeys().GetEntries()):
    obj=gDirectory.GetListOfKeys().At(ik).ReadObj()
    if obj.IsA().InheritsFrom( "TH1" ):
        if name == obj.GetName():
            return obj

  print "Not found"
  return None

def getth2(fullname):
  directory=os.path.dirname(fullname)
  name=os.path.basename(fullname)
 
  gDirectory.cd(directory)
  for ik in range(gDirectory.GetListOfKeys().GetEntries()):
    obj=gDirectory.GetListOfKeys().At(ik).ReadObj()
    if obj.IsA().InheritsFrom( "TH2" ):
        if name == obj.GetName():
            return obj

  print "Not found"
  return None

def FastEff(fname):
  F = TFile(fname)
  hp = getth1("PlanInTrack")
  print "Number of Tracks:",hp.GetBinContent(101)
  for i in range(1,50):
    if (hp.GetBinContent(101+i)<10):
      continue
    print "|%d|%d|%d|%5.2f|" % (i,hp.GetBinContent(101+i),hp.GetBinContent(151+i),hp.GetBinContent(151+i)/ hp.GetBinContent(101+i)*100)

def PrintEff(fname):
  chtime = []
  chtrack = []
  chfound = []
  result = []
  F = TFile(fname)
  htime = getth1("DifInTime")
  htrack = getth1("PlanInTrack")
  nevent = htime.GetBinContent(1)
  ntrack = htrack.GetBinContent(1)
  cresult =''
  cresult = cresult+ '%s,' % fname  
  cresult = cresult+ '%d,' % nevent  
  cresult = cresult+ '%d,' % ntrack  
  for i in range(4):
      cresult = cresult + '%f,' % (htime.GetBinContent(i+102))
      cresult = cresult + '%f,' % (htime.GetBinContent(i+102)/nevent*100)
      cresult = cresult + '%f,' % (htrack.GetBinContent(i+2))
      cresult = cresult + '%f,' % (htrack.GetBinContent(i+6))
      cresult = cresult + 'etk=%f,' % (htrack.GetBinContent(i+6)/htrack.GetBinContent(i+2)*100.)
      dirname = '/Chamber%d/Synchronised' % (i+1)
      histname = dirname+'/ClusterMapX'
      hx = getth1(dirname,dirname+'/ClusterMapX')
      hy = getth1(dirname,dirname+'/ClusterMapY')
      hcs = getth1(dirname,dirname+'/ClusterSize')
      cresult = cresult + '%f,%f,%f,%f,%f' % (hx.GetMean(),hx.GetRMS(),hy.GetMean(),hy.GetRMS(),hcs.GetMean())
      
  print cresult

def AppendMaps(fname,plan,hextf,hnearf):
  f = TFile(fname)
  f.cd()
  dirname='/Tracking/Plan%d' % plan
  extname= dirname+'/LocalExtrapolationMap'
  nearname= dirname+'/LocalNearestMap'
  hext = getth2(dirname,extname)
  hnear = getth2(dirname,nearname)
  hextf.Add(hext)
  hnearf.Add(hnear)
  f.Close()

def DrawMultpilicty():
  hfmul=TH1F("MultiplicityS","MultiplicityS",51,0.,51.)
  for plan in range(1,47):
    dirname='/OtherTracking/Plan%d' % plan
    mulname= dirname+'/OnTrack/Multiplicity'
    hmul=getth1(mulname)
    mul=hmul.GetMean()
    hfmul.SetBinContent(plan,mul)

  return hfmul

def DrawEff(plan):
  l=[]
  dirname='/OtherTracking/Plan%d' % plan
  extname= dirname+'/LocalExtrapolationMap'
  nearname= dirname+'/LocalFoundMap'
  hext = getth2(extname)
  hnear = getth2(nearname)
  hext8=hext.Rebin2D(8,8,'hext8')
  hext8.SetDirectory(0)
  hnear8=hnear.Rebin2D(8,8,'hnear8')
  hnear8.SetDirectory(0)
  hext8.Draw("TEXT")
  
  hnear8.Draw("TEXT")
 
  heff8 = hnear8.Clone("heff8")
  heff8.SetDirectory(0)
  heff8.Divide(hnear8,hext8,100.,1.)
  heff8.Draw("TEXT")
 
  l.append(hnear8)
  l.append(hext8)
  l.append(heff8)
  heffsum=TH1F("Summary%d" % plan ,"Summary for plan %d " % plan,404,85.,101.)
  st = ''
  for i in range(8):
    for j in range(8):
      st = st + '%f ' % heff8.GetBinContent(i+1,j+1)
      heffsum.Fill(heff8.GetBinContent(i+1,j+1))
  #print '%s' % st
  l.append(heffsum)
  print plan,heffsum.GetMean()
  return l

def DrawSummary(run,i):
  c=TCanvas("Resume","resume",800,800)
  c.Divide(2,2)
  gStyle.SetOptFit(1)
  gStyle.SetOptStat(0)
  l=GetEff(i)
  c.Clear()
  c.Divide(2,2)
  l[0].SetTitle("Found Position (%d,%d)" % (run,i) )
  l[1].SetTitle("Extrapolated Position (%d,%d)" % (run,i) )
  l[2].SetTitle("Local Efficiency (%d,%d)" % (run,i) )
  l[3].SetTitle("Mean Efficiency (%d,%d)" % (run,i) )
  l[4].SetTitle("Local Multiplicity (%d,%d)" % (run,i) )
  l[5].SetTitle("Mean Multiplicity (%d,%d)" % (run,i) )
  l[4].GetZaxis().SetRangeUser(0.5,4.)
#    c.cd(1);l[0].SetStats(0);l[0].Draw("COLZ")
#    c.cd(2);l[1].SetStats(0);l[1].Draw("COLZ")
  c.cd(1);l[2].SetStats(0);l[2].Draw("COLZ")
  c.cd(2);l[3].Fit("gaus","","",80.,100.)
  c.cd(3);l[4].SetStats(0);l[4].Draw("COLZ")
  c.cd(4);l[5].Fit("gaus","","",0.5,2.7)
  c.Update()
  c.Modified()
  c.SaveAs("Summary_%d_Plan%d.bmp" % (run,i))
  c.SaveAs("Summary_%d_Plan%d.pdf" % (run,i)) 
  time.sleep(3)
def GetEff(dirp,plan):
  l=[]
  dirname=dirp+'/Plan%d' % plan
  extname= dirname+'/ext'
  nearname= dirname+'/found'
  nearname1= dirname+'/found1'
  nearname2= dirname+'/found2'
  mulname= dirname+'/mul'
  hext = getth2(extname)
  hnear = getth2(nearname)
  hnear1 = getth2(nearname1)
  hnear2 = getth2(nearname2)
  hmul = getth2(mulname)
  hext.Draw("COLZ")
  
  hnear.Draw("COLZ")
  hnearm=hnear.Clone("hnearm")
  rs=2
  if (hext.GetEntries()<4E3):
    rs=4
  if (hext.GetEntries()<1E5):
    hext.Rebin2D(rs,rs)
    hnear.Rebin2D(rs,rs)
    #hmul.Rebin2D(rs,rs)
  heff = hnear.Clone("heff")
  heff.SetDirectory(0)
  heff.Divide(hnear,hext,100.,1.)
  hmulc = hmul.Clone("hmulc")
  hmulc.SetDirectory(0)
  hmulc.Divide(hmul,hnearm,1.,1.)
  hmulc.Draw("COLZ")
 
  l.append(hnear)
  l.append(hext)
  l.append(heff)
  heffsum=TH1F("Summary%d" % plan ,"Summary for plan %d " % plan,424,10.,105.5)
  hmulsum=TH1F("Summul%d" % plan ,"Multiplicity for plan %d " % plan,200,-0.1,7.1)
  st = ''
  ntk=0;
  for i in range(2,heff.GetXaxis().GetNbins()-1):
    for j in range(2,heff.GetYaxis().GetNbins()-1):
      st = st + '%f ' % heff.GetBinContent(i+1,j+1)
      ntk=ntk+hext.GetBinContent(i+1,j+1)
      # Cut a 15 change en 5
      if (hext.GetBinContent(i+1,j+1)>25):
        heffsum.Fill(heff.GetBinContent(i+1,j+1))
  for i in range(2,hmulc.GetXaxis().GetNbins()-1):
    for j in range(2,hmulc.GetYaxis().GetNbins()-1):
      if (hnearm.GetBinContent(i+1,j+1)):
        hmulsum.Fill(hmulc.GetBinContent(i+1,j+1))
  #print '%s' % st
  l.append(heffsum)
  l.append(hmulc)
  l.append(hmulsum)
  #print plan,heffsum.GetMean(),ntk
  l.append(plan)
  l.append(heffsum.GetMean())
  l.append(ntk)
  l.append(hext.GetEntries())
  l.append(hnear.GetEntries())
  l.append(100.*hnear.GetEntries()/hext.GetEntries())
  l.append(hnear2.GetEntries())
  l.append(hmulsum.GetMean())
  return l

def drawBench(fname,fch,lch,c2):
  f=TFile(fname)
  res=[]
  c2.Update()
  print (lch+1-fch)/2
  ip=1
  for i in range(fch,lch+1):
    l=GetEff("/Principal",i)
    print l
    c2.cd(ip)
    c2.Update()
    ip=ip+1
    print ip
    res.append(l[2])
    l[2].Draw("COLZ")
    c2.Update()
    try:
      mode=int(raw_input('Input:'))
    except ValueError:
      print "Not a number"
  return [c2,res]
def drawEff(c,l):
 
  
  c.cd(1);l[0].Draw("COLZ")
  c.cd(2);l[1].Draw("COLZ")
  c.cd(3);l[2].Draw("COLZ")
  c.cd(4);l[4].Draw("COLZ")
  c.cd(5);l[3].Draw("")
  c.cd(6);l[5].Draw("")
  c.Update();
  
def getDifList(chamber):
  difl={}
  path="/Chamber%d" % chamber
  gDirectory.cd(path)
  for ik in range(gDirectory.GetListOfKeys().GetEntries()):
    obj=gDirectory.GetListOfKeys().At(ik).ReadObj()
    #print obj.GetName()
    idx_dif=obj.GetName().find("DIF")
    if (idx_dif!=-1):
      print obj.GetName()[idx_dif+3:len(obj.GetName())]
      difl[int(obj.GetName()[idx_dif+3:len(obj.GetName())])]=path+"/"+obj.GetName()
      #difl.append(a)
  return difl
def tagDIFHits(hpath,idd,setupid=1,cut=0,thr=1):
     """
     MORE USEFULL
     tag for DIF 'idd' all pads with more than 'cut' hits in the '/DIF#idd#/Hits1' histogram
     It generates:
       - the UPDATE commands of masks for sqlite DB for setup id 'setupid'. The y are stored in /tmp/Mask#idd#_sid#setupid#.sql
       - the list of Asics with their Mask usable with the Oracle DB tools. They are stored in /tmp/DIFMask#idd#.txt
     """

     hname=hpath+"/Hits%d" % (thr)
     h=getth1(hname)
     if (h==None):
       return
     hm=TH1F(hname+"Mean",hname+"Mean",100,0.,h.GetMaximum()*1.1)
     asic=[]
     for i in range(48):
        asic.append(0XFFFFFFFFFFFFFFFF)
     for i in range(3071):
        if (h.GetBinContent(i+1)>0):
          hm.Fill(h.GetBinContent(i+1),1.)
     theCut=-100.
     for i in range(100):
       if (hm.GetBinContent(100-i)>10):
         theCut=hm.GetBinCenter(100-i)
         break

     if (theCut>200 and cut!=0):
       cut=theCut

     if (theCut>200):
       cut=theCut
     else:
       cut=200
     print "DIF ",idd," Cut > ",theCut,cut
     for i in range(3071):
        if ((cut==0 and h.GetBinContent(i+1)> 5 * hm.GetMean()) or (h.GetBinContent(i+1)>cut and cut>0)):
            #print i+1,(i+1)%64,i/64
           ia=i/64
           ip = i%64
            #print i,ia,ip, (1<<ip), ~(1<<ip)
           asic[ia]=asic[ia] & ~(1<<ip)
     fout=open("/tmp/Mask%d_sid%d.sql" % (idd,setupid),'w+')
     flist=open("/tmp/DIFMask%d.txt" % idd,'w+')
     for i in range(48):
        if (asic[i] == 0XFFFFFFFFFFFFFFFF):
           continue
        fout.write("UPDATE HR2 SET Mask0='0x%x',Mask1='0x%x',Mask2='0x%x' WHERE DIF_NUM='%d' AND Header='%d' AND SETUP_ID='%d';\n" % (asic[i],asic[i],asic[i],idd,(i+1),setupid))
        flist.write("%d;%d;'0x%x'\n" % (idd,(i+1),asic[i]))
     fout.close()
     flist.close()
           
     return hm  
def PadConvert(asicid,ipad):
     MapJLargeHR2=(1,1,2,2,3,3,4,4,5,5,6,6,7,7,4,3,2,0,0,1,0,5,6,7,7,6,5,4,3,2,1,0,0,1,2,3,4,5,6,7,7,6,0,0,1,1,0,2,3,4,5,7,7,6,6,5,5,4,4,3,3,2,2,1)
     MapILargeHR2=(1,0,1,0,1,0,1,0,0,1,0,1,0,1,2,2,2,0,1,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,6,5,6,7,5,5,5,5,6,7,6,7,6,7,6,7,7,6,7,6,7)
     AsicShiftI=(	0,	
                        0,	0,	0,	0,	
                        8,	8,	8,	8,	
                        16,	16,	16,	16,	
                        24,	24,	24,	24,	
                        32,	32,	32,	32,	
                        40,	40,	40,	40,	
                        48,	48,	48,	48,	
                        56,	56,	56,	56,	
                        64,	64,	64,	64,	
                        72,	72,	72,	72,	
                        80,	80,	80,	80,	
                        88,	88,	88,	88)
     AsicShiftJ=(	0,		
                        0,	8,	16,	24,
                        24,	16,	8,	0,	
                        0,	8,	16,	24,	
                        24,	16,	8,	0,	
                        0,	8,	16,	24,	
                        24,	16,	8,	0,	
                        0,	8,	16,	24,	
                        24,	16,	8,	0,	
                        0,	8,	16,	24,	
                        24,	16,	8,	0,	
                        0,	8,	16,	24,	
                        24,	16,	8,	0
                        )

     i = MapILargeHR2[ipad]+AsicShiftI[asicid]+1;
     j = MapJLargeHR2[ipad]+AsicShiftJ[asicid]+1;
     j=33-j;
     return (i,j)
 
def drawChamberMap(chnum,consdb,level=1):
  """
  MORE USEFULL
  tag for DIF 'idd' all pads with more than 'cut' hits in the '/DIF#idd#/Hits1' histogram
  It generates:
  - the UPDATE commands of masks for sqlite DB for setup id 'setupid'. The y are stored in /tmp/Mask#idd#_sid#setupid#.sql
  - the list of Asics with their Mask usable with the Oracle DB tools. They are stored in /tmp/DIFMask#idd#.txt
  """
  hname="/Chamber%d/Hits%d" % (chnum,level)
  hm=TH2F(hname+"Map",hname+"Map",96,0.,96.,96,0.,96.)

  contest =sqlite.connect(consdb)
  st1="SELECT number,dif1,dif2,dif3 FROM CASSETTE  WHERE number='%d'" % chnum
  print st1
  c=contest.cursor()
        #       print str_cmd
  c.execute(st1)
  idt=0
  idm=0
  idb=0
  for row in c:
    num=int(row[0])
    idt=int(row[1])
    idm=int(row[2])
    idb= int(row[3])
  c.close()

  print idt,idm,idb
  hdt=None
  hdm=None
  hdb=None
  if (idt>0):
    try:
      hdt=getth1("/DIF%d/Hits%d" % (idt,level))
    except:
      print idt,"not found"
  if (idm>0):
    hdm=getth1("/DIF%d/Hits%d" % (idm,level))
  if (idb>0):
    try:
      hdb=getth1("/DIF%d/Hits%d" % (idb,level))
    except:
      print idb,"not found"
  for i in range(3071):
    ia=i/64
    ip = i%64
    x=PadConvert(ia+1,ip)
         #print ia,ip,x,h.GetBinContent(i+1)
    if (hdt!=None):
      hm.Fill(x[0]*1.,x[1]*1.+64.,hdt.GetBinContent(i+1))
    if (hdm!=None):
      hm.Fill(x[0]*1.,x[1]*1.+32.,hdm.GetBinContent(i+1))
    if (hdb!=None):
      hm.Fill(x[0]*1.,x[1]*1.,hdb.GetBinContent(i+1))
  return hm  

def EventDisplay(fname,dirtk="/OtherTracking"):
    """
     Display last 100 tracks
     dirtk= directory where the xh,yh histograms are 
    """
    #gROOT.GetStyle("Default").SetCanvasPreferGL(True);
    #otherCanvas = TCanvas("OtherCanvas","OtherTracks",500,900)
    F = TFile(fname)
    otherCanvas1 = TCanvas("OtherCanvas1","OtherTracks1",600,900)
    max_chamber=0
    plots=[]
    while ( 1 > 0 ):
        h_xh=getth1(dirtk+"/xh")
        h_yh=getth1(dirtk+"/yh")
        if (max_chamber==0):
            for iev in range(100):
                for ich in range(50):
                    if (h_xh.GetBinContent(iev*50+(ich+1))!=0):
                        if (max_chamber<(ich+1)):
                            max_chamber=ich+1
            for i in range(max_chamber):
                hn="Plan_%d" % (i+1)
                h=TH2F(hn,hn,50,0.,100.,50,0.,100.)
                plots.append(h)
            d3d=TH3F("CubeView","CubeView",max_chamber*10,0.,max_chamber*9.,50,0.,100.,50,0.,100.);
            d3dx=TH2F("CubeViewX","CubeViewX",max_chamber*10,0.,max_chamber*9.,50,0.,100.);
            d3dy=TH2F("CubeViewY","CubeViewY",max_chamber*10,0.,max_chamber*9.,50,0.,100.)

        #otherCanvas.Clear()
        otherCanvas1.Clear()
        otherCanvas1.Divide(1,3)
        #c= otherCanvas
        #c.Divide(1,max_chamber)
        print "on est la"
        zshift=2.8
        for iev in range(100):
            event=iev*50;
            print iev
            for ich in range(max_chamber):
                if (h_xh.GetBinContent(event+ich+1)!=0):
                    plots[ich].Fill(h_xh.GetBinContent(event+ich+1),h_yh.GetBinContent(event+ich+1))
                    d3d.Fill(ich*zshift+0.1,h_xh.GetBinContent(event+ich+1),h_yh.GetBinContent(event+ich+1))
                    d3dx.Fill(ich*zshift+0.1,h_xh.GetBinContent(event+ich+1))
                    d3dy.Fill(ich*zshift+0.1,h_yh.GetBinContent(event+ich+1))

 #           for x in range(max_chamber):
 #               c.cd(x+1)
 #               plots[x].SetStats(False)
 #               plots[x].Draw('COLZ')
 #           c.Update()
            otherCanvas1.cd(1)
            d3d.SetFillColor(2)
            d3d.SetLineColor(2)
            d3d.Draw('BOX')

            otherCanvas1.cd(2)
            d3dx.SetFillColor(2)
            d3dx.Draw('BOX')
            otherCanvas1.cd(3)
            d3dy.SetFillColor(2)
            d3dy.Draw('BOX')

            otherCanvas1.Update()

            time.sleep(1./2.)
            for ich in range(max_chamber):
                plots[ich].Reset()
            d3d.Reset()
            d3dx.Reset()
            d3dy.Reset()
def gainDIFHits(hpath,idd,cut=0,thr=1):
     """
     MORE USEFULL
     tag for DIF 'idd' all pads with more than 'cut' hits in the '/DIF#idd#/Hits1' histogram
     It generates:
       - the UPDATE commands of masks for sqlite DB for setup id 'setupid'. The y are stored in /tmp/Mask#idd#_sid#setupid#.sql
       - the list of Asics with their Mask usable with the Oracle DB tools. They are stored in /tmp/DIFMask#idd#.txt
     """

     hname=hpath+"/Hits%d" % (thr)
     h=getth1(hname)
     if (h==None):
       return
     hm=TH1F(hname+"Mean",hname+"Mean",100,0.,h.GetMaximum()*1.1)
     asic=[]
     for i in range(48):
        asic.append(0XFFFFFFFFFFFFFFFF)
     for i in range(3071):
        if (h.GetBinContent(i+1)>0):
          hm.Fill(h.GetBinContent(i+1),1.)
     theCut=-100.
     for i in range(100):
       if (hm.GetBinContent(100-i)>10):
         theCut=hm.GetBinCenter(100-i)
         break

     if (theCut>200):
       cut=theCut
     else:
       cut=200
     print "DIF ",idd," Cut > ",theCut," will cut to ",cut
     flist=open("/tmp/DIFGain%d.txt" % idd,'w+')
     for i in range(3071):
        if ((cut==0 and h.GetBinContent(i+1)> 5 * hm.GetMean()) or (h.GetBinContent(i+1)>cut and cut>0)):
            #print i+1,(i+1)%64,i/64
           ia=i/64
           ip = i%64
           #print idd,ia,ip

           flist.write("%d;%d;%d\n" % (idd,ia,ip))
     flist.close()
           
     return hm  
def gainDIFAsics(hpath,idd,cut):
     """
     MORE USEFULL
     tag for DIF 'idd' all pads with more than 'cut' hits in the '/DIF#idd#/Hits1' histogram
     It generates:
       - the UPDATE commands of masks for sqlite DB for setup id 'setupid'. The y are stored in /tmp/Mask#idd#_sid#setupid#.sql
       - the list of Asics with their Mask usable with the Oracle DB tools. They are stored in /tmp/DIFMask#idd#.txt
     """

     hname=hpath+"/AsicOccupancy"
     h=getth1(hname)
     if (h==None):
       return
     hm=TH1F(hname+"Mean",hname+"Mean",100,0.,h.GetMaximum()*1.1)

     flist=open("/tmp/ASICGain%d.txt" % idd,'w+')
     for i in range(48):
       hm.Fill(h.GetBinContent(i+1))
       if (h.GetBinContent(i+1)>cut):
         g1=int(192*600./(4*h.GetBinContent(i+1)))
         print "======>",idd,i+1,h.GetBinContent(i+1),4*h.GetBinContent(i+1)/600.,g1

         flist.write("oa.RescaleGain(192,%d,%d,%d)\n" % (g1,idd,i+1))
     flist.close()
     return hm
def fitCB(hr,mhr=-1E9,shr=-1E9,c=None):
    x=RooRealVar("x", "Number of Hit", hr.GetXaxis().GetXmin(),hr.GetXaxis().GetXmax())
    l = RooArgList(x)
    data=RooDataHist("data", "Number of Hit data", l, hr)
    frame=x.frame()
    data.plotOn(frame)
    if c!=None:
      c.cd(3)
    else:
       c=TCanvas("Fit results")
       c.cd()
    data.plotOn(frame).Draw()
    if mhr==-1E9:
      mhr=hr.GetMean()
    if shr==-1E9:
      shr=hr.GetRMS();
    mean=RooRealVar("mean", "mean",mhr,mhr-2*shr,mhr+2*shr)
    sigma=RooRealVar("sigma", "sigma",shr,shr*0.5,2*shr)
    alpha=RooRealVar("alpha", "alpha", 2., 0.,300)
    nth=RooRealVar("nth", "nth", 2., 0.,300)
    rcb=RooCBShape("rcb","rcb",x,mean,sigma,alpha,nth)
    rcb.fitTo(data)
    rcb.paramOn(frame)
    rcb.plotOn(frame)
    frame.Draw()
    
    return [frame.chiSquare(),mean.getVal(),sigma.getVal(),alpha.getVal(),nth.getVal()]
#    if (fo!=None):
#        fo.write("%.1f| %.1f| %.1f | %.1f| %.1f|" % (frame.chiSquare(),mean.getVal(),sigma.getVal(),alpha.getVal(),nth.getVal()))
    if (fo!=None):
        fo.write("%.3f| %.3f|%.3f| %.3f|" % (mean.getVal(),mean.getError(),sigma.getVal(),sigma.getError()))
    c.SaveAs(hr.GetTitle()+"_ROOFIT.png");time.sleep(1)

def FWHM(h1,emean=1.):
  
  while (h1.GetBinContent(h1.FindFirstBinAbove(h1.GetMaximum()/2))<50):
    h1.Rebin(2)
    print h1.GetMaximum()
  print "Maximum ",h1.GetMaximum()

  bin1=h1.FindFirstBinAbove(h1.GetMaximum()/2);
  bin2=h1.FindLastBinAbove(h1.GetMaximum()/2);
  fwhm=h1.GetBinCenter(bin2)-h1.GetBinCenter(bin1);
  sigma=fwhm/math.sqrt(math.log(256))
  mode=h1.GetBinCenter(h1.GetMaximumBin())
  g1 = TF1("m1","gaus",mode-3*sigma,mode+3*sigma)
  g1.SetParameter(1,mode)
  g1.SetParameter(2,sigma)
  h1.Fit("m1","","",mode-1.5*sigma,mode+2*sigma)
  print "On trouve ",fwhm,sigma,mode,mode-sigma,mode+2*sigma
  #time.sleep(5)
  l=[bin1,bin2,fwhm,sigma,mode,g1.GetChisquare(),g1.GetParameter(0),g1.GetParameter(1),g1.GetParameter(2)]
 
  l1=fitCB(h1,mode,sigma)
  l=l+l1
  print l

  return [emean,(mode-emean)*100/emean,sigma/mode*100,(g1.GetParameter(1)-emean)/emean*100,g1.GetParameter(2)/g1.GetParameter(1)*100,(l1[1]-emean)*100./emean,l1[2]/l1[1]*100.]

