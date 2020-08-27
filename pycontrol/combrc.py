from __future__ import absolute_import
from __future__ import print_function
import lydaqrc
import time
import MongoJob as mg
import json
import os
from six.moves import range


class combRC(lydaqrc.lydaqControl):
    def __init__(self,account,config):
        super().__init__(account,config)
        self.reset=0
        self.comment="Not yet set"
        self.location="UNKNOWN"
    # daq
    # Initialising implementation
    def daq_initialising(self):
        m = {}
        r = {}
        # old DIF Fw
        if ("GPIOSERVER" in self.appMap.keys()):
            s = json.loads(self.appMap['GPIOSERVER'][0].sendTransition("CONFIGURE", m))
            r["GPIOSERVER"] = s
            json.loads(self.appMap['GPIOSERVER'][0].sendCommand("VMEON", {}))
            json.loads(self.appMap['GPIOSERVER'][0].sendCommand("VMEOFF", {}))
            json.loads(self.appMap['GPIOSERVER'][0].sendCommand("VMEON", {}))
            time.sleep(5)
        if ("CCCSERVER" in self.appMap.keys()):
            s = json.loads(self.appMap['CCCSERVER'][0].sendTransition("OPEN", m))
            s = json.loads(self.appMap['CCCSERVER'][0].sendTransition("INITIALISE", m))
            s = json.loads(self.appMap['CCCSERVER'][0].sendTransition("CONFIGURE", m))
            json.loads(self.appMap['CCCSERVER'][0].sendTransition("STOP", m))
            time.sleep(1.);
            json.loads(self.appMap['CCCSERVER'][0].sendCommand("CCCRESET", {}))
            json.loads(self.appMap['CCCSERVER'][0].sendCommand("DIFRESET", {}))
            r["CCCSERVER"] = s
        # Mdcc 
        if ("MDCCSERVER" in self.appMap.keys()):
            s = json.loads(self.appMap['MDCCSERVER'][0].sendTransition("OPEN", m))
            r["MDCCSERVER"] = s
        # Builder
        for x in self.appMap["BUILDER"]:
            s = json.loads(x.sendTransition("CONFIGURE", m))
            r["BUILDER_%d" % x.appInstance] = s
        # Reset for FEB V1
        if (self.reset != 0):
            self.mdcc_resetTdc()
            time.sleep(self.reset/1000.)

        if ("TDCSERVER" in self.appMap.keys()):
            for x in self.appMap["TDCSERVER"]:
                s = json.loads(x.sendTransition("INITIALISE", m))
                r["TDCSERVER_%d" % x.appInstance] = s
        
        if ("PMRMANAGER" in self.appMap.keys()):
            for x in self.appMap["PMRMANAGER"]:
                s = json.loads(x.sendTransition("SCAN", m))
                r["PMRMANAGER_%d" % x.appInstance] = s

            for x in self.appMap["PMRMANAGER"]:
                s = json.loads(x.sendTransition("INITIALISE", m))
                r["PMRMANAGER_%d" % x.appInstance] = s

        if ("GRICSERVER" in self.appMap.keys()):
            for x in self.appMap["GRICSERVER"]:
                s = json.loads(x.sendTransition("INITIALISE", m))
                r["GRICSERVER_%d" % x.appInstance] = s
        # Old DIF Fw
        if ("DIFMANAGER" in self.appMap.keys()):
            for x in self.appMap["DIFMANAGER"]:
                s = json.loads(x.sendTransition("SCAN", m))
                r["DIFMANAGER_SCAN_%d" % x.appInstance] = s

            for x in self.appMap["DIFMANAGER"]:
                s = json.loads(x.sendTransition("INITIALISE", m))
                r["DIFMANAGER_INIT_%d" % x.appInstance] = s

        self.daq_answer=json.dumps(r)
        self.storeState()

    def daq_configuring(self):
        m = {}
        r = {}
        if ("TDCSERVER" in self.appMap.keys()):
            for x in self.appMap["TDCSERVER"]:
                s = json.loads(x.sendTransition("CONFIGURE", m))
                r["TDCSERVER_%d" % x.appInstance] = s
        if ("PMRMANAGER" in self.appMap.keys()):
            for x in self.appMap["PMRMANAGER"]:
                s = json.loads(x.sendTransition("CONFIGURE", m))
                r["PMRMANAGER_%d" % x.appInstance] = s
        if ("GRICSERVER" in self.appMap.keys()):
            for x in self.appMap["GRICSERVER"]:
                s = json.loads(x.sendTransition("CONFIGURE", m))
                r["GRICSERVER_%d" % x.appInstance] = s
        #Old DIF Firmware
        if ("CCCSERVER" in self.appMap.keys()):
            json.loads(self.appMap['CCCSERVER'][0].sendCommand("CCCRESET", {}))
            s=json.loads(self.appMap['CCCSERVER'][0].sendCommand("DIFRESET", {}))
            r["CCCSERVER"] = s
        if ("DIFMANAGER" in self.appMap.keys()):
            for x in self.appMap["DIFMANAGER"]:
                s = json.loads(x.sendTransition("CONFIGURE", m))
                r["DIFMANAGER_%d" % x.appInstance] = s
        self.daq_answer= json.dumps(r)
        self.storeState()
       
    def daq_stopping(self):
        m = {}
        r = {}
        if ("MDCCSERVER" in self.appMap.keys()):
            s = json.loads(self.appMap['MDCCSERVER'][0].sendTransition("PAUSE", m))
            r["MDCCSERVER"] = s

        if ("TDCSERVER" in self.appMap.keys()):
            for x in self.appMap["TDCSERVER"]:
                s = json.loads(x.sendTransition("STOP", m))
                r["TDCSERVER_%d" % x.appInstance] = s
                
        if ("PMRMANAGER" in self.appMap.keys()):
            for x in self.appMap["PMRMANAGER"]:
                s = json.loads(x.sendTransition("STOP", m))
                r["PMRMANAGER_%d" % x.appInstance] = s

        if ("GRICSERVER" in self.appMap.keys()):
            for x in self.appMap["GRICSERVER"]:
                s = json.loads(x.sendTransition("STOP", m))
                r["GRICSERVER_%d" % x.appInstance] = s
        #Old DIF fw
        if ("CCCSERVER" in self.appMap.keys()):
            s=json.loads(self.appMap['CCCSERVER'][0].sendTransition("STOP", m))
            r["CCCSERVER"] = s
        if ("DIFMANAGER" in self.appMap.keys()):
            for x in self.appMap["DIFMANAGER"]:
                s = json.loads(x.sendTransition("STOP", m))
                r["DIFMANAGER_%d" % x.appInstance] = s
        
        for x in self.appMap["BUILDER"]:
            s = json.loads(x.sendTransition("STOP", m))
            r["BUILDER_%d" % x.appInstance] = s

        self.daq_answer= json.dumps(r)
        self.storeState()

    def daq_destroying(self):
        m = {}
        r = {}
        if ("TDCSERVER" in self.appMap.keys()):
            for x in self.appMap["TDCSERVER"]:
                s = json.loads(x.sendTransition("DESTROY", m))
                r["TDCSERVER_%d" % x.appInstance] = s
        if ("PMRMANAGER" in self.appMap.keys()):
            for x in self.appMap["PMRMANAGER"]:
                s = json.loads(x.sendTransition("DESTROY", m))
                r["PMRMANAGER_%d" % x.appInstance] = s
        if ("GRICSERVER" in self.appMap.keys()):
            for x in self.appMap["GRICSERVER"]:
                s = json.loads(x.sendTransition("DESTROY", m))
                r["GRICSERVER_%d" % x.appInstance] = s
        #old DIF Fw
        if ("DIFMANAGER" in self.appMap.keys()):
            for x in self.appMap["DIFMANAGER"]:
                s = json.loads(x.sendTransition("DESTROY", m))
                r["DIFMANAGER_%d" % x.appInstance] = s


        self.daq_answer=json.dumps(r)
        self.storeState()

    def daq_starting(self):

        if (self.location == "UNKNOWN"):
            self.location = os.getenv("DAQSETUP", "UNKNOWN")
       
        jnrun = self.db.getRun(self.location, self.comment)
        r = {}
        m = {}
        # print "EVENT BUILDER",jnrun['run']
        m['run'] = jnrun['run']
        for x in self.appMap["BUILDER"]:
            print("Sending Start to vent builder")
            s = json.loads(x.sendTransition("START", m))
            r["BUILDER_%d" % x.appInstance] = s

        m = {}
        if ("TDCSERVER" in self.appMap.keys()):
            for x in self.appMap["TDCSERVER"]:
                s = json.loads(x.sendTransition("START", m))
                r["TDCSERVER_%d" % x.appInstance] = s
        if ("PMRMANAGER" in self.appMap.keys()):
            for x in self.appMap["PMRMANAGER"]:
                s = json.loads(x.sendTransition("START", m))
                r["PMRMANAGER_%d" % x.appInstance] = s
        if ("GRICSERVER" in self.appMap.keys()):
            for x in self.appMap["GRICSERVER"]:
                s = json.loads(x.sendTransition("START", m))
                r["GRICSERVER_%d" % x.appInstance] = s
        if ("MDCCSERVER" in self.appMap.keys()):
            s = json.loads(self.appMap['MDCCSERVER'][0].sendTransition("RESET", m))
            s = json.loads(self.appMap['MDCCSERVER']
                           [0].sendTransition("ECALRESUME", m))
            r["MDCCSERVER"] = s


        #old firmware
        if ("DIFMANAGER" in self.appMap.keys()):
            for x in self.appMap["DIFMANAGER"]:
                s = json.loads(x.sendTransition("START", m))
                r["DIFMANAGER_%d" % x.appInstance] = s

        if ("CCCSERVER" in self.appMap.keys()):
            s=json.loads(self.appMap['CCCSERVER'][0].sendTransition("START", m))
            r["CCCSERVER"] = s

        self.daq_answer= json.dumps(r)
        self.storeState()

    def SourceStatus(self, verbose=False):
        rep = {}
        for k, v in self.appMap.items():
            if (k != "TDCSERVER" ):
                continue
            for s in v:
                mr = json.loads(s.sendCommand("STATUS", {}))
                if (mr['status'] != "FAILED"):
                    rep["%s_%d" % (s.host, s.infos['instance'])
                        ] = mr["answer"]["TDCSTATUS"]
                else:
                    rep["%s_%d" % (s.host, s.infos['instance'])] = mr

        for k, v in self.appMap.items():
            if (k != "PMRMANAGER" ):
                continue
            for s in v:
                mr = json.loads(s.sendCommand("STATUS", {}))
                if (mr['status'] != "FAILED"):
                    rep["%s_%d" % (s.host, s.infos['instance'])
                        ] = mr["answer"]["DIFLIST"]
                else:
                    rep["%s_%d" % (s.host, s.infos['instance'])] = mr

                    #rep["%s_%d" % (s.host, s.infos['instance'])] = r
        for k, v in self.appMap.items():
            if (k != "GRICSERVER" ):
                continue
            for s in v:
                mr = json.loads(s.sendCommand("STATUS", {}))
                if (mr['status'] != "FAILED"):
                    rep["%s_%d" % (s.host, s.infos['instance'])
                        ] = mr["answer"]["GRICSTATUS"]
                else:
                    rep["%s_%d" % (s.host, s.infos['instance'])] = mr

                    #rep["%s_%d" % (s.host, s.infos['instance'])] = r
        for k, v in self.appMap.items():
            if (k != "DIFMANAGER"):
                continue
            for s in v:
                mr = json.loads(s.sendCommand("STATUS", {}))
                #print(mr)
                if (mr['status'] != "FAILED"):
                    rep["%s_%d" % (s.host, s.infos['instance'])
                        ] = mr["answer"]["DIFLIST"]
                else:
                    rep["%s_%d" % (s.host, s.infos['instance'])] = mr
        if (not verbose):
            return json.dumps(rep)
        # Verbose Printout
        print("""
        \t \t ******************************    
        \t \t ** Data sources information **
        \t \t ******************************
        """)
        for k, v in rep.items():
            print(k)
            if (v != None):
                for x in v:
                    print("\t \t", x)

    # FEBV1 specific

    def set6BDac(self, dac):
        param = {}
        param["value"] = dac
        return self.processCommand("SET6BDAC", "TDCSERVER", param)

    def cal6BDac(self, mask, shift):
        param = {}
        param["shift"] = shift
        param["mask"] = int(mask, 16)
        return self.processCommand("CAL6BDAC", "TDCSERVER", param)

    def setVthTime(self, Threshold):
        param = {}
        param["value"] = Threshold
        return self.processCommand("SETVTHTIME", "TDCSERVER", param)

    def setTdcMode(self, mode):
        param = {}
        param["value"] = mode
        return self.processCommand("SETMODE", "TDCSERVER", param)

    def setTdcDelays(self, active, dead):
        param = {}
        param["value"] = active
        r = {}
        r["active"] = json.loads(self.processCommand(
            "SETDELAY", "TDCSERVER", param))

    def setTdcMask(self, channelmask, asicmask):
        param = {}
        param["value"] = channelmask
        param["asic"] = asicmask
        return self.processCommand("SETMASK", "TDCSERVER", param)

    def tdcLUTCalib(self, instance, channel):
        if (not "TDCSERVER" in self.appMap):
            return '{"answer":"NOTDCSERVER","status":"FAILED"}'
        if (len(self.appMap["TDCSERVER"]) <= instance):
            return '{"answer":"InvalidInstance","status":"FAILED"}'

        tdc = self.appMap["TDCSERVER"][instance]
        n = (1 << channel)
        param = {}
        param["value"] = "%x" % n
        r = {}
        r["cal_mask"] = json.loads(tdc.sendCommand("CALIBMASK", param))
        r["cal_status"] = json.loads(tdc.sendCommand("CALIBSTATUS", param))
        return json.dumps(r)

    def tdcLUTDump(self, instance, channel):
        if (not "TDCSERVER" in self.appMap):
            return '{"answer":"NOTDCSERVER","status":"FAILED"}'
        if (len(self.appMap["TDCSERVER"]) <= instance):
            return '{"answer":"InvalidInstance","status":"FAILED"}'

        tdc = self.appMap["TDCSERVER"][instance]
        param = {}
        param["value"] = channel
        r = {}
        r["lut_%d" % channel] = json.loads(tdc.sendCommand("GETLUT", param))
        return json.dumps(r)

    def tdcLUTMask(self, instance, mask,feb):
        if (not "TDCSERVER" in self.appMap):
            return '{"answer":"NOTDCSERVER","status":"FAILED"}'
        if (len(self.appMap["TDCSERVER"]) <= instance):
            return '{"answer":"InvalidInstance","status":"FAILED"}'

        tdc = self.appMap["TDCSERVER"][instance]
        param = {}
        param["value"] = mask
        param["feb"] = feb
        r = {}
        r["test_mask"] = json.loads(tdc.sendCommand("TESTMASK", param))
        r["cal_status"] = json.loads(tdc.sendCommand("CALIBSTATUS", param))
        return json.dumps(r)

    def febScurve(self, ntrg, ncon, ncoff, thmin, thmax, step):
        r = {}
        self.mdcc_Pause()
        self.mdcc_setSpillOn(ncon)
        print(" Clock On %d Off %d" % (ncon, ncoff))
        self.mdcc_setSpillOff(ncoff)
        self.mdcc_setSpillRegister(4)
        self.mdcc_CalibOn(1)
        self.mdcc_setCalibCount(ntrg)
        self.mdcc_Status()
        thrange = (thmax - thmin + 1) // step
        for vth in range(0, thrange+1):
            self.mdcc_Pause()
            self.setVthTime(thmax - vth * step)
            time.sleep(0.2)
            self.builder_setHeader(2, thmax - vth * step, 0xFF)

            # Check Last built event
            sr = json.loads(self.BuilderStatus())

            firstEvent = 0
            for k, v in sr.items():
                if (v["event"] > firstEvent):
                    firstEvent = v["event"]
            # print sr,firstEvent
            # Resume Calibration
            self.mdcc_ReloadCalibCount()
            self.mdcc_Resume()
            self.mdcc_Status()

            # Wait for ntrg events capture
            lastEvent = firstEvent
            nloop = 0
            while (lastEvent < (firstEvent + ntrg - 20)):
                sr = json.loads(self.BuilderStatus())
                lastEvent = 0
                for k, v in sr.items():
                    if (v["event"] > lastEvent):
                        lastEvent = v["event"]

                print(" %d First %d  Last %d Step %d" % (thmax-vth*step, firstEvent, lastEvent, step))
                time.sleep(0.2)
                nloop = nloop+1
                if (nloop > 20):
                    break

            r["TH_%d" % (thmax-vth*step)] = lastEvent - firstEvent + 1

            # End Point
            self.mdcc_CalibOn(0)
            self.mdcc_CalibOff()
            self.mdcc_Pause()
        return json.dumps(r)

    def runScurve(self, run, ch, spillon, spilloff, beg, las, step=2, asic=255, Comment="PR2 Calibration", Location="UNKNOWN", nevmax=50):
        oldfirmware = [3, 4, 5, 6, 7, 8, 9, 10, 11,
                    12, 20, 21, 22, 23, 24, 26, 28, 30]
        firmware = [0,2, 4, 6, 7, 8, 10, 12,14,16,18,20,22,24,26,28,30]


        comment = Comment + \
            " Mode %d ON %d Off %d TH min %d max %d Step %d" % (
                ch, spillon, spilloff, beg, las, step)
        self.daq_start(run, location=Location, comment=comment)
        r = {}
        r["run"] = run
        if (ch == 255):
            print("Run Scurve on all channel together")
            mask = 0
            for i in firmware:
                mask = mask | (1 << i)
            self.setTdcMask(mask, asic)
            r["S_%d" % ch] = json.loads(self.febScurve(
                nevmax, spillon, spilloff, beg, las, step))
            self.daq_stop()
            return json.dumps(r)
        if (ch == 1023):
            print("Run Scurve on all channel one by one")
            mask = 0
            for i in firmware:
                print("SCurve for channel %d " % i)
                mask = mask | (1 << i)
                self.setTdcMask(mask, asic)
                r["S_%d" % ch] = json.loads(self.febScurve(
                    nevmax, spillon, spilloff, beg, las, step))
            self.daq_stop()
            return json.dumps(r)
        print("Run Scurve on  channel %d " % ch)
        mask = 0
        mask = mask | (1 << ch)
        self.setTdcMask(mask, asic)
        r["S_%d" % ch] = json.loads(self.febScurve(
            nevmax, spillon, spilloff, beg, las, step))
        self.daq_stop()
        return json.dumps(r)
# DIF Specific
    def setControlRegister(self,ctrlreg):
        param = {}
        param["value"] = int(ctrlreg, 16)
        return self.processCommand("CTRLREG", "DIFMANAGER", param)