from __future__ import absolute_import
from __future__ import print_function
import json
import daqrc
import time


class lydaqControl(daqrc.daqControl):

    def Connect(self):
        self.parseMongo()
        self.discover()
        self.pn="MDCCSERVER"  
        for k, v in self.appMap.items():
            if (k == "MBMDCCSERVER"):
                self.pn="MBMDCCSERVER"
                break
    # Status

    def BuilderStatus(self, verbose=False):
        rep = {}
        for k, v in self.appMap.items():
            if (k != "BUILDER"):
                continue
            for s in v:
                r = {}
                r['run'] = -1
                r['event'] = -1
                r['url'] = s.host
                mr = json.loads(s.sendCommand("STATUS", {}))
                if (mr['status'] != "FAILED"):
                    r["run"] = mr["answer"]["answer"]["run"]
                    r["event"] = mr["answer"]["answer"]["event"]
                    r["builder"] = mr["answer"]["answer"]["difs"]
                    r["built"] = mr["answer"]["answer"]["build"]
                    r["total"] = mr["answer"]["answer"]["total"]
                    r["compressed"] = mr["answer"]["answer"]["compressed"]
                    r["time"] = time.time()
                    rep["%s_%d" % (s.host, s.infos['instance'])] = r
                else:
                    rep["%s_%d" % (s.host, s.infos['instance'])] = mr
        if (not verbose):
            return json.dumps(rep)
        print("""
        \t \t *************************    
        \t \t ** Builder information **
        \t \t *************************
        """)
        #rep = json.loads(sr)
        for k, v in rep.items():
            print(k)
            for xk, xv in v.items():
                    if (xk != "builder"):
                        print("\t", xk, xv)
                    else:
                        if (xv != None):
                            for y in xv:
                                print("\t \t ID %x => %d " % (int(y['id'].split('-')[2]), y['received']))

    def TriggerStatus(self,verbose=False):
        pn="MDCCSERVER"  
        for k, v in self.appMap.items():
            if (k == "MBMDCCSERVER"):
                pn="MBMDCCSERVER"
                break
        mr = json.loads(self.mdcc_Status(ptrgname=pn))
        #print("ON DEBUG ",mr)
        #print("ON DEBUG ",mr)
        if (not verbose):
            return json.dumps(mr["%s_0" % pn]["answer"])
        else:
            print("""
            \t \t *************************    
            \t \t ** Trigger information **
            \t \t *************************
            """)
            for k,v in mr["%s_0" % pn]["answer"]["COUNTERS"].items():
                print("\t \t ",k,v)
    # Builder

    def builder_setHeader(self, rtype, rval, mask):
        l = []
        l.append(rtype)
        l.append(rval)
        l.append(mask)
        param = {}
        param["header"] = l
        return self.processCommand("SETHEADER", "BUILDER", param)
    # MDCC

    def mdcc_Status(self,ptrgname="MDCCSERVER"):
        return self.processCommand("STATUS", self.pn, {})

    def mdcc_Pause(self,ptrgname="MDCCSERVER"):
        return self.processCommand("PAUSE", self.pn, {})

    def mdcc_Resume(self,ptrgname="MDCCSERVER"):
        return self.processCommand("RESUME", self.pn, {})

    def mdcc_EcalPause(self,ptrgname="MDCCSERVER"):
        return self.processCommand("ECALPAUSE", self.pn, {})

    def mdcc_EcalResume(self,ptrgname="MDCCSERVER"):
        return self.processCommand("ECALRESUME", self.pn, {})

    def mdcc_CalibOn(self, value,ptrgname="MDCCSERVER"):
        param = {}
        param["value"] = value
        return self.processCommand("CALIBON", self.pn, param)

    def mdcc_CalibOff(self,ptrgname="MDCCSERVER"):
        return self.processCommand("CALIBOFF", self.pn, {})

    def mdcc_ReloadCalibCount(self,ptrgname="MDCCSERVER"):
        return self.processCommand("RELOADCALIB", self.pn, {})

    def mdcc_setCalibCount(self, value,ptrgname="MDCCSERVER"):
        param = {}
        param["nclock"] = value
        return self.processCommand("SETCALIBCOUNT", self.pn, param)

    def mdcc_Reset(self,ptrgname="MDCCSERVER"):
        return self.processCommand("RESET", self.pn, {})

    def mdcc_ReadRegister(self, address,ptrgname="MDCCSERVER"):
        param = {}
        param["address"] = address
        return self.processCommand("READREG", self.pn, param)

    def mdcc_WriteRegister(self, address, value,ptrgname="MDCCSERVER"):
        param = {}
        param["address"] = address
        param["value"] = value
        return self.processCommand("READREG", self.pn, param)

    def mdcc_setSpillOn(self, value,ptrgname="MDCCSERVER"):
        param = {}
        param["nclock"] = value
        return self.processCommand("SPILLON", self.pn, param)

    def mdcc_setSpillOff(self, value,ptrgname="MDCCSERVER"):
        param = {}
        param["nclock"] = value
        return self.processCommand("SPILLOFF", self.pn, param)

    def mdcc_setResetTdcBit(self, value,ptrgname="MDCCSERVER"):
        param = {}
        param["value"] = value
        return self.processCommand("RESETTDC", self.pn, param)

    def mdcc_resetTdc(self,bar=True,ptrgname="MDCCSERVER"):
        if (bar):
            self.mdcc_setResetTdcBit(0)
            return self.mdcc_setResetTdcBit(0XFFF)
        else:
            self.mdcc_setResetTdcBit(1)
            return self.mdcc_setResetTdcBit(0)
            

    def mdcc_setBeamOn(self, value,ptrgname="MDCCSERVER"):
        param = {}
        param["nclock"] = value
        return self.processCommand("BEAMON", self.pn, param)
    
    def mdcc_setChannelOn(self, value,ptrgname="MDCCSERVER"):
        param = {}
        param["value"] = value
        return self.processCommand("CHANNELON", self.pn, param)

    def mdcc_setHardReset(self, value,ptrgname="MDCCSERVER"):
        param = {}
        param["value"] = value
        return self.processCommand("SETHARDRESET", self.pn, param)

    def mdcc_setSpillRegister(self, value,ptrgname="MDCCSERVER"):
        param = {}
        param["value"] = value
        return self.processCommand("SETSPILLREGISTER", self.pn, param)

    def mdcc_setExternal(self, value,ptrgname="MDCCSERVER"):
        param = {}
        param["value"] = value
        return self.processCommand("SETEXTERNAL", self.pn, param)

    def mdcc_setCalibRegister(self, value,ptrgname="MDCCSERVER"):
        param = {}
        param["value"] = value
        return self.processCommand("SETCALIBREGISTER", self.pn, param)

    def mdcc_setTriggerDelays(self, delay, busy,ptrgname="MDCCSERVER"):
        param = {}
        param["delay"] = delay
        param["busy"] = busy
        return self.processCommand("SETTRIGEXT", self.pn, param)
