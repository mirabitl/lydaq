import lydaqrc
import time

class febRC(lydaqrc.lydaqControl):
    
    
    def SourceStatus(self):
        rep={}
        for ( k,v in self.appMap.items):
            if (k != "TDCSERVER"):
                continue
            for (s in v):
                mr=json.loads(s.sendCommand("STATUS",None))
                if (mr['status']!="FAILED"):
                    rep["%s_%d" % (s.host,s.infos['instance'])]=mr["answer"]["TDCSTATUS"]
                else:
                    rep["%s_%d" % (s.host,s.infos['instance'])]=mr

                rep["%s_%d" % (s.host,s.infos['instance'])]=r
        return json.dumps(rep)


    def set6BDac(self,dac):
        param={}
        param["value"]=dac
        return self.processCommand("SET6BDAC", "TDCSERVER", param)

    def cal6BDac(self,mask,shift):
        param={}
        param["shift"]=shift
        param["mask"]=int(mask,16)
        return self.processCommand("CAL6BDAC", "TDCSERVER", param)

    def setVthTime(self,Threshold):
        param={}
        param["value"]=Threshold
        return self.processCommand("SETVTHTIME", "TDCSERVER", param)
    
    def setTdcMode(self,mode):
        param={}
        param["value"]=mode
        return self.processCommand("SETMODE", "TDCSERVER", param)
        
    def setTdcDelays(self,active,dead):
        param={}
        param["value"]=active
        r={}
        r["active"]=json.loads( self.processCommand("SETDELAY", "TDCSERVER", param))
        
