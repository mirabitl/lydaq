import daqrc
import time


class lydaqControl(daqrc.daqControl):

    def Connect(self):
        self.parseMongo()
        self.discover()

    # Status

    def BuilderStatus(self):
        rep = {}
        for k, v in self.appMap.items:
            if (k != "BUILDER"):
                continue
            for s in v:
                r = {}
                r['run'] = -1
                r['event'] = -1
                r['url'] = s.host
                mr = json.loads(s.sendCommand("STATUS", None))
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

        return json.dumps(rep)

    def TriggerStatus(self):
        mr = json.loads(self.mdcc_Status())
        return json.dumps(mr["answer"])
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

    def mdcc_Status(self):
        return self.processCommand("STATUS", "MDCCSERVER", None)

    def mdcc_Pause(self):
        return self.processCommand("PAUSE", "MDCCSERVER", None)

    def mdcc_Resume(self):
        return self.processCommand("RESUME", "MDCCSERVER", None)

    def mdcc_EcalPause(self):
        return self.processCommand("ECALPAUSE", "MDCCSERVER", None)

    def mdcc_EcalResume(self):
        return self.processCommand("ECALRESUME", "MDCCSERVER", None)

    def mdcc_CalibOn(self, value):
        param = {}
        param["value"] = value
        return self.processCommand("CALIBON", "MDCCSERVER", param)

    def mdcc_CalibOff(self):
        return self.processCommand("CALIBOFF", "MDCCSERVER", None)

    def mdcc_ReloadCalibCount(self):
        return self.processCommand("RELOADCALIB", "MDCCSERVER", None)

    def mdcc_setCalibCount(self, value):
        param = {}
        param["nclock"] = value
        return self.processCommand("SETCALIBCOUNT", "MDCCSERVER", param)

    def mdcc_Reset(self):
        return self.processCommand("RESET", "MDCCSERVER", None)

    def mdcc_ReadRegister(self, address):
        param = {}
        param["address"] = address
        return self.processCommand("READREG", "MDCCSERVER", param)

    def mdcc_WriteRegister(self, address, value):
        param = {}
        param["address"] = address
        param["value"] = value
        return self.processCommand("READREG", "MDCCSERVER", param)

    def mdcc_setSpillOn(self, value):
        param = {}
        param["nclock"] = value
        return self.processCommand("SPILLON", "MDCCSERVER", param)

    def mdcc_setSpillOff(self, value):
        param = {}
        param["nclock"] = value
        return self.processCommand("SPILLOFF", "MDCCSERVER", param)

    def mdcc_setResetTdcBit(self, value):
        param = {}
        param["value"] = value
        return self.processCommand("RESETTDC", "MDCCSERVER", param)

    def mdcc_resetTdc(self):
        mdcc_setResetTdcBit(0)
        return mdcc_setResetTdcBit(1)

    def mdcc_setBeamOn(self, value):
        param = {}
        param["nclock"] = value
        return self.processCommand("BEAMON", "MDCCSERVER", param)

    def mdcc_setHardReset(self, value):
        param = {}
        param["value"] = value
        return self.processCommand("SETHARDRESET", "MDCCSERVER", param)

    def mdcc_setSpillRegister(self, value):
        param = {}
        param["value"] = value
        return self.processCommand("SETSPILLREGISTER", "MDCCSERVER", param)

    def mdcc_setExternal(self, value):
        param = {}
        param["value"] = value
        return self.processCommand("SETEXTERNAL", "MDCCSERVER", param)

    def mdcc_setCalibRegister(self, value):
        param = {}
        param["value"] = value
        return self.processCommand("SETCALIBREGISTER", "MDCCSERVER", param)

    def mdcc_setTriggerDelays(self, delay, busy):
        param = {}
        param["delay"] = delay
        param["busy"] = busy
        return self.processCommand("SETTRIGEXT", "MDCCSERVER", param)
