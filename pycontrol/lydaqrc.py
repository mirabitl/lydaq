import daqrc

class lydaqControl(daqrc.daqControl):
    
    def mdcc_Status(self):
        return self.processCommand("STATUS","MDCCSERVER",None)

    def mdcc_Pause(self):
        return self.processCommand("PAUSE","MDCCSERVER",None)

    def mdcc_Resume(self):
        return self.processCommand("RESUME","MDCCSERVER",None)

    def mdcc_EcalPause(self):
        return self.processCommand("ECALPAUSE","MDCCSERVER",None)

    def mdcc_EcalResume(self):
        return self.processCommand("ECALRESUME","MDCCSERVER",None)

    def mdcc_CalibOn(self,value):
        param={}
        param["value"]=value
        return self.processCommand("CALIBON","MDCCSERVER",param)

    def mdcc_CalibOff(self):
        return self.processCommand("CALIBOFF","MDCCSERVER",None)

    def mdcc_ReloadCalibCount(self):
        return self.processCommand("RELOADCALIB","MDCCSERVER",None)

    def mdcc_setCalibCount(self,value):
        param={}
        param["nclock"]=value
        return self.processCommand("SETCALIBCOUNT","MDCCSERVER",param)

    def mdcc_Reset(self):
        return self.processCommand("RESET","MDCCSERVER",None)

    def mdcc_ReadRegister(self,address):
        param={}
        param["address"]=address
        return self.processCommand("READREG","MDCCSERVER",param)

    def mdcc_WriteRegister(self,address,value):
        param={}
        param["address"]=address
        param["value"]=value
        return self.processCommand("READREG","MDCCSERVER",param)

    def mdcc_setSpillOn(self,value):
        param={}
        param["nclock"]=value
        return self.processCommand("SPILLON","MDCCSERVER",param)
    
    def mdcc_setSpillOff(self,value):
        param={}
        param["nclock"]=value
        return self.processCommand("SPILLOFF","MDCCSERVER",param)
    
    def mdcc_setResetTdcBit(self,value):
        param={}
        param["value"]=value
        return self.processCommand("RESETTDC","MDCCSERVER",param)

    def mdcc_resetTdc(self):
        mdcc_setResetTdcBit(0)
        return  mdcc_setResetTdcBit(1)
