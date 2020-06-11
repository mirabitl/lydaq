import lydaqrc
import time
import MongoJob as mg
class febRC(lydaqrc.lydaqControl):
    
    #daq
    def daq_initialise(self,reset=0):
        m={}
        r={}
        s=json.loads(self.appMap['MDCCSERVER'][0].sendTransition("OPEN", m))
        r["MDCCSERVER"] = s
        
        for x in self.appMap["BUILDER"]:
            s=json.loads(x.sendTransition("CONFIGURE", m))
            r["BUILDER_%d" % x.appInstance]=s

        if (reset!=0):
            self.mdcc_resetTdc()
            time.sleep(reset)
            
        for x in self.appMap["TDCSERVER"]:
            s=json.loads(x.sendTransition("INITIALISE", m))
            r["TDCSERVER_%d" % x.appInstance]=s
            
        return json.dumps(r)

    def daq_configure(self):
        m={}
        r={}
        for x in self.appMap["TDCSERVER"]:
            s=json.loads(x.sendTransition("CONFIGURE", m))
            r["TDCSERVER_%d" % x.appInstance]=s
        return json.dumps(r)

    
    def daq_stop(self):
        m={}
        r={}
        s=json.loads(self.appMap['MDCCSERVER'][0].sendTransition("PAUSE", m))
        r["MDCCSERVER"] = s

        for x in self.appMap["TDCSERVER"]:
            s=json.loads(x.sendTransition("STOP", m))
            r["TDCSERVER_%d" % x.appInstance]=s

        for x in self.appMap["BUILDER"]:
            s=json.loads(x.sendTransition("STOP", m))
            r["BUILDER_%d" % x.appInstance]=s

        return json.dumps(r)

    def daq_destroy(self):
        m={}
        r={}
        for x in self.appMap["TDCSERVER"]:
            s=json.loads(x.sendTransition("DESTROY", m))
            r["TDCSERVER_%d" % x.appInstance]=s

        return json.dumps(r)

    def daq_start(self,run,location="UNKNOWN",comment="Not set"):
        nrun=run
        if (run==0):
            smg=mg.instance()
            nrun=smg.getRun(location,comment)
        r={}
        m={}
        m['run']=nrun
        for x in self.appMap["BUILDER"]:
            s=json.loads(x.sendTransition("START", m))
            r["BUILDER_%d" % x.appInstance]=s

        m={}
        for x in self.appMap["TDCSERVER"]:
            s=json.loads(x.sendTransition("START", m))
            r["TDCSERVER_%d" % x.appInstance]=s

        s=json.loads(self.appMap['MDCCSERVER'][0].sendTransition("RESET", m))
        s=json.loads(self.appMap['MDCCSERVER'][0].sendTransition("ECALRESUME", m))
        r["MDCCSERVER"] = s

        return json.dumps(r)
        
    def SourceStatus(self):
        rep={}
        for  k,v in self.appMap.items:
            if (k != "TDCSERVER"):
                continue
            for s in v:
                mr=json.loads(s.sendCommand("STATUS",None))
                if (mr['status']!="FAILED"):
                    rep["%s_%d" % (s.host,s.infos['instance'])]=mr["answer"]["TDCSTATUS"]
                else:
                    rep["%s_%d" % (s.host,s.infos['instance'])]=mr

                rep["%s_%d" % (s.host,s.infos['instance'])]=r
        return json.dumps(rep)

    #FEBV1 specific
    
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

    def setTdcMask(self,channelmask,asicmask):
        param={}
        param["value"] = channelmask;
        param["asic"] = asicmask;
        return  self.processCommand("SETMASK", "TDCSERVER", param)

    def tdcLUTCalib(self,instance,channel):
        if (not "TDCSERVER" in self.appMap):
            return '{"answer":"NOTDCSERVER","status":"FAILED"}'
        if (len(self.appMap["TDCSERVER"])<=instance):
            return '{"answer":"InvalidInstance","status":"FAILED"}'

        tdc = appMap["TDCSERVER"][instance]
        n=(1<<channel)
        param={}
        param["value"]="%x" % n
        r={}
        r["cal_mask"]=json.loads(tdc.sendCommand("CALIBMASK", param))
        r["cal_status"]=json.loads(tdc.sendCommand("CALIBSTATUS", param))
        return json.dumps(r)

    def tdcLUTDump(self,instance,channel):
        if (not "TDCSERVER" in self.appMap):
            return '{"answer":"NOTDCSERVER","status":"FAILED"}'
        if (len(self.appMap["TDCSERVER"])<=instance):
            return '{"answer":"InvalidInstance","status":"FAILED"}'

        tdc = appMap["TDCSERVER"][instance]
        param={}
        param["value"]=channel
        r={}
        r["lut_%d" % channel]=json.loads(tdc.sendCommand("GETLUT", param))
        return json.dumps(r)

    def tdcLUTMask(self,instance,mask):
        if (not "TDCSERVER" in self.appMap):
            return '{"answer":"NOTDCSERVER","status":"FAILED"}'
        if (len(self.appMap["TDCSERVER"])<=instance):
            return '{"answer":"InvalidInstance","status":"FAILED"}'

        tdc = appMap["TDCSERVER"][instance]
        param={}
        param["value"]=mask
        r={}
        r["test_mask"]=json.loads(tdc.sendCommand("TESTMASK", param))
        r["cal_status"]=json.loads(tdc.sendCommand("CALIBSTATUS", param))
        return json.dumps(r)

    def febScurve(self,ntrg,ncon,ncoff,thmin,thmax,step):
        r={}
        self.mdcc_Pause()
        self.mdcc_setSpillOn(ncon)
        print " Clock On %d Off %d" % (ncon,ncoff)
        self.mdcc_setSpillOff(ncoff)
        self.mdcc_setSpillRegister(4)
        self.mdcc_CalibOn(1)
        self.mdcc_setCalibCount(ntrg)
        self.mdcc_Status()
        thrange = (thmax - thmin + 1) / step;
        for vth in range(0,thrange+1):
            self.mdcc_Pause();
            self.setVthTime(thmax - vth * step);
            time.sleep(0.5)
            self.builder_setHeader(2, thmax - vth * step, 0xFF)

            #Check Last built event
            sr = json.loads(self.BuilderStatus())
            firstEvent = sr["event"]

            #Resume Calibration
            self.mdcc_ReloadCalibCount()
            self.mdcc_Resume()
            self.mdcc_Status()

            # Wait for ntrg events capture
            lastEvent = firstEvent
            nloop = 0
            while (lastEvent < (firstEvent + ntrg - 20)):
                sr = json.loads(self.BuilderStatus());
                lastEvent = sr["event"]
                print " %d First %d  Last %d Step %d" (thmax-vth*step,firstEvent,lastEvent,step)
                time.sleep(0.4)
                nloop=nloop+1
                if (nloop > 20):
                    break
      
            r["TH_%d" % (thmax-vth*step)] = lastEvent - firstEvent + 1;

            # End Point
            self.mdcc_CalibOn(0)
            self.mdcc_Pause()
        return json.dumps(r)

    def runScurve(self,run,ch,spillon,spilloff,beg,las,step=2,asic=255,Comment="PR2 Calibration",Location="UNKNOWN",nevmax=50):
        firmware=[3,4,5,6,7,8,9,10,11,12,20,21,22,23,24,26,28,30]
        self.daq_start(run, location=Location, comment=Comment)
        r={}
        r["run"]=run
        if (ch==255):
            print "Run Scurve on all channel together"
            mask = 0
            for i in firmware:
                mask = mask | (1 << i)
            self.setTdcMask(mask, asic)
            r["S_%d" %ch] = json.loads(self.febSCurve(nevmax, spillon, spilloff, beg, las, step))
            self.daq_stop();
            return json.dumps(r)
        if (ch==1023):
            print "Run Scurve on all channel one by one"
            mask = 0
            for i in firmware:
                print "SCurve for channel %d " % i
                mask = mask | (1 << i)
                self.setTdcMask(mask, asic)
                r["S_%d" %ch] = json.loads(self.febSCurve(nevmax, spillon, spilloff, beg, las, step))
            self.daq_stop();
            return json.dumps(r)
        print "Run Scurve on  channel %d " % ch
        mask = 0
        mask = mask | (1 << i)
        self.setTdcMask(mask, asic)
        r["S_%d" %ch] = json.loads(self.febSCurve(nevmax, spillon, spilloff, beg, las, step))
        self.daq_stop();
        return json.dumps(r)
        
