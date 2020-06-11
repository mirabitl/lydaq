import os
import socks
import socket
import httplib
import urllib
import urllib2
from urllib2 import URLError, HTTPError
import json
from copy import deepcopy
import base64
import time
import requests

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


class FSMAccess:
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
        print "Access to %s " % url

        req = urllib2.Request(url)
        try:
            r1 = urllib2.urlopen(req)
        except URLError, e:
            p_rep = {}
            p_rep["STATE"] = "DEAD"
            return json.dumps(p_rep, sort_keys=True)

        return r1.read()

    def getProcInfo(self):

        sr = self.executeRequest(self.url)
        self.procInfos = json.loads(sr)
        self.pid = self.procInfos['PID']
        self.prefix = self.procInfos['PREFIX']
        self.fUrl = "http://%s:%d/%s" % (self.host, self.port, self.prefix)
        self.state = self.procInfos['STATE']

    def getInfo(self):
        self.getProcInfo()
        if (self.pid < 0):
            return
        if (self.isBaseApplication(self.procInfos)):
            sinf = self.sendCommand('INFO', None)
            infos = json.loads(sinf)['answer']['INFO']
            self.appType = infos['name']
            self.appInstance = infos['instance']
            spar = self.sendCommand('GETPARAM', None)
            self.params = json.loads(spar)['answer']['PARAMETER']

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

        luri = "%s/CMD?name=%s" % (self.fUrl, name)
        if (content != None):
            for key, value in content.items():
                luri = luri + "&%s=%s" % (key, value)

        rep = self.executeRequest(luri)
        return rep

    def sendTransition(self, name, content):
        self.getProcInfo()
        isValid = False
        for key, value in self.procInfos.items():
            if (key == 'ALLOWED'):
                for x in value:
                    if (x['name'] == name):
                        isValid = True
        if (not isValid):
            return '{"answer":"invalid","status":"FAILED"}'

        luri = "%s/CMD?command=%s&content=%s" % (
            self.fUrl, name, json.dumps(content))
        rep = self.executeRequest(luri)
        return rep

    def printInfos(self, vverb):
        if (vverb):
            print "\n FSM is %s on %s, PID %s Service %s" (self.procInfos["STATE"], self.url, self.procInfos["PID"], self.procInfos["PREFIX"])
            # print COMMAND and TRANSITION
            for k, v in self.procInfos.items():
                if (k == 'ALLOWED' or k == 'CMD' or k == 'FSM'):
                    s = " %s \t" % k
                    for x in v:
                        s = s + x['name'] + " "
                    print(s)
            print "\n BaseApplication %s _ %d" % (self.infos["name"], self.infos["instance"])
            print "Parameters"
            for k, v in self.params.items():
                print "\t", k, v
        else:
            print "FSM is %s on %s, PID %s Service %s" (self.procInfos["STATE"], self.url, self.procInfos["PID"], self.procInfos["PREFIX"])
