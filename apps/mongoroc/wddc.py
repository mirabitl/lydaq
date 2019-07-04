import os
import socket
import httplib, urllib
import json
try:
   import elementtree.ElementTree as ET
   from elementtree.ElementTree import parse,fromstring, tostring
   import sqlite as SQLITE
except:
   print "SLC6"
   import xml.etree.ElementTree as ET
   from xml.etree.ElementTree import parse,fromstring, tostring
   import sqlite3 as SQLITE
from copy import deepcopy
from operator import itemgetter
import time


class wddClient:
   def __init__ (self,host,port):
      self.host=host
      self.port=port

   def createDaq(self,name):
      lq={}
      lq['name']=name
      lqs=urllib.urlencode(lq)
      myurl = self.host+ ":%d" % (self.port)
      conn = httplib.HTTPConnection(myurl)
      saction = '/createDaq?%s' % (lqs)
      conn.request("GET",saction)
      r1 = conn.getresponse()
      print r1.status, r1.reason
      
   def ChangeState(self,name):
      lq={}
      lq['state']=name
      lqs=urllib.urlencode(lq)
      myurl = self.host+ ":%d" % (self.port)
      conn = httplib.HTTPConnection(myurl)
      saction = '/ChangeState?%s' % (lqs)
      conn.request("GET",saction)
      r1 = conn.getresponse()
      print r1.status, r1.reason
      
   def ChangeRegister(self,r):
      lq={}
      lq['register']=r
      lqs=urllib.urlencode(lq)
      myurl = self.host+ ":%d" % (self.port)
      conn = httplib.HTTPConnection(myurl)
      saction = '/ChangeRegister?%s' % (lqs)
      conn.request("GET",saction)
      r1 = conn.getresponse()
      print r1.status, r1.reason
      

   def stopHost(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/stopHost'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
   def startHost(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/startHost'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
   def stopRPI(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/stopRPI'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
   def startRPI(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/startRPI'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
   def Discover(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/Discover'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
   def DiscoverDNS(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/DiscoverDNS'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
   def DownloadDB(self):
      myurl = self.host+ ":%d" % (self.port)
      conn = httplib.HTTPConnection(myurl)
      saction = '/DownloadDB'
      conn.request("GET",saction)
      r1 = conn.getresponse()
      print r1.status, r1.reason

   def initialiseZup(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/initialiseZup'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
       
   def initialiseWriter(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/initialiseWriter'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
   def scan(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/scan'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
   def Print(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/Print'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
   def Initialise(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/Initialise'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
   def Configure(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/Configure'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
   def Start(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/Start'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
   def Stop(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/Stop'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
   def Destroy(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/Destroy'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
   def Halt(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/Halt'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
   def LVOff(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/LVOff'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
   def LVOn(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/LVOn'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
   def zon(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/zon'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
   def zoff(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/zoff'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
   def HVOn(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/HVOn'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason

   def HVOff(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/HVOff'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason

   def DifStatus(self):
       myurl = self.host+ ":%d" % (self.port)
       conn = httplib.HTTPConnection(myurl)
       saction = '/DifStatus'
       conn.request("GET",saction)
       r1 = conn.getresponse()
       print r1.status, r1.reason
       return r1.read()
       
   def HVSetVoltage(self,v,f,l):
        lq={}
        lq['voltage']=v
        lq['firstchannel']=f
        lq['lastchannel']=l
        lqs=urllib.urlencode(lq)
        myurl = self.host+ ":%d" % (self.port)
        conn = httplib.HTTPConnection(myurl)
        saction = '/HVSetVoltage?%s' % (lqs)
        conn.request("GET",saction)
        r1 = conn.getresponse()
       
   def HVSwitchOn(self,f,l):
        lq={}
        lq['firstchannel']=f
        lq['lastchannel']=l
        lqs=urllib.urlencode(lq)
        myurl = self.host+ ":%d" % (self.port)
        conn = httplib.HTTPConnection(myurl)
        saction = '/HVSwitchOn?%s' % (lqs)
        conn.request("GET",saction)
        r1 = conn.getresponse()
       

   def HVSwitchOff(self,f,l):
        lq={}
        lq['firstchannel']=f
        lq['lastchannel']=l
        lqs=urllib.urlencode(lq)
        myurl = self.host+ ":%d" % (self.port)
        conn = httplib.HTTPConnection(myurl)
        saction = '/HVSwitchOff?%s' % (lqs)
        conn.request("GET",saction)
        r1 = conn.getresponse()
       

   def HVDump(self,f,l):
        lq={}
        lq['firstchannel']=f
        lq['lastchannel']=l
        lqs=urllib.urlencode(lq)
        myurl = self.host+ ":%d" % (self.port)
        conn = httplib.HTTPConnection(myurl)
        saction = '/HVDump?%s' % (lqs)
        conn.request("GET",saction)
        r1 = conn.getresponse()
        return r1.read()

   def HVGet(self,f,l):
        lq={}
        lq['firstchannel']=f
        lq['lastchannel']=l
        lqs=urllib.urlencode(lq)
        myurl = self.host+ ":%d" % (self.port)
        conn = httplib.HTTPConnection(myurl)
        saction = '/HVGet?%s' % (lqs)
        conn.request("GET",saction)
        r1 = conn.getresponse()
        return r1.read()


