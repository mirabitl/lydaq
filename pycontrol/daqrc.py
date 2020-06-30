from __future__ import absolute_import
from __future__ import print_function
import rcbase
import MongoJob as mg
import json
import time

class daqControl:
    def __init__(self,account,config):
        self.account=account
        self.config=config
        self.jobcontrols=[]
        self.appMap={}
        self.db=mg.instance()
    def parseMongo(self):
        self.db.downloadConfig(self.config.split(":")[0],int(self.config.split(":")[1]),True)
        daq_file="/dev/shm/mgjob/"+self.config.split(":")[0]+"_"+self.config.split(":")[1]+".json"
        with open(daq_file) as data_file:    
            self._mgConfig = json.load(data_file)
            
    def discover(self):
        if (not "HOSTS" in self._mgConfig):
            return
        self.appMap={}
        mh = self._mgConfig['HOSTS'];
        for  key,value in mh.items():
            #print "Host found %s" % key
            fsm = rcbase.FSMAccess(key, 9999);
            self.jobcontrols.append(fsm)
    
        for x in self.jobcontrols:
            x.getInfo()
            if (x.state == "FAILED"):
                print("Failed request %s exiting" % x.url)
                exit(0)
      
            s = x.sendCommand("STATUS",{})
            m = json.loads(s)
            #print s
            if (not 'JOBS' in m['answer']):
                print("%s has NO Jobs : %s" % (x.url,s))
            else:
                if (m['answer']['JOBS'] != None):
                    for  pcs in m['answer']['JOBS']:
                        if (pcs['STATUS'].split(' ')[0] == 'X'):
                            print(pcs)
                            continue
          
                        bapp = rcbase.FSMAccess(pcs['HOST'], int(pcs['PORT']))
                        bapp.getInfo();
                        if (not bapp.infos['name'] in self.appMap):
                            l=[]
                            l.append(bapp)
                            self.appMap[bapp.infos['name']]=l
                        else:
                            self.appMap[bapp.infos['name']].append(bapp) 
    
    def updateInfo(self,printout,vverbose):
        if (printout):
            print("""
        \t \t *****************************    
        \t \t ** Application information **
        \t \t *****************************
        """)
        if (len(self.appMap)==0):
            print("No Application Map found. Please Connect first or create process")
        for k,v in self.appMap.items():
            for x in v:
                x.getInfo()
                if (printout):
                    x.printInfos(vverbose)
                    
    def processCommand(self,cmd,appname,param):
        r={}
        if (not appname in self.appMap):
            return '{"answer":"invalidname","status":"FAILED"}'
        for x in self.appMap[appname]:
            s=json.loads(x.sendCommand(cmd,param))
            r["%s_%d" % (appname,x.appInstance)]=s
        return json.dumps(r)
  
    # JOB Control
    def jc_transition(self,Transition,par):
        rep={}
        if (len(self.jobcontrols)==0):
            print("No jobcontrols found. Please Connect first")
            exit(0)
        for x in self.jobcontrols:
            print("Calling",Transition,par)
            ans=x.sendTransition(Transition,par)
            rep["%s" % x.host] = json.loads(ans)
        return json.dumps(rep)
    
    def jc_command(self,Command,par):
        rep={}
        if (len(self.jobcontrols)==0):
            print("No jobcontrols found. Please Connect first")
            exit(0)
        for x in self.jobcontrols:
            ans=x.sendCommand(Command,par)
            rep["%s" % x.host] = json.loads(ans)
        return json.dumps(rep)

    # Transition
    def jc_initialise(self):
        par={}
        par['mongo']=self.config
        print("Initialising",par)
        return self.jc_transition("INITIALISE",par)
    def jc_start(self):
        return self.jc_transition("START",{})
    def jc_kill(self):
        return self.jc_transition("KILL",{})
    def jc_destroy(self):
        return self.jc_transition("DESTROY",{})
    def jc_status(self):
        return self.jc_command("STATUS",{})
    def jc_appcreate(self):
        return self.jc_command("APPCREATE",{})
    
