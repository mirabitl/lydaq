import sqlite3
import json
import matplotlib.pyplot as plt
import time
import matplotlib.gridspec as gridspec
from ROOT import TCanvas, TGraph
from ROOT import gROOT
from array import array

#plt.figure(num=None, figsize=(12, 12), dpi=80, facecolor='w', edgecolor='k')

class sla:
  """
  Handle all application definition and parameters
  """
  def __init__(self,name):
      self.name=name
      self.conn=None
      self.curs=None
      self.runlist={}
      self.run=0
      self.graphs={}
      self.connect(name)
      self.canvas=TCanvas('csla', 'Slow Control display', 200, 110, 700, 700 )
      self.canvas.Draw()
      self.canvas.Update()
  def connect(self,name):
    self.conn = sqlite3.connect('slowdb.sqlite')
    self.conn.text_factory = str
    self.curs = self.conn.cursor()

  def parse(self,cond=None):
    print cond
    if (cond==None):
        self.curs.execute("SELECT * FROM RESULTS");
    else:
        self.curs.execute('SELECT * FROM RESULTS WHERE %s'% cond);
    v=self.curs.fetchall()
    return v

  def selectdate(self,hw,first,last):
    cond='HARDWARE="%s" AND DATETIME(CTIME,"unixepoch")>"%s" AND DATETIME(CTIME,"unixepoch")<"%s"' % (hw,first,last)
    v=self.parse(cond);
    a=[]
    for x in v:
        a.append([x[3],json.loads(x[4].decode('latin-1').encode("utf-8"))])
    return a

  def selectepoch(self,hw,first,last):
    cond='HARDWARE="%s" AND CTIME>%d AND CTIME<%d' % (hw,first,last)
    v=self.parse(cond);
    a=[]
    for x in v:
        a.append([x[3],json.loads(x[4].decode('latin-1').encode("utf-8"))])
    return a
  def buildRunList(self,first,last):
    v=self.selectdate("FDAQ",first,last)
    tbeg=0
    tlast=0
    rlast=0
    self.runlist={}
    for x in v:
        tr=x[0]
        run=x[1]['builder']['run']
        state=x[1]['STATE']

        if (run!=rlast and state=="RUNNING"):
            #print state
            if (rlast!=0):
                #print rlast,tbeg,tlast
                self.runlist[rlast]=[tbeg,tlast]
            rlast=run
            tbeg=tr
        if (state == "RUNNING"):    
            tlast=tr
    #print rlast,tbeg,tlast
    self.runlist[rlast]=[tbeg,tlast]
  def drawRun(self,run):
      if (len(self.runlist)==0):
          print "Build run list first "
          return
      if (self.runlist[run]==None):
          print "unknown run ",run
          return
      self.run=run
      self.drawSlowControl(self.runlist[run][0],self.runlist[run][1])
      self.run=0
  def drawScan(self,rfirst,rlast):
      if (len(self.runlist)==0):
          print "Build run list first "
          return
      if (self.runlist[rfirst]==None):
          print "unknown run ",rfirst
          return
      if (self.runlist[rlast]==None):
          print "unknown run ",rlast
          return


      self.drawSlowControl(self.runlist[rfirst][0]-30,self.runlist[rlast][1]+30)
      self.run=0
  def drawDate(self,dfirst,dlast):
    pattern = '%Y-%m-%d %H:%M:%S'
    rfirst = int(time.mktime(time.strptime(dfirst, pattern)))
    rlast = int(time.mktime(time.strptime(dlast, pattern)))
    self.drawSlowControl(rfirst,rlast)
    
  def buildTGraph(self,title,vx,vy,tx,ty):
    r_ti, r_p = array( 'd' ), array( 'd' )
    for x in vx:
      r_ti.append(x)
    for y in vy:
      r_p.append(y)
    gr = TGraph( len(vx), r_ti, r_p )
    gr.SetLineColor( 1 )
    gr.SetLineWidth( 1 )
    gr.SetMarkerColor( 2 )
    gr.SetMarkerStyle( 21 )
    gr.SetMarkerSize( 0.4 )
    gr.SetTitle(title)
    gr.GetXaxis().SetTitle(tx )
    gr.GetYaxis().SetTitle( ty )
    return gr
  def drawSlowControl(self,first,last):
      #v=self.selectepoch("BMP",self.runlist[run][0],self.runlist[run][1])
      fig=plt.figure(1,figsize=(12,12),facecolor='w')

      v=self.selectepoch("BMP",first,last)
      #print v
      x_t=[]
      y_p=[]
      z_t=[]
      c1 = TCanvas( 'c1', 'A Simple Graph Example', 200, 10, 700, 700 )

      c1.SetFillColor( 0 )
      c1.SetGrid()
      c1.Divide(2,2)

      r_ti, r_p,r_t = array( 'd' ), array( 'd' ),array('d')


      
      for x in v:
           x_t.append(x[0]-first)
           y_p.append(x[1]['pressure'])
           #print x[1]['pressure']
           z_t.append(x[1]['temperature'])
           r_ti.append(x[0]-first)
           r_p.append(x[1]['pressure'])
           #print x[1]['pressure']
           r_t.append(x[1]['temperature'])

      g1=self.buildTGraph('Pressure vs t (s)',x_t,y_p,'t(s)','P(mbar)')
      g2=self.buildTGraph('GIF++ Temperature vs t (s)',x_t,z_t,'t(s)','T(C)')
      c1.cd(1);g1.Draw('APC');c1.Update();
      c1.cd(2);g2.Draw('APC');c1.Update();
      self.graphs["BMP-Pressure"]=g1
      self.graphs["BMP-Temperature"]=g2
      plt.subplot(221)
      plt.plot(x_t,y_p,color='r')
      plt.ylabel('Pressure (mbar)')
      plt.xlabel('time (s)')

      plt.subplot(222)
      plt.plot(x_t,z_t)
      plt.ylabel('Temperature BMP (C)')
      plt.xlabel('time (s)')
      
      v=self.selectepoch("HIH8000",first,last)
      #print v
      x_t=[]
      y_p=[]
      z_t=[]
      r_ti, r_h0,r_h1 = array( 'd' ), array( 'd' ),array('d')
      for x in v:
           x_t.append(x[0]-first)
           y_p.append(x[1]['humidity0'])
           #print x[1]['pressure']
           z_t.append(x[1]['humidity11'])
           r_ti.append(x[0]-first)
           r_h0.append(x[1]['humidity0'])
           #print x[1]['pressure']
           r_h1.append(x[1]['humidity11'])
      g3=self.buildTGraph('Humidity 0 vs t (s)',x_t,y_p,'t(s)','H(%)')
      g4=self.buildTGraph('Humidity 1 vs t (s)',x_t,z_t,'t(s)','H(%)')
      c1.cd(3);g3.Draw('APC');c1.Update();
      c1.cd(4);g4.Draw('APC');c1.Update();
      self.graphs["HIH-Inlet"]=g3
      self.graphs["HIH-Outlet"]=g4

      
      #plt.subplot(223)
      #plt.plot(x_t,y_p,color='r')
      #plt.ylabel('Humidity Inlet (%)')
      #plt.xlabel('time (s)')

      #plt.subplot(224)
      #plt.plot(x_t,z_t)
      #plt.ylabel('Humidity Outlet (%)')
      #plt.xlabel('time (s)')
      #plt.show()
      if (self.run==0):
          #plt.savefig('SlowControl_%s.png' % time.strftime('%Y-%m-%d_%H-%M-%S', time.localtime(first)))
        c1.SaveAs('SlowControl_%s.png' % time.strftime('%Y-%m-%d_%H-%M-%S', time.localtime(first)))
      else:
          #plt.savefig('SlowControl_%d.png' % self.run)
        c1.SaveAs('SlowControl_%d.png' % self.run)
      #plt.gcf().clear()
      #fig=plt.figure(num=None, figsize=(21, 12), dpi=60, facecolor='w', edgecolor='k')

      c2 = TCanvas( 'c2', 'HV studies', 200, 10, 1200, 1200 )

      c2.SetFillColor( 0 )
      c2.SetGrid()
      c2.Divide(3,4)


      
      v=self.selectepoch("SY1527",first,last)
      botco=1
      topco=2
      botre=4
      topre=5
      chan=[1,2,4,5]
      idx=0
      gs = gridspec.GridSpec(4,3)
      tgr=[]
      for ch in chan:
          x_t=[]
          y_vs=[]
          z_vm=[]
          w_im=[]
          chname=""
          for x in v:
            if (x[1]['channels'][ch]['rampup']==0):
              continue
            x_t.append((x[0]-first)/3600.)
            y_vs.append(x[1]['channels'][ch]['vset'])
            z_vm.append(x[1]['channels'][ch]['vout'])
            w_im.append(x[1]['channels'][ch]['iout'])
              
            #print ch,x
            chname=x[1]['channels'][ch]['name']
          #print chname
          tgr.append(self.buildTGraph('V set vs t (h) %s' % chname,x_t,y_vs,'t(h)','V set (V)'))
          tgr.append(self.buildTGraph('V Mon vs t (h) %s' % chname,x_t,z_vm,'t(h)','V mon (V)'))
          tgr.append(self.buildTGraph('I Mon vs t (h) %s' % chname,x_t,w_im,'t(h)','I mon ([m]A)'))
          self.graphs["SY127-vset-%s" % chname]=self.buildTGraph('V set vs t (h) %s' % chname,x_t,y_vs,'t(h)','V set (V)')
          self.graphs["SY127-vmon-%s" % chname]=self.buildTGraph('V Mon vs t (h) %s' % chname,x_t,z_vm,'t(h)','V mon (V)')
          self.graphs["SY127-imon-%s" % chname]=self.buildTGraph('I Mon vs t (h) %s' % chname,x_t,w_im,'t(h)','I mon ([m]A)')
            


          
          #plt.subplot(gs[idx,0])
          #plt.plot(x_t,y_vs,color='r')
          #plt.ylabel('HV set (V) Channel %d' % ch)
          #plt.xlabel('time (h)')
          #plt.subplot(gs[idx,1])
          #plt.plot(x_t,z_vm,color='r')
          #plt.ylabel('HV Applied (V) Channel %d' % ch)
          #plt.xlabel('time (h)')
          #plt.subplot(gs[idx,2])
          #plt.plot(x_t,w_im,color='r')
          #plt.ylabel('I out (microA) Channel %d' % ch)
          #plt.xlabel('time (h)')
          idx=idx+1
      for idx in range(len(tgr)):
        c2.cd(idx+1);tgr[idx].Draw('APC');c2.Update()
      #plt.show()
      if (self.run==0):
          #plt.savefig('Caen_%s.png' % time.strftime('%Y-%m-%d_%H-%M-%S', time.localtime(first)))
        c2.SaveAs('Caen_%s.png' % time.strftime('%Y-%m-%d_%H-%M-%S', time.localtime(first)))
      else:
          #plt.savefig('Caen_%d.png' % self.run)
        c2.SaveAs('Caen_%d.png' % self.run)
          #plt.savefig('Caen_%s.png' % time.strftime('%Y-%m-%d_%H-%M-%S', time.localtime(first)))
      #val=raw_input()
      

      return [c1,c2]
  def List(self):
    for x,y in self.graphs.iteritems():
        print x
  def Draw(self,name):
    self.canvas.Clear()
    self.canvas.cd()
    self.graphs[name].Draw()
    self.canvas.Update()
