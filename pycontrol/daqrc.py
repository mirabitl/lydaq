import rcbase
import MongoJob as mg

class daqControl:
    def __init__(self,account,config):
        self.account=account
        self.config=config
        self.jobcontrols=[]
        self.appMap={}

    def parseMongo(self):
        mgi=mg.instance()
        mgi.downloadConfig(config.split(":")[0],config.split(":")[1],True)
        daq_file="/dev/shm/mgjob/"+config.split(":")[0]+"_"+config.split(":")[1]+".json"
        with open(daq_file) as data_file:    
            self._mgConfig = json.load(data_file)
            
    def discover(self):
        if (not "HOSTS" in self._mgConfig):
            return
        self.appMap={}
        mh = _mgConfig['HOSTS'];
        for ( key,value in mh.items):
            print "Host found %s" % key
            fsm = rcbase.FSMaccess(key, 9999);
            self.jobcontrols.append(fsm)
    
        for (x in _jobcontrols):
            x.getInfo()
            if (x.state == "FAILED"):
                print "Failed request %s exiting" % x.url
                exit(0)
      
            s = x.sendCommand("STATUS",None)
            m = json.loads(s)

            if (not 'JOBS' in m['answer']):
                print "%s has NO Jobs : %s" % (x.url,s)
            else:
                for ( pcs in m['answer']['JOBS']):
                    if (pcs['STATUS'].split(' ')[0] == 'X'):
                        print pcs
                    continue
          
          bapp = FSMaccess(pcs['HOST'], int(pcs['PORT']))
          bapp.getInfo();
          if (not bapp.infos['name'] in self.appMap):
              l=[]
              l.append(bapp)
              self.appMap[bapp.infos['name']]=l
          else:
              self.appMap[bapp.infos['name']].append(bapp) 
    
    def updateInfo(self,printout,vverbose):
        if (len(self.appMap)==0):
            print "No Application Map found. Please Connect first or create process"
        for (k,v in self.appMap.items()):
            for (x in v):
                x.getInfo()
                if (printout):
                    x.printInfos(vverbose)
                    
    def processCommand(self,cmd,appname,param):
        r={}
        if (not appname in self.appMap):
            return '{"answer":"invalidname","status":"FAILED"}'
        for (x in self.appMap[appname]):
            s=json.loads(x.sendCommand(cmd,param))
            r["%s_%d" % (appname,x.appInstance)]=s
        return json.dumps(r)
  
    # JOB Control
    def jc_transition(self,Transition,par):
        rep={}
        if (len(self.jobcontrols)==0):
            print "No jobcontrols found. Please Connect first"
            exit(0)
        for (x in self.jobcontrols):
            ans=x.sendTransition(Transition,par)
            rep["%s" % x.host] = json.loads(ans)
        return json.dumps(rep)
    
    def jc_command(self,Command,par):
        rep={}
        if (len(self.jobcontrols)==0):
            print "No jobcontrols found. Please Connect first"
            exit(0)
        for (x in self.jobcontrols):
            ans=x.sendCommand(Command,par)
            rep["%s" % x.host] = json.loads(ans)
        return json.dumps(rep)

    # Transition
    def jc_initialise(self):
        par={}
        par['mongo']=self.config
        return this.jc_transition("INITIALISE",par)
    def jc_start(self):
        return this.jc_transition("START",None)
    def jc_kill(self):
        return this.jc_transition("KILL",None)
    def jc_destroy(self):
        return this.jc_transition("DESTROY",None)
    def jc_status(self):
        return this.jc_command("STATUS",None)
    def jc_appcreate(self):
        return this.jc_command("APPCREATE",None)
    
