#!/usr/bin/env python
import os
import socks
import socket
import httplib, urllib,urllib2
from urllib2 import URLError, HTTPError
import json
from copy import deepcopy
import base64
import time
import argparse
import requests
from ROOT import *

sockport=None
sp=os.getenv("SOCKPORT","Not Found")
if (sp!="Not Found"):
    sockport=int(sp)
if (sockport !=None):
    print "Using SOCKPORT ",sockport
    socks.setdefaultproxy(socks.PROXY_TYPE_SOCKS5, "127.0.0.1", sockport)
    socket.socket = socks.socksocket
    #socks.wrapmodule(urllib2)

def parseReturn(command,sr,res=None,verbose=False):
    if (command=="jobStatus"):
        #s=r1.read()
        #print s["answer"]
        sj=json.loads(sr)
        #sj=s
        #ssj=json.loads(sj["answer"]["ANSWER"])
        ssj=sj["answer"]["ANSWER"]
        print "== %6s %15s %25s %20s ==" % ('PID','NAME','HOST','STATUS')
        for x in ssj:
            if (x['DAQ']=='Y'):
                print "%6d %15s %25s %20s" % (x['PID'],x['NAME'],x['HOST'],x['STATUS'])
    if (command=="hvStatus"):
        sj=json.loads(sr.decode('latin-1').encode("utf-8"))
        device=sj["answer"]["STATUS"]["name"]
        ssj=sj["answer"]["STATUS"]["channels"]
        #print ssj

        if (device == "ISEG"):
            print "== %5s %10s %10s %10s %10s %10s %30s ==" % ('Chan','VSET','ISET','VOUT','IOUT','RAMPUP',"STATUS")
            for x in ssj:
                print "#%.4d %10.2f %10.2f %10.2f %10.2f %10.2f %30s " % (x['id'],x['vset'],x['iset']*1E6,x['vout'],x['iout']*1E6,x['rampup'],x['status'][x['status'].find("=")+1:len(x['status'])-1])
        if (device == "SY1527"):
            print "== %5s %10s %10s %10s %10s %10s %10s ==" % ('Chan','VSET','ISET','VOUT','IOUT','RAMPUP',"STATUS")
            for x in ssj:
                print "#%.4d %10.2f %10.2f %10.2f %10.2f %10.2f %10d " % (x['id'],x['vset'],x['iset'],x['vout'],x['iout'],x['rampup'],x['status'])
    if (command=="LVSTATUS"):
        sj=json.loads(sr)
        
        ssj=sj["answer"]["STATUS"]
        print "== %10s %10s %10s ==" % ('VSET','VOUT','IOUT')
        print " %10.2f %10.2f %10.2f" % (ssj['vset'],ssj['vout'],ssj['iout'])
    if (command=="PTSTATUS"):
        sj=json.loads(sr)
        
        ssj=sj["answer"]["STATUS"]
        print "== %10s %10s  ==" % ('P','T')
        print " %10.2f %10.2f " % (ssj['pressure'],ssj['temperature']+273.15)
    if (command=="HUMSTATUS"):
        sj=json.loads(sr)
        
        ssj=sj["answer"]["STATUS"]
        print "== %10s %10s %10s %10s  ==" % ('H0','T0','H1','T1')
        print " %10.2f %10.2f %10.2f %10.2f " % (ssj['humidity0'],ssj['temperature0'],ssj['humidity11'],ssj['temperature1'])
    if (command=="PTCOR"):
        sj=json.loads(sr)
        
        ssj=sj["answer"]["STATUS"]
        print "== %10s %10s  ==" % ('P','T')
        print " %10.2f %10.2f " % (ssj['pressure'],ssj['temperature']+273.15)
        p=ssj['pressure']
        t=ssj['temperature']+273.15
        if (res.v0==None):
	  print "V0 must be set"
	  return
        if (res.t0==None):
	  print "T0 must be set"
	  return
        if (res.p0==None):
	  print "P0 must be set"
	  return
	print "== Voltage to be set is== %10.2f" % (res.v0*p/res.p0*res.t0/t)
    if (command=="status" and not verbose):

        sj=json.loads(sr)
        ssj=sj["answer"]["diflist"]
        print "== %4s %5s %6s %12s %12s %15s  %s ==" % ('DIF','SLC','EVENT','BCID','BYTES','SERVER','STATUS')

        for d in ssj:
            #print d
            #for d in x["difs"]:
            print '#%4d %5x %6d %12d %12d %15s %s ' % (d["id"],d["slc"],d["gtc"],d["bcid"],d["bytes"],d["host"],d["state"])
    if (command=="tdcstatus" and not verbose):

        sj=json.loads(sr)
        ssj=sj["answer"]["tdclist"]
        print "== %4s %6s %6s %12s %5s %5s ==" % ('DIF','SLC','EVENT','BCID','DETID','TRIGS')

        for d in ssj:
            #print d
            #for d in x["difs"]:
            #print ((d["sourceid"]-10)/256,d["event"],d["gtc"],d["abcid"],d["detid"])
            for x in d:
                print '#%4d %6d %6d %12d %5d %5d ' % ((x["sourceid"]-10)/256,x["gtc"],x["event"],x["abcid"],x["detid"],x["triggers"])
    if (command=="dbStatus" ):
        sj=json.loads(sr)
        ssj=sj["answer"]
        print "== %10s %10s ==" % ('Run','State')
        print " %10d %s " % (ssj['run'],ssj['state'])
    if (command=="shmStatus" ):
        sj=json.loads(sr)
        ssj=sj["answer"]
        print "== %10s %10s ==" % ('Run','Event')
        print " %10d %10d " % (ssj['run'],ssj['event'])
    if (command=="state"):
        sj=json.loads(sr)
        print "== State == :",sj["STATE"]
        scm=""
        for z in sj["CMD"]:
            scm=scm+"%s:" % z["name"]
        scf=""
        for z in sj["FSM"]:
            scf=scf+"%s:" % z["name"]

        print "== Commands == :",scm
        print "== F S M == :",scf
    if (command=="triggerStatus"):
          
        sj=json.loads(sr)
        ssj=sj["answer"]["COUNTERS"]
        print "== %10s %10s %10s %10s %12s %12s %10s %10s %10s ==" % ('Spill','Busy1','Busy2','Busy3','SpillOn','SpillOff','Beam','Mask','EcalMask')
        print " %10d %10d %10d %10d  %12d %12d %12d %10d %10d " % (ssj['spill'],ssj['busy1'],ssj['busy2'],ssj['busy3'],ssj['spillon'],ssj['spilloff'],ssj['beam'],ssj['mask'],ssj['ecalmask'])
    if (command=="difLog" or command=="cccLog" or command=="mdccLog" or command =="zupLog"):
          
        sj=json.loads(sr)
        print  "== %s ==" % sj["answer"]["FILE"]
        ssj=sj["answer"]["LINES"]
        print ssj
        #print "== %10s %10s %10s %10s %12s %12s %10s %10s %10s ==" % ('Spill','Busy1','Busy2','Busy3','SpillOn','SpillOff','Beam','Mask','EcalMask')
        #print " %10d %10d %10d %10d  %12d %12d %12d %10d %10d " % (ssj['spill'],ssj['busy1'],ssj['busy2'],ssj['busy3'],ssj['spillon'],ssj['spilloff'],ssj['beam'],ssj['mask'],ssj['ecalmask'])

        
def executeFSM(host,port,prefix,cmd,params):
   if (params!=None):
       myurl = "http://"+host+ ":%d" % (port)
       #conn = httplib.HTTPConnection(myurl)
       #if (name!=None):
       #    lq['name']=name
       #if (value!=None):
       #    lq['value']=value
       myurl = "http://"+host+ ":%d" % (port)
       #conn = httplib.HTTPConnection(myurl)
       lq={}
       
       lq["content"]=json.dumps(params,sort_keys=True)
       #for x,y in params.iteritems():
       #    lq["content"][x]=y
       lq["command"]=cmd           
       lqs=urllib.urlencode(lq)
       #print lqs
       saction = '/%s/FSM?%s' % (prefix,lqs)
       myurl=myurl+saction
       #print myurl
       req=urllib2.Request(myurl)
       r1=urllib2.urlopen(req)
       return r1.read()

def executeCMD(host,port,prefix,cmd,params):
   if (params!=None and cmd!=None):
       myurl = "http://"+host+ ":%d" % (port)
       #conn = httplib.HTTPConnection(myurl)
       #if (name!=None):
       #    lq['name']=name
       #if (value!=None):
       #    lq['value']=value
       myurl = "http://"+host+ ":%d" % (port)
       #conn = httplib.HTTPConnection(myurl)
       lq={}
       lq["name"]=cmd
       for x,y in params.iteritems():
           lq[x]=y
       lqs=urllib.urlencode(lq)
       saction = '/%s/CMD?%s' % (prefix,lqs)
       myurl=myurl+saction
       #print myurl
       req=urllib2.Request(myurl)
       try:
           r1=urllib2.urlopen(req)
       except URLError, e:
           p_rep={}
           p_rep["STATE"]="DEAD"
           return json.dumps(p_rep,sort_keys=True)
       else:
           return r1.read()
       #proxies={"http":"socks5://127.0.0.1:2080"}
       #requests.get(myurl,timeout=24,proxies=proxies)
   else:
       myurl = "http://"+host+ ":%d/%s/" % (port,prefix)
       #conn = httplib.HTTPConnection(myurl)
       #print myurl
       req=urllib2.Request(myurl)
       try:
           r1=urllib2.urlopen(req)
       except URLError, e:
           p_rep={}
           p_rep["STATE"]="DEAD"
           return json.dumps(p_rep,sort_keys=True)
       else:
           return r1.read()

def executeRequest(surl):
    req=urllib2.Request(surl)
    try:
        r1=urllib2.urlopen(req)
    except URLError, e:
        p_rep={}
        p_rep["STATE"]="DEAD"
        return json.dumps(p_rep,sort_keys=True)

    return r1.read()


#
# Check the configuration
#

class fdaqClient:
  """
  Handle all application definition and parameters
  """
  def __init__(self):
      self.p_conf=None
      self.daq_url=None
      self.daq_file=None
      #self.parseConfig()
      self.daqhost=None
      self.daqport=None
      self.slowhost=None
      self.slowport=None
      self.anhost=None
      self.anport=None
      self.injhost=None
      self.injport=None
      self.tdchost=[]
      self.tdcport=[]
      self.daq_par={}
      self.slow_par={}
      self.scurve_running=False
      #print self.daqhost,self.daqport,self.daq_par
      #print self.slowhost,self.slowport

  def loadConfig(self,name=None,login=None):
      if (name == None):
          self.login=os.getenv("DAQLOGIN","NONE")
          useAuth=self.login!="NONE"
          self.dm=os.getenv("DAQURL","NONE")
      else:
          self.login=login
          self.dm=name
      self.parseConfig()
      for x,y in self.p_conf["HOSTS"].iteritems():
          for p in y:
              port=0
              
              for e in p["ENV"]:
                  if (e.split("=")[0]=="WEBPORT"):
                      port=int(e.split("=")[1])
              if (p["NAME"]=="FDAQ"):
                  self.daqhost=x
                  self.daqport=port
                  if ("PARAMETER" in p):
                      self.daq_par.update(p["PARAMETER"])
                      if ("s_ctrlreg" in self.daq_par):
                          self.daq_par["ctrlreg"]=int(self.daq_par["s_ctrlreg"],16)
              
              if (p["NAME"]=="FSLOW"):
                  self.slowhost=x
                  self.slowport=port
              if (p["NAME"]=="ANALYZER"):
                  self.anhost=x
                  self.anport=port
              if (p["NAME"]=="FEBINJ"):
                  self.injhost=x
                  self.injport=port
              if (p["NAME"]=="TDCSERVER"):
                  print "TDC=============>",x
                  self.tdchost.append(x)
                  self.tdcport.append(port)

              if (p["NAME"]=="DBSERVER"):
                  if ("PARAMETER" in p):
                      self.daq_par["db"]=p["PARAMETER"]
              if (p["NAME"]=="WRITER"):
                  if ("PARAMETER" in p):
                      self.daq_par["builder"]=p["PARAMETER"]
              if (p["NAME"]=="ZUP"):
                  if ("PARAMETER" in p):
                      self.daq_par["zup"]=p["PARAMETER"]
              if (p["NAME"]=="CCCSERVER"):
                  if ("PARAMETER" in p):
                      self.daq_par["ccc"]=p["PARAMETER"]
              if (p["NAME"]=="MDCCSERVER"):
                  if ("PARAMETER" in p):
                      self.daq_par["mdcc"]=p["PARAMETER"]

  def parseConfig(self):
    useAuth=self.login!="NONE"
    dm=self.dm
    if (dm!="NONE"):
        self.daq_url=dm
        read_conf=None
        if (useAuth):
            r = requests.get(dm, auth=(self.login.split(":")[0],self.login.split(":")[1]))
            read_conf=r.json()
        else:
            req=urllib2.Request(dm)
            try:
                r1=urllib2.urlopen(req)
            except URLError, e:
                print e
                p_rep={}
                return
            
            read_conf=json.loads(r1.read())
        #print read_conf
        if ("content" in read_conf):
            self.p_conf=read_conf["content"]
        else:
            self.p_conf=read_conf
    dm=os.getenv("DAQFILE","NONE")
    if (dm!="NONE"):
        self.daq_file=dm
        with open(dm) as data_file:    
            self.p_conf = json.load(data_file)
 
  def jc_create(self):
    lcgi={}
    if (self.daq_url!=None):
        lcgi["url"]=self.daq_url
        if (self.login!="NONE"):
            lcgi["login"]=self.login
    else:
        if (self.daq_file!=None):
            lcgi["file"]=self.daq_file
    rep={}
    for x,y in self.p_conf["HOSTS"].iteritems():
        #print x,"  found"
        sr=executeFSM(x,9999,"LJC-%s" % x,"INITIALISE",lcgi)
    rep[x]=json.loads(sr)
    return json.dumps(rep)
            
  def jc_start(self):
    lcgi={}
    rep={}
    for x,y in self.p_conf["HOSTS"].iteritems():
        #print x,"  found"
        sr=executeFSM(x,9999,"LJC-%s" % x,"START",lcgi)
        rep[x]=json.loads(sr)
    return json.dumps(rep)
  def jc_kill(self):
    lcgi={}
    rep={}
    for x,y in self.p_conf["HOSTS"].iteritems():
        #print x," found"
        sr=executeFSM(x,9999,"LJC-%s" % x,"KILL",lcgi)
        rep[x]= json.loads(sr)
    return json.dumps(rep)        
  def jc_destroy(self):
    lcgi={}
    rep={}
    for x,y in self.p_conf["HOSTS"].iteritems():
        # print x," found"
        sr=executeFSM(x,9999,"LJC-%s" % x,"DESTROY",lcgi)
        rep[x]= json.loads(sr)
    return json.dumps(rep)  

  def jc_appcreate(self):
    lcgi={}
    rep={}
    for x,y in self.p_conf["HOSTS"].iteritems():
        #print x,"  found"
        sr=executeCMD(x,9999,"LJC-%s" % x,"APPCREATE",lcgi)
        rep[x]=json.loads(sr)
    return json.dumps(rep)

  def jc_info(self,hostname,apname=None):
    lcgi={}
    resum={}
    rep=""
    for xh in [hostname]:
        #print "== HOST:  ==",xh
        rep = rep+ "== HOST: %s  == \n" % xh
        resum[xh]=[]
        sr=executeCMD(xh,9999,"LJC-%s" % xh,"STATUS",lcgi)
        sj=json.loads(sr)
        #print "== Acquisition %s ===\n \n == Processus ==\n" % sj['answer']['NAME']
        rep=rep+"== Acquisition %s ===\n \n == Processus ==\n" % sj['answer']['NAME']
        ssj=sj["answer"]["JOBS"]

        if (ssj != None):
            #print "== %6s %15s %25s %20s %20s ==" % ('PID','NAME','HOST','STATUS','PORT')
            rep=rep+"== %6s %15s %25s %20s %20s ==\n" % ('PID','NAME','HOST','STATUS','PORT')
            for x in ssj:
                #print x
                if (apname!=None and x['NAME']!=apname):
                    continue

                if ('PORT' in x):
                    srcmd=executeRequest("http://%s:%s/" % (x['HOST'],x['PORT']))
                    sjcmd=json.loads(srcmd)
                    #print sjcmd
                    resum[xh].append([x,sjcmd])
                else:
                    x['PORT']="0"
                #print "%6d %15s %25s %20s %20s " % (x['PID'],x['NAME'],x['HOST'],x['STATUS'],x['PORT'])
                rep =rep + "%6d %15s %25s %20s %20s\n" % (x['PID'],x['NAME'],x['HOST'],x['STATUS'],x['PORT'])
        else:
            rep=rep+"No Jobs"
    #print "== List of Applications  =="
    rep=rep+ "== List of Applications  ==\n"
    for xh,y in resum.iteritems():
        for x in y:
            # Dead process or non ZDAQ
            if ('PREFIX' not in x[1]):
                #print "================================================= \n == %s   running on %s PID %s is  %s == \n ==========================================\n " % (x[0]['NAME'],x[0]['HOST'],x[0]['PID'],x[0]['STATUS'])
                rep=rep+"================================================= \n == %s   running on %s PID %s is  %s == \n ==========================================\n " % (x[0]['NAME'],x[0]['HOST'],x[0]['PID'],x[0]['STATUS'])
                continue
            # normal process
            #print "== %s :== http://%s:%s/%s/ on %s status %s" % (x[0]['NAME'],x[0]['HOST'],x[0]['PORT'],x[1]['PREFIX'],x[0]['PID'],x[0]['STATUS'])
            rep=rep+"== %s :== http://%s:%s/%s/ on %s status %s\n" % (x[0]['NAME'],x[0]['HOST'],x[0]['PORT'],x[1]['PREFIX'],x[0]['PID'],x[0]['STATUS'])
            #print "\t == STATE :==",x[1]['STATE']
            rep=rep+"\t == STATE : %s == \n" % x[1]['STATE']
            rep=rep+ "\t == FSM :== "
            for z in x[1]['FSM']:
                rep=rep+" "+z['name']
            rep=rep+"\n"
            rep=rep+"\t == ALLOWED :== "
            for z in x[1]['ALLOWED']:
                rep=rep+" "+z['name']
            rep=rep+"\n"
            rep=rep+"\t == COMMAND :=="
            for z in x[1]['CMD']:
                rep=rep+" "+z['name']
            rep=rep+"\n"
            #print "\t == FSM PARAMETERS :=="
            rep=rep+"\t == FSM PARAMETERS :== \n"
            for z in x[1]['CMD']:
                if (z['name'] != "GETPARAM"):
                    continue
                srcmd1=executeRequest("http://%s:%s/%s/CMD?name=GETPARAM" % (x[0]['HOST'],x[0]['PORT'],x[1]['PREFIX']))
                sjcmd1=json.loads(srcmd1)
                #print sjcmd1
                if ('PARAMETER' in sjcmd1['answer']):
                    for xp,vp in sjcmd1['answer']['PARAMETER'].iteritems():
                        #print "\t \t  == %s : == %s " % (xp,vp)
                        rep=rep+ "\t \t  == %s : == %s \n" % (xp,vp)
            
    print rep
    return rep

  def jc_status(self,apname=None):
    lcgi={}
    resum={}
    rep=""
    for xh,y in self.p_conf["HOSTS"].iteritems():
      rep=rep+self.jc_info(xh,apname)
    return rep
  def jc_oldstatus(self,apname=None):
    lcgi={}
    resum={}
    rep=""
    for xh,y in self.p_conf["HOSTS"].iteritems():
        print "== HOST:  ==",xh
        resum[xh]=[]
        sr=executeCMD(xh,9999,"LJC-%s" % xh,"STATUS",lcgi)
        sj=json.loads(sr)
        print "== Acquisition %s \n \n Processus \n==" % sj['answer']['NAME']
        ssj=sj["answer"]["JOBS"]

        if (ssj != None):
            print "== %6s %15s %25s %20s %20s ==" % ('PID','NAME','HOST','STATUS','PORT')
            for x in ssj:
                #print x
                if (apname!=None and x['NAME']!=apname):
                    continue
                if ('PORT' in x):
                    srcmd=executeRequest("http://%s:%s/" % (x['HOST'],x['PORT']))
                    sjcmd=json.loads(srcmd)
                    #print sjcmd
                    resum[xh].append([x,sjcmd])
                else:
                    x['PORT']="0"
                print "%6d %15s %25s %20s %20s " % (x['PID'],x['NAME'],x['HOST'],x['STATUS'],x['PORT'])
                rep =rep + "%6d %15s %25s %20s %20s\n" % (x['PID'],x['NAME'],x['HOST'],x['STATUS'],x['PORT'])
        else:
            rep="No Jobs"
    print "== List of Applications  ==" 
    for xh,y in resum.iteritems():
        for x in y:
            # Dead process or non ZDAQ
            if ('PREFIX' not in x[1]):
                print "================================================= \n == %s   running on %s PID %s is  %s == \n ==========================================\n " % (x[0]['NAME'],x[0]['HOST'],x[0]['PID'],x[0]['STATUS'])
                continue
            # normal process
            print "== %s :== http://%s:%s/%s/ on %s status %s" % (x[0]['NAME'],x[0]['HOST'],x[0]['PORT'],x[1]['PREFIX'],x[0]['PID'],x[0]['STATUS'])
            print "\t == STATE :==",x[1]['STATE']
            rep="\t == FSM :=="
            for z in x[1]['FSM']:
                rep=rep+" "+z['name']
            print rep
            rep="\t == ALLOWED :=="
            for z in x[1]['ALLOWED']:
                rep=rep+" "+z['name']
            print rep
            rep="\t == COMMAND :=="
            for z in x[1]['CMD']:
                rep=rep+" "+z['name']
            print rep
            print "\t == FSM PARAMETERS :=="
            for z in x[1]['CMD']:
                if (z['name'] != "GETPARAM"):
                    continue
                srcmd1=executeRequest("http://%s:%s/%s/CMD?name=GETPARAM" % (x[0]['HOST'],x[0]['PORT'],x[1]['PREFIX']))
                sjcmd1=json.loads(srcmd1)
                #print sjcmd1
                if ('PARAMETER' in sjcmd1['answer']):
                    for xp,vp in sjcmd1['answer']['PARAMETER'].iteritems():
                        print "\t \t  == %s : == %s " % (xp,vp)
            

    return rep

  def jc_restart(self,host,jobname,jobpid):
    lcgi={}
    lcgi["processname"]=jobname
    lcgi["pid"]=jobpid
    sr=executeCMD(host,9999,"LJC-%s" % host,"RESTARTJOB",lcgi)
    print sr
    
  def daq_create(self):
      lcgi={}
      rep={}
      if (self.daq_url!=None):
          lcgi["url"]=self.daq_url
          if (self.login!="NONE"):
            lcgi["login"]=self.login
      else:
          if (self.daq_file!=None):
              lcgi["file"]=self.daq_file
      for x,y in self.p_conf["HOSTS"].iteritems():
          for p in y:
              if (p["NAME"] != "FDAQ"):
                  continue;
              print x,p["NAME"]," process found"
              port=0
              for e in p["ENV"]:
                  if (e.split("=")[0]=="WEBPORT"):
                      port=int(e.split("=")[1])
              if (port==0):
                  continue
              p_rep={}
              surl="http://%s:%d/" % (x,port)
              req=urllib2.Request(surl)
              try:
                  r1=urllib2.urlopen(req)
                  p_rep=json.loads(r1.read())
              except URLError, e:
                  print surl,e
                  p_rep={}
              print x,port,p["NAME"],p_rep
              if ("STATE" in p_rep):
                  if (p_rep["STATE"]=="VOID"):
                      sr=executeFSM(x,port,p_rep["PREFIX"],"CREATE",lcgi)
                      #print sr
  def daq_list(self):
      lcgi={}
      rep=""
      if (self.daq_url!=None):
          lcgi["url"]=self.daq_url
      else:
          if (self.daq_file!=None):
              lcgi["file"]=self.daq_file
      for x,y in self.p_conf["HOSTS"].iteritems():
          print "HOST ",x
          rep=rep+" Host %s \n" % x
          print "== %12s %12s %8s %8s %20s ==" % ('NAME','INSTANCE','PORT','PID','STATE')
          rep=rep+"== %12s %12s %8s %8s %20s ==\n" % ('NAME','INSTANCE','PORT','PID','STATE')
          for p in y:
              #print x,p["NAME"]," process found"
              port=0
              for e in p["ENV"]:
                  if (e.split("=")[0]=="WEBPORT"):
                      port=int(e.split("=")[1])
              if (port==0):
                  continue
              p_rep={}
              surl="http://%s:%d/" % (x,port)
              req=urllib2.Request(surl)
              try:
                  r1=urllib2.urlopen(req)
                  p_rep=json.loads(r1.read())
                  print "%12s %12s %8d %8d %20s" % (p["NAME"],p_rep["PREFIX"],port,p_rep["PID"],p_rep["STATE"])
                  rep =rep +"%12s %12s %8d %8d %20s\n" % (p["NAME"],p_rep["PREFIX"],port,p_rep["PID"],p_rep["STATE"])
              except URLError, e:
                  print surl,e
                  p_rep={}
      return rep
              
  def daq_info(self,name):
      lcgi={}
      rep=""
      if (self.daq_url!=None):
          lcgi["url"]=self.daq_url
      else:
          if (self.daq_file!=None):
              lcgi["file"]=self.daq_file
      for x,y in self.p_conf["HOSTS"].iteritems():
          print "HOST ",x
          rep=rep+" Host %s \n" % x
          print "== %12s %12s %8s %8s %20s ==" % ('NAME','INSTANCE','PORT','PID','STATE')
          rep=rep+"== %12s %12s %8s %8s %20s ==\n" % ('NAME','INSTANCE','PORT','PID','STATE')
          for p in y:
              #print x,p["NAME"]," process found"
              port=0
              for e in p["ENV"]:
                  if (e.split("=")[0]=="WEBPORT"):
                      port=int(e.split("=")[1])
              if (port==0):
                  continue
              p_rep={}
              surl="http://%s:%d/" % (x,port)
              req=urllib2.Request(surl)
              try:
                  r1=urllib2.urlopen(req)
                  p_rep=json.loads(r1.read())
                  if (p["NAME"]!=name):
                      continue
                  print p
                  print "%12s %12s %8d %8d %20s" % (p["NAME"],p_rep["PREFIX"],port,p_rep["PID"],p_rep["STATE"])
                  rep =rep +"%12s %12s %8d %8d %20s\n" % (p["NAME"],p_rep["PREFIX"],port,p_rep["PID"],p_rep["STATE"])
              except URLError, e:
                  print surl,e
                  p_rep={}
      return rep
              
  def daq_discover(self):
      lcgi={}
      sr=executeFSM(self.daqhost,self.daqport,"FDAQ","DISCOVER",lcgi)
      return sr
      
     
  def daq_setparameters(self):
      lcgi={}
      lcgi["PARAMETER"]=json.dumps(self.daq_par,sort_keys=True)
      
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","SETPARAM",lcgi)
      print sr
      
  def daq_getparameters(self):
      lcgi={}
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","GETPARAM",lcgi)
      print sr
  def daq_downloaddb(self,state):
      lcgi={}
      print "Downloading ",state
      if ("db" in self.daq_par):
        self.daq_par["db"]["dbstate"]=state
      lcgi["state"]=state
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","DOWNLOADDB",lcgi)
      rep=json.loads(sr)
      return json.dumps(rep)
  def daq_forceState(self,name):
      lcgi={}
      lcgi["state"]=name
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","FORCESTATE",lcgi)
      print sr
      
  def daq_services(self):

      lcgi={}
      sr=executeFSM(self.daqhost,self.daqport,"FDAQ","PREPARE",lcgi)
      rep=json.loads(sr)
      return json.dumps(rep)

  def daq_initialise(self):
      lcgi={}
      sr=executeFSM(self.daqhost,self.daqport,"FDAQ","INITIALISE",lcgi)
      rep=json.loads(sr)
      #print rep
      #print "COUCOU"
      return json.dumps(rep)

  def daq_configure(self):
      lcgi=self.daq_par
      sr=executeFSM(self.daqhost,self.daqport,"FDAQ","CONFIGURE",lcgi)
      rep=json.loads(sr)
      return json.dumps(rep)

  def daq_start(self):
      self.trig_reset()
      #self.trig_spillon(1000000)
      #self.trig_spilloff(100000)
      #####   self.trig_spillregister(0)
      self.trig_calibon(0)
      self.trig_status()

      lcgi={}
      
      srs=executeFSM(self.daqhost,self.daqport,"FDAQ","START",lcgi)
      rep=json.loads(srs)
      lcgi["value"]=1
      srm=executeCMD(self.daqhost,self.daqport,"FDAQ","MONITOR",lcgi)
      #print srm
      self.daq_setrunheader(0,0)
      return json.dumps(rep)

  def daq_normalstop(self):
      lcgi={}
      sr=executeFSM(self.daqhost,self.daqport,"FDAQ","STOP",lcgi)
      rep=json.loads(sr)

      lcgi["value"]=0
      srm=executeCMD(self.daqhost,self.daqport,"FDAQ","MONITOR",lcgi)
      return json.dumps(rep)
  def daq_stop(self):
      if (self.scurve_running):
          self.scurve_running=False;
          rep={}
          rep["SCURVE"]="STOPPED"
          
      else:    
         return self.daq_normalstop()
      #print srm
      return json.dumps(rep)

  def daq_destroy(self):
      lcgi={}
      sr=executeFSM(self.daqhost,self.daqport,"FDAQ","DESTROY",lcgi)
      rep=json.loads(sr)
      return json.dumps(sr)

  def daq_status(self):
      lcgi={}
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","DIFSTATUS",lcgi)
      return sr
  def daq_tdcstatus(self):
      lcgi={}
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","TDCSTATUS",lcgi)
      #print sr
      return sr
  def daq_resettdc(self):
      lcgi={}
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","RESETTDC",lcgi)
      #print sr
      rep=json.loads(sr)
      return json.dumps(sr)

  def daq_evbstatus(self):
      lcgi={}
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","EVBSTATUS",lcgi)
      return sr    
      
  def daq_dbstatus(self):
      lcgi={}
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","DBSTATUS",lcgi)
      return sr    
      
  def daq_state(self):
      p_rep={}
      state="UNKNOWN"
      req=urllib2.Request("http://%s:%d/" % (self.daqhost,self.daqport))
      try:
         r1=urllib2.urlopen(req)
         p_rep=json.loads(r1.read())
      except URLError, e:
         print "no connection to DAQ"
         return "NO CONNECTION"
         exit(0)
      if ("STATE" in p_rep):
         #print p_rep["STATE"]
         return p_rep["STATE"]
         
  def daq_ctrlreg(self,ctrl):
      lcgi={}
      lcgi["value"]=ctrl
      self.daq_par["ctrlreg"]=ctrl
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","CTRLREG",lcgi)
      rep=json.loads(sr)
      return json.dumps(sr)

      
  def daq_setgain(self,gain):
      lcgi={}
      lcgi["GAIN"]=gain
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","SETGAIN",lcgi)
      rep=json.loads(sr)
      return json.dumps(sr)

      
  def daq_setthreshold(self,b0,b1,b2):
      lcgi={}
      lcgi["B0"]=b0
      lcgi["B1"]=b1
      lcgi["B2"]=b2
      
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","SETTHRESHOLD",lcgi)
      rep=json.loads(sr)
      return json.dumps(sr)
      
  def daq_settdcmode(self,mode):
      lcgi={}
      lcgi["value"]=mode
      
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","SETTDCMODE",lcgi)
      print sr
      
  def daq_settdcdelays(self,active,dead):
      lcgi={}
      lcgi["active"]=active
      lcgi["dead"]=dead      
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","SETTDCDELAYS",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)
      

      
  def trig_status(self):
      lcgi={}
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","TRIGGERSTATUS",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)
      
      

  def trig_reset(self):
      lcgi={}
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","RESETCOUNTERS",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)


  def trig_pause(self):
      lcgi={}
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","PAUSE",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)

  def trig_resume(self):
      lcgi={}
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","RESUME",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)

  def ecal_pause(self):
      lcgi={}
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","ECALPAUSE",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)


  def ecal_resume(self):
      lcgi={}
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","ECALRESUME",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)



  def trig_spillon(self,clock):
      lcgi={}
      lcgi["clock"]=clock
      
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","SPILLON",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)


  def trig_spilloff(self,clock):
      lcgi={}
      lcgi["clock"]=clock
      
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","SPILLOFF",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)

      
  def trig_beam(self,clock):
      lcgi={}
      lcgi["clock"]=clock
      
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","BEAMON",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)


  def trig_spillregister(self,value):
      lcgi={}
      lcgi["value"]=value
      
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","SPILLREGISTER",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)

  def trig_setregister(self,address,value):
      lcgi={}
      lcgi["address"]=address
      lcgi["value"]=value
      
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","TRIGGERSETREG",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)

      print sr    
  def trig_getregister(self,address):
      lcgi={}
      lcgi["address"]=address
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","TRIGGERGETREG",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)

      print sr    
  def trig_tdcreset(self):
      lcgi={}
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","RESETTDC",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)



  def trig_hardreset(self):
      lcgi={}
      lcgi["value"]=0
      
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","SETHARDRESET",lcgi)
      print sr
      time.sleep(2)
      lcgi={}
      lcgi["value"]=1
      
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","SETHARDRESET",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)


  def trig_calibcount(self,value):
      lcgi={}
      lcgi["clock"]=value
      
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","CALIBCOUNT",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)



  def trig_calibon(self,value):
      lcgi={}
      lcgi["value"]=value
      
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","CALIBON",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)



  def daq_process(self):
      lcgi={}
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","LISTPROCESS",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)



  def trig_reloadcalib(self):
      lcgi={}
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","RELOADCALIB",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)



  def tdc_set6bdac(self,value):
      lcgi={}
      lcgi["value"]=value
      
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","SET6BDAC",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)


  def tdc_setvthtime(self,value):
      lcgi={}
      lcgi["value"]=value
      
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","SETVTHTIME",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)


  def tdc_setmask(self,value,asic):
      lcgi={}
      lcgi["value"]=value
      lcgi["asic"]=asic
      
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","SETMASK",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)


  def daq_setrunheader(self,rtyp,value,mask=0XFFFFFFFF):
      lcgi={}
      lcgi["value"]=value
      lcgi["type"]=rtyp
      lcgi["mask"]=mask
      sr=executeCMD(self.daqhost,self.daqport,"FDAQ","SETRUNHEADER",lcgi)
      rep =json.loads(sr)
      return json.dumps(rep)



  def daq_calibdac(self,ntrg,ncon,dacmin,dacmax,mask):
      self.trig_pause()
      self.trig_spillon(30)
      self.trig_spilloff(1000000)
      self.trig_spillregister(4)
      self.trig_calibon(1)
      self.trig_calibcount(ntrg)
      self.trig_status()
      #self.tdc_setmask(63)

      for idac in range(dacmin,dacmax+1):
          self.tdc_set6bdac(idac)
          #self.tdc_setmask(mask)
          self.daq_setrunheader(1,idac)
          # check current evb status
          sr=self.daq_evbstatus()
          sj=json.loads(sr)
          ssj=sj["answer"]
          firstEvent=int(ssj["event"])
          time.sleep(1)
          self.trig_reloadcalib()
          self.trig_resume()
          self.trig_status()
          lastEvent=firstEvent
          nloop=0;
          while (lastEvent<(firstEvent+ntrg-10)):
              sr=self.daq_evbstatus()
              sj=json.loads(sr)
              ssj=sj["answer"]
              lastEvent=int(ssj["event"])
              print firstEvent,lastEvent,idac
              time.sleep(1)
              nloop=nloop+1
              if (nloop>20):
                  break
      self.trig_calibon(0)
      self.trig_pause()
      return
  def daq_scurve(self,ntrg,ncon,ncoff,thmin,thmax,mask,step=5):
      self.trig_pause()
      self.trig_spillon(ncon)
      print "ncon=",ncon
      print "ncoff=",ncoff
      self.trig_spilloff(ncoff)
      self.trig_spillregister(4)
      self.trig_calibon(1)
      self.trig_calibcount(ntrg)
      self.trig_status()
      #self.tdc_setmask(mask)
      thrange=(thmax-thmin+1)/step
      for vth in range(0,thrange+1):
          if ( not self.scurve_running):
              break;

          #self.tdc_setvthtime(thmax-vth*step)
          xi=thmin+vth*step
          xa=thmax-vth*step
          xi=xa
          self.trig_pause()
          self.tdc_setvthtime(xi)
          #name = input("What's your name? ")
          self.curvth=xi
          time.sleep(0.1)
         
          #self.tdc_setmask(mask)
          #self.daq_setrunheader(2,(thmax-vth*step))
          self.daq_setrunheader(2,xi)
          # check current evb status
          sr=self.daq_evbstatus()
          sj=json.loads(sr)
          ssj=sj["answer"]
          firstEvent=int(ssj["event"])
          time.sleep(0.1)
          
          self.trig_reloadcalib()
          self.trig_resume()
          self.trig_status()
          lastEvent=firstEvent
          nloop=0;
          while (lastEvent<(firstEvent+ntrg-20)):
              sr=self.daq_evbstatus()
              sj=json.loads(sr)
              ssj=sj["answer"]
              lastEvent=int(ssj["event"])
              print firstEvent,lastEvent,xi
              time.sleep(1)
              nloop=nloop+1
              if (nloop>8):
                  break
      self.trig_calibon(0)
      self.trig_pause()
      return
  def daq_fullscurve(self,ch,spillon,spilloff,beg,las,step=2,asic=255,mode="FEB1"):
      ### petiroc to scan OLD
      firmware1=[31,0,30,1,29,2,28,3,27,4,26,5,25,6,24,7,23,8,22,9,21,10,20,11]
      # Coaxial chamber COAX
      firmwarec=[0,1,2,3,4,5,6,7,8,9,10,19,20,21,22,23,24,25,26,27,28,29,30,31]
      # Return chamber FEB0
      firmwaret=[31,29,27,25,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6]
      # return chamber FEB1
      # Buggy firmwaret1=[21,20,23,22,25,24,27,26,29,28,31,30,1,0,3,2,5,4,7,6,10,8,15,14,12]
      firmwaret1=[3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,26,28,30]
      self.scurve_running=True
      if (mode=="OLD"):
          firmware=firmware1
      if (mode=="COAX"):
          firmware=firmwarec
      if (mode=="FEB0"):
          firmware=firmwaret
      if (mode=="FEB1"):
          firmware=firmwaret1
      nevmax=200

      ###
      self.daq_start()
      #### commenter en dessous
      if (ch==255):
          self.tdc_setmask(0XFFFFFFFF,asic)
          #self.tdc_setmask(0Xf7fffffb)
          #self.tdc_setmask(1073741832)
          mask=0
          for i in firmware:
              mask=mask|(1<<i)
          self.daq_scurve(nevmax,spillon,spilloff,beg,las,mask,step)
          self.daq_normalstop()
          return
      if (ch==1023):
          #for ist in range(0,12):
          #    self.tdc_setmask((1<<ist))
          #    self.daq_scurve(100,200,beg,las,(1<<ist),step)
          #    self.tdc_setmask((1<<(31-ist)))
          #    self.daq_scurve(100,200,beg,las,(1<<(31-ist)),step)
          for ist in firmware:
              if ( not self.scurve_running):
                  break;
              self.tdc_setmask((1<<ist),asic)
              self.daq_scurve(nevmax,spillon,spilloff,beg,las,(1<<ist),step)
          self.daq_normalstop()
          return
      ipr=0
      if (ch%2==1):
          ipr=ch/2
      else:
          ipr=(31-ch/2)
      ipr=ch
      self.tdc_setmask((1<<ipr),asic)
      self.daq_scurve(nevmax,spillon,spilloff,beg,las,(1<<ipr),step)
      self.daq_normalstop()
      return
      # channel 1
      #self.tdc_setmask((1<<0))
      #self.daq_scurve(100,200,280,420,4294967295,4)
      # channel 3
      #self.tdc_setmask((1<<1))
      #self.daq_scurve(100,200,270,430,4294967295,4)
      # channel 5
      #self.tdc_setmask((1<<2))
      #self.daq_scurve(100,200,280,420,4294967295,4)
      # channel 7
      #self.tdc_setmask((1<<3))
      #self.daq_scurve(100,200,320,430,4294967295,2)
      # channel 9
      #self.tdc_setmask((1<<4))
      #self.daq_scurve(100,200,280,420,4294967295,4)
      # channel 13
      #self.tdc_setmask((1<<6))
      #self.daq_scurve(100,200,280,420,4294967295,4)
      

      #self.daq_stop()
      

  def slow_create(self):
      lcgi={}
      if (self.daq_url!=None):
          lcgi["url"]=self.daq_url
          if (self.login!="NONE"):
            lcgi["login"]=self.login
      else:
          if (self.daq_file!=None):
              lcgi["file"]=self.daq_file
      for x,y in self.p_conf["HOSTS"].iteritems():
          for p in y:
              if (p["NAME"] != "FSLOW"):
                  continue;
              print x,p["NAME"]," process found"
              port=0
              for e in p["ENV"]:
                  if (e.split("=")[0]=="WEBPORT"):
                      port=int(e.split("=")[1])
              if (port==0):
                  continue
              p_rep={}
              surl="http://%s:%d/" % (x,port)
              req=urllib2.Request(surl)
              try:
                  r1=urllib2.urlopen(req)
                  p_rep=json.loads(r1.read())
              except URLError, e:
                  print surl,e
                  p_rep={}
              print x,port,p["NAME"],p_rep
              if ("STATE" in p_rep):
                  if (p_rep["STATE"]=="VOID"):
                      sr=executeFSM(x,port,p_rep["PREFIX"],"CREATE",lcgi)
                      #print sr


  def slow_discover(self):
      lcgi={}
      sr=executeFSM(self.slowhost,self.slowport,"FSLOW","DISCOVER",lcgi)
      print sr
      
  def slow_configure(self):
      lcgi={}
      sr=executeFSM(self.slowhost,self.slowport,"FSLOW","CONFIGURE",lcgi)
      print sr
  def slow_start(self):
      lcgi={}
      sr=executeFSM(self.slowhost,self.slowport,"FSLOW","START",lcgi)
      print sr
          
  def slow_stop(self):
      lcgi={}
      sr=executeFSM(self.slowhost,self.slowport,"FSLOW","STOP",lcgi)
      print sr
          
  def slow_lvon(self):
      lcgi={}
      sr=executeCMD(self.slowhost,self.slowport,"FSLOW","LVON",lcgi)
      return sr
      
  def slow_lvoff(self):
      lcgi={}
      sr=executeCMD(self.slowhost,self.slowport,"FSLOW","LVOFF",lcgi)
      return sr
      
  def slow_lvstatus(self):
      lcgi={}
      sr=executeCMD(self.slowhost,self.slowport,"FSLOW","LVSTATUS",lcgi)
      return sr
      
  def slow_ptstatus(self):
      lcgi={}
      sr=executeCMD(self.slowhost,self.slowport,"FSLOW","PTSTATUS",lcgi)
      return sr
  
  def slow_humstatus(self):
      lcgi={}
      sr=executeCMD(self.slowhost,self.slowport,"FSLOW","HUMSTATUS",lcgi)
      return sr


  def slow_hvstatus(self,first,last):
      lcgi={}
      lcgi["first"]=first
      lcgi["last"]=last
      sr=executeCMD(self.slowhost,self.slowport,"FSLOW","HVSTATUS",lcgi)
      return sr

  def slow_hvon(self,first,last):
      lcgi={}
      lcgi["first"]=first
      lcgi["last"]=last
      sr=executeCMD(self.slowhost,self.slowport,"FSLOW","HVON",lcgi)
      return sr

  def slow_hvoff(self,first,last):
      lcgi={}
      lcgi["first"]=first
      lcgi["last"]=last
      sr=executeCMD(self.slowhost,self.slowport,"FSLOW","HVOFF",lcgi)
      return sr
      
  def slow_clearalarm(self,first,last):
      lcgi={}
      lcgi["first"]=first
      lcgi["last"]=last
      sr=executeCMD(self.slowhost,self.slowport,"FSLOW","CLEARALARM",lcgi)
      return sr

  def slow_vset(self,first,last,value):
      lcgi={}
      lcgi["first"]=first
      lcgi["last"]=last
      lcgi["value"]=value
      sr=executeCMD(self.slowhost,self.slowport,"FSLOW","VSET",lcgi)
      return sr

  def slow_iset(self,first,last,value):
      lcgi={}
      lcgi["first"]=first
      lcgi["last"]=last
      lcgi["value"]=value
      sr=executeCMD(self.slowhost,self.slowport,"FSLOW","ISET",lcgi)
      return sr

  def slow_rampup(self,first,last,value):
      lcgi={}
      lcgi["first"]=first
      lcgi["last"]=last
      lcgi["value"]=value
      sr=executeCMD(self.slowhost,self.slowport,"FSLOW","RAMPUP",lcgi)
      return sr
  def analysis_process(self,run):
      if (self.anhost==None):
          return "NO Analysis"
      lcgi={}
      lcgi["run"]=run
      sr=executeFSM(self.anhost,self.anport,"RB","PROCESS",lcgi)
      return sr
  def analysis_monitor(self,run):
      if (self.anhost==None):
          return "NO Analysis"
      lcgi={}
      lcgi["run"]=run
      sr=executeFSM(self.anhost,self.anport,"RB","MONITOR",lcgi)
      return sr
  def analysis_stop(self,run):
      if (self.anhost==None):
          return "NO Analysis"
      lcgi={}
      lcgi["run"]=run
      sr=executeFSM(self.anhost,self.anport,"RB","STOP",lcgi)
      return sr
  def analysis_status(self):
      if (self.anhost==None):
          return "NO Analysis"
      
      lcgi={}
      sr=executeCMD(self.anhost,self.anport,"RB","STATUS",lcgi)
      return sr
  def analysis_histolist(self):
      if (self.anhost==None):
          return "NO Analysis"
      lcgi={}
      sr=executeCMD(self.anhost,self.anport,"RB","HISTOLIST",lcgi)
      rep=json.loads(sr)
      return json.dumps(rep)
  
  def analysis_histo(self,h):
      if (self.anhost==None):
          return "NO Analysis"
      lcgi={}
      lcgi["histo"]=h
      sr=executeCMD(self.anhost,self.anport,"RB","HISTO",lcgi)
      rep=json.loads(sr)
      return json.dumps(rep)

  def injection_configure(self):
      if (self.injhost==None):
          return "No Injection board"
      lcgi={}
      sr=executeFSM(self.injhost,self.injport,"FebInj-0","CONFIGURE",lcgi)
      return sr
  def injection_destroy(self):
      if (self.injhost==None):
          return "No Injection board"
      lcgi={}
      sr=executeFSM(self.injhost,self.injport,"FebInj-0","DESTROY",lcgi)
      return sr
  def injection_mask(self,side,mask):
      if (self.injhost==None):
          return "No Injection board"
      lcgi={}
      lcgi["mask"]=mask
      lcgi["side"]=side
      print "Injection ",side," Mask ",mask
      sr=executeCMD(self.injhost,self.injport,"FebInj-0","MASK",lcgi)
      return sr
  def injection_source(self,source):
      if (self.injhost==None):
          return "No Injection board"
      lcgi={}
      lcgi["value"]=source
      sr=executeCMD(self.injhost,self.injport,"FebInj-0","TRIGGERSOURCE",lcgi)
      return sr
  def injection_number(self,nb):
      if (self.injhost==None):
          return "No Injection board"
      lcgi={}
      lcgi["value"]=nb
      sr=executeCMD(self.injhost,self.injport,"FebInj-0","TRIGGERMAX",lcgi)
      return sr
  def injection_delay(self,nb):
      if (self.injhost==None):
          return "No Injection board"
      lcgi={}
      lcgi["value"]=nb
      sr=executeCMD(self.injhost,self.injport,"FebInj-0","DELAY",lcgi)
      return sr
  def injection_duration(self,nb):
      if (self.injhost==None):
          return "No Injection board"
      lcgi={}
      lcgi["value"]=nb
      sr=executeCMD(self.injhost,self.injport,"FebInj-0","DURATION",lcgi)
      return sr
  def injection_height(self,nb):
      if (self.injhost==None):
          return "No Injection board"
      lcgi={}
      lcgi["value"]=nb
      sr=executeCMD(self.injhost,self.injport,"FebInj-0","PULSEHEIGHT",lcgi)
      return sr
  def injection_soft(self):
      if (self.injhost==None):
          return "No Injection board"
      lcgi={}
      sr=executeCMD(self.injhost,self.injport,"FebInj-0","TRIGGERSOFT",lcgi)
      return sr
  def injection_internal(self):
      if (self.injhost==None):
          return "No Injection board"
      lcgi={}
      sr=executeCMD(self.injhost,self.injport,"FebInj-0","TRIGGERINT",lcgi)
      return sr
  def injection_pause(self):
      if (self.injhost==None):
          return "No Injection board"
      lcgi={}
      sr=executeCMD(self.injhost,self.injport,"FebInj-0","PAUSE",lcgi)
      return sr
  def injection_resume(self):
      if (self.injhost==None):
          return "No Injection board"
      lcgi={}
      sr=executeCMD(self.injhost,self.injport,"FebInj-0","RESUME",lcgi)
      return sr
  def lut_calib(self,tdc,channel):
      if (len(self.tdchost)<tdc+1):
          return "non existing tdc"
      lcgi={}
      lcgi["value"]="%x" % ((1<<channel))
      sr=executeCMD(self.tdchost[tdc],self.tdcport[tdc],"TDC-%d" % tdc,"CALIBMASK",lcgi)
      print sr
      sr=executeCMD(self.tdchost[tdc],self.tdcport[tdc],"TDC-%d" % tdc,"CALIBSTATUS",lcgi)
      print sr
  def tdc_testmask(self,tdc,mask):
      if (len(self.tdchost)<tdc+1):
          return "non existing tdc"
      lcgi={}
      lcgi["value"]="%x" % mask
      sr=executeCMD(self.tdchost[tdc],self.tdcport[tdc],"TDC-%d" % tdc,"TESTMASK",lcgi)
      print sr
      sr=executeCMD(self.tdchost[tdc],self.tdcport[tdc],"TDC-%d" % tdc,"CALIBSTATUS",lcgi)
      print sr
  def lut_draw(self,tdc,channel,canvas=None):
      if (len(self.tdchost)<tdc+1):
          return "non existing tdc"
      lcgi={}
      sr=executeCMD(self.tdchost[tdc],self.tdcport[tdc],"TDC-%d" % tdc,"CALIBSTATUS",lcgi)
      print sr
      lcgi["value"]=channel
      sr=executeCMD(self.tdchost[tdc],self.tdcport[tdc],"TDC-%d" % tdc,"GETLUT",lcgi)
      print sr
      
      s=json.loads(sr)
      ar=s["answer"]["LUT"]["192.168.10.15"]["content"]
      lut=[]
      xi=[]
      idx=0
      a=0
      xmi=9999
      xma=0
      print "N",ar[5]
      N=int(ar[5])
      if (N==0):
          N=149
      idx=0
      tdlen=0
      tdorig=9999
      for i in range(6,2*N+6):
          x=int(ar[i])
          ##print x
          if (idx%2 ==0):
              a=(x<<8)
          else:
              a=a+x
              if (a>32768):
                  break
              if (a==0 and idx/2>140):
                  break

              if (a==32768 and tdlen==0):
                  tdlen=idx/2
                  #lut.append(a/128.*2.5/256.)
              #print idx/2,a,tdorig,tdlen
              if (a==0 and tdorig>0):
                  tdorig=idx/2+1
              lut.append(a)
              xi.append(idx/2)
              if (idx/2 < xmi):
                  xmi=idx/2
              if (idx/2 > xma):
                   xma=idx/2
              a=0
          idx=idx+1
      print idx
      print xi
      print lut
      print "Delay line length",tdorig,tdlen
      standalone=(canvas==None)
      if (standalone):
          canvas=TCanvas()
      gStyle.SetOptStat(0)
      gStyle.SetOptFit(1)
      xmi=xmi
      xma=xma

      h=TH1F("lut%d" % int(channel),"LUT %d " % int(channel),len(xi),xmi,xma)
      for i in range(0,len(xi)):
          h.SetBinContent(i+1,lut[i])
      canvas.cd()
      h.Draw()
      h.Fit("pol1","","",tdorig+1,tdlen-5)
      canvas.Update()
      if (standalone):
          v=raw_input()
      return (h,tdorig,tdlen)

  def  histo_draw(self,name):
        # If the changed item is not checked, don't bother checking others
        print name
        r=self.analysis_histo(name)
        #print r
        sr=json.loads(r)
        #print sr["answer"]["histo"].replace('\\n','').replace('\"','"')
        h=TBufferJSON.ConvertFromJSON(sr["answer"]["histo"].replace('\\n','').replace('\"','"'))
        #tree = ET.XML(sr["answer"]["histo"])
        #h=TBufferXML.ConvertFromXML(ET.tostring(tree))
        #print ET.tostring(tree)
        c=TCanvas()
        
        h.Draw()
        h.Print()
        vb=[]
        s=""
        for i in range(1,h.GetNbinsX()):
            s=s+"%f " % h.GetBinContent(i)
            vb.append(h.GetBinContent(i))
        print s
        #self.canvas.Draw()
        c.Modified()
        c.Update()
        v=raw_input()
        return vb
  def  get_channel_summary(self,run,feb,delay,nchan):
      namem="/run%d/Raw/Feb%d/ChannelMean" % (run,feb)
      namer="/run%d/Raw/Feb%d/ChannelRMS" % (run,feb)
      vm=self.histo_draw(namem)
      vr=self.histo_draw(namer)

      foutm=open("mean%d.txt" % (run),"a")
      foutr=open("rms%d.txt" % (run),"a")
      sm="%7.2f " % delay
      sr="%7.2f " % delay
      for i in range(0,nchan-1):
          sm=sm+"%7.3f " % vm[i]
          sr=sr+"%7.2f " % (vr[i]*1000)

      sr=sr+"\n"
      sm=sm+"\n"
      foutm.write(sm)
      foutr.write(sr)
      foutm.close()
      foutr.close()
