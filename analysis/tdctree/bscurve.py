import accessHisto as ah
from ROOT import *
#f=TFile("scurve735113.root")
defped=[31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31]
c=TCanvas()
MAX_CHAN=16

def calcthm(run,tdc,num):
   
    h=ah.getth1("/run%d/TDC%d/vth%d" % (run,tdc,num))
   
    i20=0;i180=0;
    ped=0;width=0;
    h.Rebin(4)
    c.cd()
    h.Draw()
    c.Update()
    c.SaveAs("Run%d_TDC%d_Channel%d.png" % (run,tdc,num))
    raw_input("Please hit return ...")
    if (h.GetEntries()>100):
        ped=h.GetMean()
        width=h.GetRMS()
        #print ped,width
    else:
        ped=0
        width=0;
        #print num," is dead"
    return (ped,width)

def calcth(run,tdc,num):
   
    h=ah.getth1("/run%d/TDC%d/vth%d" % (run,tdc,num))
   
    i20=0;i180=0;
    ped=0;width=0;
    h.Rebin(4)
    c.cd()
    h.Draw()
    c.Update()
    c.SaveAs("Run%d_TDC%d_Channel%d.png" % (run,tdc,num))
    raw_input("Please hit return ...")
    ax=h.GetXaxis()
    for i in range(0,ax.GetNbins()):
        if (h.GetBinContent(ax.GetNbins()-i)>10 and h.GetBinContent(ax.GetNbins()-i-1)>10):
            i20=i;break
    for i in range(0,ax.GetNbins()):
        if (h.GetBinContent(ax.GetNbins()-i)>90 ):
            i180=i;break

    if (i20>0 and i180>0):
        ped=(ax.GetBinCenter(ax.GetNbins()-i20)+ax.GetBinCenter(ax.GetNbins()-i180))/2
        width=ax.GetBinCenter(ax.GetNbins()-i20)-ax.GetBinCenter(ax.GetNbins()-i180)
        #print ped,width
    else:
        ped=0
        width=0;
        #print num," is dead"
    return (ped,width)
def calcall(run,tdc,old=defped):
    med={}
    med[1]=315
    med[2]=313
    
    hp=TH1F("hp%d" % tdc,"Pedestal TDC %d" % tdc,32,0.,32.)
    hw=TH1F("hw%d" % tdc,"Width TDC %d" % tdc,32,0.,32.)
    pi=9999;pa=0;
    ped={}
    width={}
    
    for i in range(0,MAX_CHAN):
        ped[i]=0
        width[i]=0
        a=calcth(run,tdc,i)
        if (a[0]>0):
            if (a[0]>pa and a[0]<500):
                pa=a[0]
            if (a[0]<pi and a[0]>280):
                pi=a[0]
            ped[i]=a[0]
            width[i]=0
        print i,a[0],a[1]
        istrip=0
        ipr=0
        if (i%2==0):
            istrip=i/2
            ipr=istrip
        else:
            istrip=i/2+16
            ipr=31-i/2
        hp.Fill(i+0.1,a[0])
        hw.Fill(i+0.1,a[1])
        
    med[tdc]=(pa+pi)/2
    print "MEDIANE", med[tdc],pa,pi
    med[tdc]=530
    dacn={}
    for i in range (0,32):
        dacn[i]=31
    for i in range(0,MAX_CHAN):
        istrip=0
        ipr=0
        if (i%2!=0):
            istrip=i/2
            ipr=istrip
        else:
            istrip=i/2+16
            ipr=31-i/2
        dac=1
        if (ped[i]>0):
            #dac=old[ipr]+1.0*(med[tdc]-ped[i])*0.9/1.46
            dac=old[ipr]+(med[tdc]-ped[i])/2.97
        #print i,ped[i],width[i],dac
        dacn[ipr]=int(dac)
        if (dacn[ipr]<1):
            dacn[ipr]=1
        if (dacn[ipr]>63):
            dacn[ipr]=63
    st="\"6bDac\":["
    for i in range (0,31):
        st=st+"%d," % dacn[i]
    print st,dacn[31],"],"
    return (hp,hw)
