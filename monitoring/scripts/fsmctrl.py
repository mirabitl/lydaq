from __future__ import absolute_import
from __future__ import print_function
import os
import socks
import socket
import six.moves.http_client
import six.moves.urllib.request, six.moves.urllib.parse, six.moves.urllib.error
import six.moves.urllib.request, six.moves.urllib.error, six.moves.urllib.parse
from six.moves.urllib.error import URLError, HTTPError
import json
from copy import deepcopy
import base64
import time
import requests
import six
try:
    from urllib.parse import urlparse
except ImportError:
     from six.moves.urllib.parse import urlparse


# Sock support
sockport = None
sp = os.getenv("SOCKPORT", "Not Found")
if (sp != "Not Found"):
    sockport = int(sp)
if (sockport != None):
    # print "Using SOCKPORT ",sockport
    socks.setdefaultproxy(socks.PROXY_TYPE_SOCKS5, "127.0.0.1", sockport)
    socket.socket = socks.socksocket
    # socks.wrapmodule(urllib2)


class FSMCtrl:
    def __init__(self, vhost, vport):
        """
        Handle all application definition and p  arameters , It controls the acquisition via the FDAQ application and the Slow control via the FSLOW application
        """
        self.host = vhost
        self.port = vport
        self.url = "http://%s:%d" % (vhost, vport)
        self.getInfo()
        self.appType = 'UNKNOWN'
        self.appInstance = 0
        self.infos = {}
        self.procInfos = {}

    def executeRequest(self, url):
        """
         Acces to an url

        :param surl: The url
        :return: url answer
        """
        #print "Access to %s " % url
        req = six.moves.urllib.request.Request(url)
        try:
            r1 = six.moves.urllib.request.urlopen(req)
        except URLError as e:
            p_rep = {}
            p_rep["STATE"] = "DEAD"
            return json.dumps(p_rep, sort_keys=True)

        return r1.read()

    def executeFSM(self,host,port,prefix,cmd,params):
        """
        Access to the FSM of a zdaq::baseApplication
   
        :param host: Host name
        :param port: Application port
        :param prefix: Prefix of the application , ie, http:://host:port/prefix/.....
        :param cmd: Transition
        :param params: Value of the content tag
   
        :return: url answer
        """
        if (params!=None):
            myurl = "http://"+host+ ":%d" % (port)
            lq={}
       
            lq["content"]=json.dumps(params,sort_keys=True)

            lq["command"]=cmd           
            lqs=six.moves.urllib.parse.urlencode(lq)
            saction = '/%s/FSM?%s' % (prefix,lqs)
            myurl=myurl+saction
            #print myurl
            req=six.moves.urllib.request.Request(myurl)

            r1=six.moves.urllib.request.urlopen(req)

            return r1.read()
        else:
            return '{"STATE"="FAILED","STATUS"="No Params given"}'
        

    def executeCMD(self,host,port,prefix,cmd,params):
        """
        Access to the CoMmanDs of a zdaq::baseApplication
        
        :param host: Host name
        :param port: Application port
        :param prefix: Prefix of the application , ie, http:://host:port/prefix/.....
        :param cmd: Command name
        :param params: CGI additional parameters
        :return: url answer
        """

        if (params!=None and cmd!=None):
            myurl = "http://"+host+ ":%d" % (port)

            lq={}
            lq["name"]=cmd
            for x,y in six.iteritems(params):
                lq[x]=y
            lqs=six.moves.urllib.parse.urlencode(lq)
            saction = '/%s/CMD?%s' % (prefix,lqs)
            myurl=myurl+saction

            req=six.moves.urllib.request.Request(myurl)
            try:
                r1=six.moves.urllib.request.urlopen(req)
            except URLError as e:
                p_rep={}
                p_rep["STATE"]="DEAD"
                return json.dumps(p_rep,sort_keys=True)
            else:
                return r1.read()

        else:
            myurl = "http://"+host+ ":%d/%s/" % (port,prefix)
            req=six.moves.urllib.request.Request(myurl)
            try:
                r1=six.moves.urllib.request.urlopen(req)
            except URLError as e:
                p_rep={}
                p_rep["STATE"]="DEAD"
                return json.dumps(p_rep,sort_keys=True)
            else:
                return r1.read()
    
    def getProcInfo(self):

        sr = self.executeRequest(self.url)
        #print("Avant ",sr,type(sr))
        if (type(sr) is bytes):
            sr=sr.decode("utf-8")
        #print(sr)
        self.procInfos = json.loads(sr)
        self.prefix="NONE"
        if (self.procInfos['STATE'] !="DEAD"): 
            self.pid = self.procInfos['PID']
            self.prefix = self.procInfos['PREFIX']
        self.fUrl = "http://%s:%d/%s" % (self.host, self.port, self.prefix)
        self.state = self.procInfos['STATE']

    def getInfo(self):
        self.getProcInfo()
        if (self.state == "DEAD"):
           return
        if (self.pid < 0):
            return
        if (self.isBaseApplication(self.procInfos)):
            sinf = self.sendCommand('INFO', {})
            if (type(sinf) is bytes):
                sinf=sinf.decode("utf-8")

            self.infos = json.loads(sinf)['answer']['INFO']
            self.appType = self.infos['name']
            self.appInstance = self.infos['instance']
            spar = self.sendCommand('GETPARAM', {})
            if (type(spar) is bytes):
                spar=spar.decode("utf-8")

            jpar=json.loads(spar)
            if ('PARAMETER' in jpar['answer']):
                self.params = json.loads(spar)['answer']['PARAMETER']
            else:
                self.params={}
    def allInfos(self):
        jdict={}
        jdict['infos']=self.infos
        jdict['params']=self.params
        jdict['process']=self.procInfos
        return jdict
    def isBaseApplication(self, m):
        base = False
        for key, value in m.items():
            if (key == 'CMD'):
                for x in value:
                    if (x['name'] == 'GETPARAM'):
                        base = True
        return base

    def sendCommand(self, name, content):
        self.getProcInfo()
        isValid = False
        for key, value in self.procInfos.items():
            if (key == 'CMD'):
                for x in value:
                    if (x['name'] == name):
                        isValid = True
        if (not isValid):
            return '{"answer":"invalid","status":"FAILED"}'
        
        #luri = "%s/CMD?name=%s" % (self.fUrl, name)
        #if (content != None):
        #    for key, value in content.items():
        #        luri = luri + "&%s=%s" % (key, value)

        #rep = self.executeRequest(luri)
        #return rep
        rep=self.executeCMD(self.host,self.port,self.prefix,name,content)
        #print(rep)
        if (type(rep) is bytes):
            rep=rep.decode("utf-8")
        return rep
    def sendTransition(self, name, content):
        self.getProcInfo()
        #print "Send Transition",self.procInfos
        isValid = False
        for key, value in self.procInfos.items():
            if (key == 'ALLOWED'):
                for x in value:
                    if (x['name'] == name):
                        isValid = True
        if (not isValid):
            return '{"answer":"invalid","status":"FAILED"}'

        #luri = "%s/FSM?command=%s&content=%s" % (
        #    self.fUrl, name, json.dumps(content))
        #rep = self.executeRequest(luri)
        #return rep
        rep=self.executeFSM(self.host,self.port,self.prefix,name,content)
        if (type(rep) is bytes):
            rep=rep.decode("utf-8")
        return rep

    def configure(self):
        self.sendTransition('CONFIGURE',{})
    def stop(self):
        self.sendTransition('STOP',{})
    def start(self):
        self.sendTransition('START',{})
    def status(self):
        return self.sendCommand('STATUS',{})
    
    def isSyx27(self):
        for k, v in self.procInfos.items():
            if (k == 'CMD'):
                for x in v:
                    if (x['name']=='SY_STATUS'):
                        return True
        return False
    def syx27_status(self,first=-1,last=-1):
        if (self.isSyx27()):
            p={}
            p["first"]=first
            p["last"]=last

            sr=self.sendCommand('SY_STATUS',p)
            #print(sr)
            sjr = json.loads(sr)
            x=sjr["answer"]["SY_STATUS"]
            for y in x["channels"]:
                    
                print("%12s %8.2f %8.2f %8.2f %8.2f %8.2f %d" %(y["name"],y["vset"],y["iset"],y["rampup"],y["vout"],y["iout"],y["status"]))                
           
            return x
    def syx27_vset(self,first,last,vset):
        if (self.isSyx27()):
            p={}
            p["first"]=first
            p["last"]=last
            p["value"]=vset
            sr=self.sendCommand('SY_VSET',p)
            self.syx27_status(first,last)
    def syx27_iset(self,first,last,iset):
        if (self.isSyx27()):
            p={}
            p["first"]=first
            p["last"]=last
            p["value"]=iset
            sr=self.sendCommand('SY_ISET',p)
            self.syx27_status(first,last)
    def syx27_rampup(self,first,last,ramp):
        if (self.isSyx27()):
            p={}
            p["first"]=first
            p["last"]=last
            p["value"]=ramp
            sr=self.sendCommand('SY_RAMPUP',p)
            self.syx27_status(first,last)
            
    def syx27_hvon(self,first,last):
        if (self.isSyx27()):
            p={}
            p["first"]=first
            p["last"]=last
            sr=self.sendCommand('SY_ON',p)
            self.syx27_status(first,last)
    def syx27_hvoff(self,first,last):
        if (self.isSyx27()):
            p={}
            p["first"]=first
            p["last"]=last
            sr=self.sendCommand('SY_OFF',p)
            self.syx27_status(first,last)

    def isWiener(self):
        for k, v in self.procInfos.items():
            if (k == 'CMD'):
                for x in v:
                    if (x['name']=='WP_STATUS'):
                        return True
        return False
    def wiener_status(self,first=-1,last=-1):
        if (self.isWiener()):
            p={}
            p["first"]=first
            p["last"]=last

            sr=self.sendCommand('WP_STATUS',p)
            sjr = json.loads(sr)
            x=sjr["answer"]["WP_STATUS"]
            for y in x["channels"]:
                sstat=y["status"].split("=")[1]
                    
                print("ch%.3d %8.2f %8.2f %8.2f %8.2f %8.2f %s" %(y["id"],y["vset"],y["iset"]*1E6,y["rampup"],y["vout"],y["iout"]*1E6,sstat[:len(sstat)-1]))                
           
            return x
    def wiener_vset(self,first,last,vset):
        if (self.isWiener()):
            p={}
            p["first"]=first
            p["last"]=last
            p["value"]=vset
            sr=self.sendCommand('WP_VSET',p)
            sr=self.sendCommand('WP_STATUS',p)
            sjr = json.loads(sr)
            x=sjr["answer"]["WP_STATUS"]
            for y in x["channels"]:
                sstat=y["status"].split("=")[1]
                    
                print("ch%.3d %8.2f %8.2f %8.2f %8.2f %8.2f %s" %(y["id"],y["vset"],y["iset"]*1E6,y["rampup"],y["vout"],y["iout"]*1E6,sstat[:len(sstat)-1]))                
           
            return x
    def wiener_hvon(self,first,last):
        if (self.isWiener()):
            p={}
            p["first"]=first
            p["last"]=last
            sr=self.sendCommand('WP_ON',p)
            sr=self.sendCommand('WP_STATUS',p)
            sjr = json.loads(sr)
            x=sjr["answer"]["WP_STATUS"]
            for y in x["channels"]:
                sstat=y["status"].split("=")[1]
                    
                print("ch%.3d %8.2f %8.2f %8.2f %8.2f %8.2f %s" %(y["id"],y["vset"],y["iset"]*1E6,y["rampup"],y["vout"],y["iout"]*1E6,sstat[:len(sstat)-1]))                
           
            return x
    def wiener_hvoff(self,first,last):
        if (self.isWiener()):
            p={}
            p["first"]=first
            p["last"]=last
            sr=self.sendCommand('WP_OFF',p)
            sr=self.sendCommand('WP_STATUS',p)
            sjr = json.loads(sr)
            x=sjr["answer"]["WP_STATUS"]
            for y in x["channels"]:
                sstat=y["status"].split("=")[1]
                print("ch%.3d %8.2f %8.2f %8.2f %8.2f %8.2f %s" %(y["id"],y["vset"],y["iset"]*1E6,y["rampup"],y["vout"],y["iout"]*1E6,sstat[:len(sstat)-1]))                
           
            return x
    def wiener_clear(self,first,last):
        if (self.isWiener()):
            p={}
            p["first"]=first
            p["last"]=last
            sr=self.sendCommand('WP_CLEARALARM',p)
            sr=self.sendCommand('WP_STATUS',p)
            sjr = json.loads(sr)
            x=sjr["answer"]["WP_STATUS"]
            for y in x["channels"]:
                sstat=y["status"].split("=")[1]
                print("ch%.3d %8.2f %8.2f %8.2f %8.2f %8.2f %s" %(y["id"],y["vset"],y["iset"]*1E6,y["rampup"],y["vout"],y["iout"]*1E6,sstat[:len(sstat)-1]))
            
           
            return x
    def wiener_iset(self,first,last,iset):
        if (self.isWiener()):
            p={}
            p["first"]=first
            p["last"]=last
            p["value"]=iset*1E-6
            sr=self.sendCommand('WP_ISET',p)
            sr=self.sendCommand('WP_STATUS',p)
            sjr = json.loads(sr)
            x=sjr["answer"]["WP_STATUS"]
            for y in x["channels"]:
                sstat=y["status"].split("=")[1]
                print("ch%.3d %8.2f %8.2f %8.2f %8.2f %8.2f %s" %(y["id"],y["vset"],y["iset"]*1E6,y["rampup"],y["vout"],y["iout"]*1E6,sstat[:len(sstat)-1]))            
            return x
    def wiener_rampup(self,first,last,ramp):
        if (self.isWiener()):
            p={}
            p["first"]=first
            p["last"]=last
            p["value"]=ramp
            sr=self.sendCommand('WP_RAMPUP',p)
            sr=self.sendCommand('WP_STATUS',p)
            sjr = json.loads(sr)
            x=sjr["answer"]["WP_STATUS"]
            for y in x["channels"]:
                sstat=y["status"].split("=")[1]
                print("ch%.3d %8.2f %8.2f %8.2f %8.2f %8.2f %s" %(y["id"],y["vset"],y["iset"]*1E6,y["rampup"],y["vout"],y["iout"]*1E6,sstat[:len(sstat)-1]))            
            return x


    def isBmp(self):
        for k, v in self.procInfos.items():
            if (k == 'CMD'):
                for x in v:
                    if (x['name']=='BMP_STATUS'):
                        return True
        return False
    def bmp_status(self):
        if (self.isBmp()):
            sr=self.sendCommand('BMP_STATUS',{})
            sjr = json.loads(sr)
            x=sjr["answer"]["BMP_STATUS"]
            print(" P=%.2f mbar T=%.2f K " % (x["pressure"],x["temperature"]+273.15))
            return x

    def isHih(self):
        for k, v in self.procInfos.items():
            if (k == 'CMD'):
                for x in v:
                    if (x['name']=='HIH_STATUS'):
                        return True
        return False
    def hih_status(self):
        if (self.isHih()):
            sr=self.sendCommand('HIH_STATUS',{})
            sjr = json.loads(sr)
            x=sjr["answer"]["HIH_STATUS"]
            print(" H0=%.2f %% T0=%.2f K H1=%.2f %% T1=%.2f K " % (x["humidity0"],x["temperature0"],x["humidity1"],x["temperature1"]))
            return x
    
    def isGenesys(self):
        for k, v in self.procInfos.items():
            if (k == 'CMD'):
                for x in v:
                    if (x['name']=='GS_STATUS'):
                        return True
        return False
    
    def genesys_on(self):
        if (self.isGenesys()):
            self.sendCommand('GS_ON',{})
    def genesys_off(self):
        if (self.isGenesys()):
            self.sendCommand('GS_OFF',{})
    def genesys_status(self):
        if (self.isGenesys()):
            sr=self.sendCommand('GS_STATUS',{})
            sjr = json.loads(sr)
            x=sjr["answer"]["GS_STATUS"]
            print(" VSET=%.2f V VOut=%.2f V IOut=%.2f V Status %s " % (x["vset"],x["vout"],x["iout"],x["status"]))
            return x
        
    def isZup(self):
        for k, v in self.procInfos.items():
            if (k == 'CMD'):
                for x in v:
                    if (x['name']=='ZUP_STATUS'):
                        return True
        return False
    
    def zup_on(self):
        if (self.isZup()):
            self.sendCommand('ZUP_ON',{})
    def zup_off(self):
        if (self.isZup()):
            self.sendCommand('ZUP_OFF',{})
    def zup_status(self):
        if (self.isZup()):
            sr=self.sendCommand('ZUP_STATUS',{})
            sjr = json.loads(sr)
            x=sjr["answer"]["ZUP_STATUS"]
            print(" VSET=%.2f V VOut=%.2f V IOut=%.2f V Status %s " % (x["vset"],x["vout"],x["iout"],x["status"]))
            return x
        
    def printInfos(self, vverb):
        if (vverb):
            #print "PROCINFO ",self.procInfos
            print("\n FSM is %s on %s, PID %d Service %s" % (self.procInfos["STATE"], self.url, self.procInfos["PID"], self.procInfos["PREFIX"]))
            # print COMMAND and TRANSITION
            for k, v in self.procInfos.items():
                if (k == 'ALLOWED' or k == 'CMD' or k == 'FSM'):
                    s = " \t %s \t" % k
                    for x in v:
                        s = s + x['name'] + " "
                    print(s)
            
            if (self.isBaseApplication(self.procInfos)):        
                print("\n BaseApplication %s _ %d" % (self.infos["name"], self.infos["instance"]))
                print("\t Parameters")
                for k, v in self.params.items():
                    print("\t \t", k, v)
        else:
            print("FSM is %s on %s, PID %d Service %s"  % (self.procInfos["STATE"], self.url, self.procInfos["PID"], self.procInfos["PREFIX"]))
            
