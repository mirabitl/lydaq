
from ilcconfdb import *
import os
import sqlite3 as sqlite


def IP2Int(ip):
    o = map(int, ip.split('.'))
    res = (16777216 * o[3]) + (65536 * o[2]) + (256 * o[1]) + o[0]
    return res


class TdcAccess:
    """
    Main class to access the DB
    """

    def __init__(self, setupname=None):
        """
        Download of the specified setup setupname or just initilise the DB if None
        """
        DBInit_init()
        if (setupname != None):
            self.setupname = setupname
            self.setup = Setup_getSetup(setupname)
            self.states = self.setup.getStates()
            self.asicConf = self.states[0].getAsicConfiguration()
            self.asics = self.asicConf.getVector()
            self.difConf = self.states[0].getDifConfiguration()
            self.difs = self.difConf.getVector()
            self.ldaConf = self.states[0].getLdaConfiguration()
            self.dccConf = self.states[0].getDccConfiguration()

        self.diflist = []
        self.asiclist = []
        self.ldas = []
        self.dccs = []

    def ChangeLatch(self, Latch, idif=0, iasic=0):
        """
        Change the Latch mode of the asic #asic on the TDCDIF #dif
        If 0 all hardware is changed
        """
        for a in self.asics:
            if (idif != 0 and a.getInt("DIF_ID") != idif):
                continue
            if (iasic != 0 and a.getInt("HEADER") != iasic):
                continue
            try:
                a.setInt("LATCH", Latch)
                a.setModified(1)
            except Exception, e:
                print e.getMessage()

    def ChangeVthTime(self, VthTime, idif=0, iasic=0):
        """
        Change the VTHTIME  of the asic #asic on the TDCDIF #dif
        If 0 all hardware is changed
        """
        for a in self.asics:
            if (idif != 0 and a.getInt("DIF_ID") != idif):
                continue
            if (iasic != 0 and a.getInt("HEADER") != iasic):
                continue
            try:
                a.setInt("VTHTIME", VthTime)
                a.setModified(1)
            except Exception, e:
                print e.getMessage()

    def ChangePAC(self,A0,A1,A2,A3, idif=0, iasic=0):
        """
        Change the VTHTIME  of the asic #asic on the TDCDIF #dif
        If 0 all hardware is changed
        """
        for a in self.asics:
            if (idif != 0 and a.getInt("DIF_ID") != idif):
                continue
            if (iasic != 0 and a.getInt("HEADER") != iasic):
                continue
            try:
                a.setInt("PA_CCOMP_0",A0)
                a.setInt("PA_CCOMP_1",A1)
                a.setInt("PA_CCOMP_2",A2)
                a.setInt("PA_CCOMP_3",A3)
                a.setModified(1)
            except Exception, e:
                print e.getMessage()

    def ChangeDacDelay(self, delay, idif=0, iasic=0):
        """
        Change the DACDELAY  of the asic #asic on the TDCDIF #dif
        If 0 all hardware is changed
        """
        for a in self.asics:
            if (idif != 0 and a.getInt("DIF_ID") != idif):
                continue
            if (iasic != 0 and a.getInt("HEADER") != iasic):
                continue
            try:
                a.setInt("DACDELAY", delay)
                a.setModified(1)
            except Exception, e:
                print e.getMessage()

    def ChangeAllEnabled(self, idif=0, iasic=0):
        """
        Change all the ENable signals  of the asic #asic on the TDCDIF #dif
        If 0 all hardware is changed
        """
        for a in self.asics:
            if (idif != 0 and a.getInt("DIF_ID") != idif):
                continue
            if (iasic != 0 and a.getInt("HEADER") != iasic):
                continue
            try:

                a.setInt('EN_BIAS_DISCRI', 1)
                a.setInt('EN_BIAS_PA', 1)
                a.setInt('EN_BIAS_DISCRI_CHARGE', 1)
                a.setInt('EN_DOUT_OC', 1)
                a.setInt('EN_BIAS_DAC_DELAY', 1)
                a.setInt('EN10BDAC', 1)
                a.setInt('EN_BIAS_DISCRI_ADC_CHARGE', 1)
                a.setInt('EN_BIAS_SCA', 1)
                a.setInt('EN_BIAS_6BIT_DAC', 1)
                a.setInt('EN_TRANSMIT', 1)
                a.setInt('EN_BIAS_RAMP_DELAY', 1)
                a.setInt('EN_BIAS_CHARGE', 1)
                a.setInt('EN_FAST_LVDS_REC', 1)
                a.setInt('EN_TRANSMITTER', 1)
                a.setInt('EN_ADC', 1)
                a.setInt('EN_NOR32_CHARGE', 1)
                a.setInt('EN_80M', 1)
                a.setInt('EN_DISCRI_DELAY', 1)
                a.setInt('EN_BIAS_DISCRI_ADC_TIME', 1)
                a.setInt('EN_NOR32_TIME', 1)
                a.setInt('EN_TEMP_SENSOR', 1)

                a.setModified(1)
            except Exception, e:
                print e.getMessage()
    def ChangeInputDac(self, idif, iasic, ich, dac):
        """
        Change the InputDAC value to dac  of the asic #asic on the TDCDIF #dif       
        """

        for a in self.asics:
            if (a.getInt("DIF_ID") != idif):
                continue
            if (a.getInt("HEADER") != iasic):
                continue

            vg = a.getIntVector("INPUTDAC")
            vg[ich] = dac
            print " Input Dac changed", idif, iasic, ich, dac
            try:
                a.setIntVector("INPUTDAC", vg)
            except Exception, e:
                print e.getMessage()
            a.setModified(1)


    def Change6BDac(self, idif, iasic, ich, dac):
        """
        Change the 6BDAC value to dac  of the asic #asic on the TDCDIF #dif       
        """

        for a in self.asics:
            if (a.getInt("DIF_ID") != idif):
                continue
            if (a.getInt("HEADER") != iasic):
                continue

            vg = a.getIntVector("DAC6B")
            vg[ich] = dac
            print " Dac changed", idif, iasic, ich, dac
            try:
                a.setIntVector("DAC6B", vg)
            except Exception, e:
                print e.getMessage()
            a.setModified(1)

    def Correct6BDac(self, idif, iasic, cor):
        """
        Change the 6BDAC value   of the asic #asic on the TDCDIF #dif
        cor is an array of 32 value , 
        6BDAC[i]=6BDAC[i]+cor[i]
        """

        for a in self.asics:
            if (a.getInt("DIF_ID") != idif):
                continue
            if (a.getInt("HEADER") != iasic):
                continue

            vg = a.getIntVector("DAC6B")
            for ich in range(32):
                print " Dac changed", idif, iasic, ich, vg[ich], cor[ich]
                vg[ich] = vg[ich]+cor[ich]

            try:
                a.setIntVector("DAC6B", vg)
            except Exception, e:
                print e.getMessage()
            a.setModified(1)

    def ChangeMask(self, idif, iasic, ich, mask):
        """
        Change PETIROC2 MASKDISCRITIME parameter for one channel
        Careful: 1 = inactive, 0=active
        """

        for a in self.asics:
            if (a.getInt("DIF_ID") != idif):
                continue
            if (a.getInt("HEADER") != iasic):
                continue

            vg = a.getIntVector("MASKDISCRITIME")
            vg[ich] = mask
            print " Dac changed", idif, iasic, ich, mask
            try:
                a.setIntVector("MASKDISCRITIME", vg)
            except Exception, e:
                print e.getMessage()
            a.setModified(1)

    def SetEnabled(self, idif=0, iasic=0, status=1):
        """
        Change ENABLED status of the ASIC tagged by idif and iasic to the value status
         if idif is 0 all difs are concerned
         if iasic is 0 all asics are concerned
         status is 1 by default
        """
        for a in self.asics:
            if (idif != 0 and a.getInt("DIF_ID") != idif):
                continue
            if (iasic != 0 and a.getInt("HEADER") != iasic):
                continue
            try:
                a.setInt("ENABLED", status)
                a.setModified(1)

            except Exception, e:
                print e.getMessage()
            print a.getInt('DIF_ID'), a.getInt('HEADER'), a.getInt("ENABLED")

    def SetSlowControlMask(self, idif, iasic=0):
        """
        Mask Slow control for one DIF idif and 1 or all asics (iasic=0)
        The Masked DIF or Asic are not used for slow control
        """
        if (iasic == 0):
            for a in self.difs:
                if (a.getInt("ID") != idif):
                    continue
                try:
                    a.setInt("MASK", 1)
                    a.setModified(1)
                except Exception, e:
                    print e.getMessage()
                    print a.getInt('MASK')
        else:
           for a in self.asics:
               if (idif != 0 and a.getInt("DIF_ID") != idif):
                   continue
               if (iasic != 0 and a.getInt("HEADER") != iasic):
                   continue
               try:
                a.setInt("ENABLED", 0)
                a.setModified(1)

               except Exception, e:
                   print e.getMessage()
               print a.getInt('DIF_ID'), a.getInt('HEADER'), a.getInt("ENABLED")

    def setLDACabling(self, dif_id, ether_address, lda_channel, dcc_channel):
      """
      (Obsolete) Keep for coherency ,
      Set the LDA cabling of the DIF dif_id
      for a in self.asics:
      """
      for a in self.difs:
          if (a.getInt("ID") != dif_id):
              continue
          try:
              a.setString("LDA_ADDRESS", ether_address)
              a.setInt("LDA_CHANNEL", lda_channel)
              a.setInt("DCC_CHANNEL", dcc_channel)
              a.setModified(1)

          except Exception, e:
              print e.getMessage()
          print a.getInt('ID')

    def uploadChanges(self):
        """
        Upload a new version
        The state name will be, old_state_name_xx where xx is the new index (starting from 00)
        """
        self.asicConf.setModified(1)
        self.difConf.setModified(1)
        self.dccConf.setModified(0)
        try:
            self.setup.recursiveUpload()
        except Exception, e:
            print e.getMessage()

    def getDIFList(self):
        """
        return the list of DIF in the current setup
        """
        dl = []
        for x in self.difConf.getVector():
            dl.append(x.getInt("ID"))
        return dl

    def dumpStateNames(self):
        """
        print the list of states in the DB
        """
        for x in Setup_getSetupNames():
            print x

    def toXML(self):
        """
        Save the current setup to setup_name.xml 
        """
        fname = self.setupname+".xml"
        self.states[0].saveToXML(fname)

    def addLDA(self, lda_address):
        """
        (Obsolete) Keep for coherency ,Add an LDA
        """
        print "force LDA"
        theLDA = Lda()
        theLDA.setString("MAC_ADRESS", lda_address)
        self.ldaConf.add(theLDA)
        self.ldas.append(theLDA)
        print "LDA added ", lda_address
        #print "in DCCCONF: ",l.getString("LDA_ADDRESS"),l.getInt("LDA_CHANNEL"),lda_channel

    def addDCC(self, lda_address, lda_channel):
        """
         (Obsolete) Keep for coherency ,add a DCCC
        """
        print "force DCC"
        theDcc = Dcc()
        theDcc.setString("LDA_ADDRESS", lda_address)
        theDcc.setInt("LDA_CHANNEL", lda_channel)
        self.dccConf.add(theDcc)
        self.dccs.append(theDcc)
        print "\t DCC added", theDcc.getString("LDA_ADDRESS"), theDcc.getInt("LDA_CHANNEL")

    def addAsic(self, dif_num, header,version="A"):
        """
        Add a new PETIROC2 
        dif_num = DIF ID (ipaddr in integer >>16)
        header= ASIC number
        """
        print "force ASIC"

        thePR2 = self.initPR2(dif_num, header,version)
        self.asiclist.append(thePR2)
        self.asicConf.add(thePR2)

    def addDIF(self, ipaddr, nb_asic, version="A",lda_address="ff:ff:ff:ff:ff:ff", lda_channel=0, dcc_channel=0, xmlfile=None):
        """
        Add a new DIF and load asics conf from file if any
        dif_num = DIF ID = LSB of the ipaddr in integer
        nb_asic = Number of ASICs
        lda_address = LDA MAC address (ff:ff:ff:ff:ff:ff)
        lda_channel = LDA Channel (0)
        dcc_channel = DCC Channel (0)
        xmlfile = XML list of ASICs (None)
        """
        print "force DIF"
        theDif = self.initDif(ipaddr)
        dif_num = theDif.getInt("ID")
        print "DIF number %d" % (dif_num)
        if (lda_channel != 0):
            theDif.setString("LDA_ADDRESS", lda_address)
            theDif.setInt("LDA_CHANNEL", lda_channel)
            theDif.setInt("DCC_CHANNEL", dcc_channel)

        self.difConf.add(theDif)
        self.diflist.append(theDif)
        if (xmlfile != None):
            self.asicConf.addFromXML(xmlfile)
        else:
            for ih in range(nb_asic):
                header = ih+1

                thePR2 = self.initPR2(dif_num, header,version)
                self.asiclist.append(thePR2)
                self.asicConf.add(thePR2)

    def initDif(self, addr):
        """
        Default TDCDIF initialisation
        """
        d = Dif("TDCDIF")
        d.setString('NAME', "TDC%d" % ((IP2Int(addr) >> 16) & 0xFFFF))
        d.setString('IP_ADDRESS', addr)
        d.setInt('TYPE', 2)
        #d.setString('LDA_ADDRESS',lda_address)
        #d.setInt('LDA_CHANNEL',lda_chan)
        #d.setInt('DCC_CHANNEL',dcc_chan)
        d.setInt('ID', (IP2Int(addr) >> 16) & 0xFFFF)
        print "creating DIF %d %x" % ((IP2Int(addr) >> 16) & 0xFFFF, IP2Int(addr))
        d.thisown = 0
        return d

    def initialiseState(self, name):
        """
        Create a new state named 'name'
        """

        self.setupname = name
        self.asicConf = AsicConfiguration()
        self.difConf = DifConfiguration()
        self.ldaConf = LdaConfiguration()
        self.dccConf = DccConfiguration()
        self.state = State(name)
        self.setup = Setup(name)
        self.asics = self.asicConf.getVector()
        self.difs = self.difConf.getVector()

    def uploadFromFile(self, setupname, fname):
        """
        Upload a new setup from an XML file
        setupname = new setup name
        fname = XML file
        """
        try:
            print "Setup"
            self.setup = Setup(setupname)
        except Exception, e:
            print " Setup creation Problem : ", e.getMessage()
        try:
            print "State"
            self.state = State_createStateFromXML(setupname+"_"+fname, fname)
        except Exception, e:
            print " State creation Problem : ", e.getMessage()
        try:
            print "Addstate"
            self.setup.addState(self.state)
        except Exception, e:
            print " add State  Problem : ", e.getMessage()
        try:
            print "Upload"
            self.setup.upload()
        except Exception, e:
            print " upload Problem : ", e.getMessage()

    def uploadNewState(self, name):
        """
        upload newly created state in oracle with state named 'name'
        """
        if (len(self.ldas) > 0):
            try:
                lda_vers = self.ldaConf.upload(name)
            except Exception, e:
                print "LDA upload Problem : ", e.getMessage()
                return
            print "et c est bon pour les LDA"

            try:
                dcc_vers = self.dccConf.upload(name)
            except Exception, e:
                print "DCC upload Problem : ", e.getMessage()
                return
            print "et c est bon pour les DCC"

        try:
            dif_vers = self.difConf.upload(name)
        except Exception, e:
            print "DIF upload Problem : ", e.getMessage()
            return
        print "et c est bon pour les DIF"
        try:
            asic_vers = self.asicConf.upload(name)
        except Exception, e:
            print "Asic upload Problem : ", e.getMessage()
            return
        print "et c est bon pour les ASIC"

        self.state.setAsicVersion(asic_vers)
        self.state.setDifVersion(dif_vers)
        v0 = Version(0, 0)
        try:
            if (len(self.ldas) > 0):
                self.state.setLdaVersion(lda_vers)
                self.state.setDccVersion(dcc_vers)
            else:
                self.state.setLdaVersion(v0)
                self.state.setDccVersion(v0)

            self.state.upload()
        except Exception, e:
            print "State upload Problem : ", e.getMessage()
            return
        print "et c est bon pour le STATE"

        self.setup.addState(self.state)
        try:
            self.setup.upload()
        except Exception, e:
            print "Setup upload Problem : ", e.getMessage()
            return
        print "et c est bon pour le SETUP"
        #DBInit_terminate();

    def setLDACabling(self, dif_id, ether_address, lda_channel, dcc_channel):
      """
      Set the LDA cabling of the DIF dif_id
      for a in self.asics:
      """
      for a in self.difs:
          if (a.getInt("ID") != dif_id):
              continue
          try:
              a.setString("LDA_ADDRESS", ether_address)
              a.setInt("LDA_CHANNEL", lda_channel)
              a.setInt("DCC_CHANNEL", dcc_channel)
              a.setModified(1)

          except Exception, e:
              print e.getMessage()
          print a.getInt('ID')

    def DumpDIFs(self):
        """
        return XML string of the list of DIFs
        """
        s = ""
        for x in self.difConf.getVector():
            s = s+x.toXML()+"\n"
        return s

    def DumpAsics(self):
        """
        Print list of ASicx
        """
        for x in self.asicConf.getVector():
            print x.getInt('DIF_ID'), x.getInt('HEADER')

    def initPR2(self, dif, num,version="A"):
        """
        PETIROC 2  initialisation
        """
	#print "***** init HR2"
        asi=None
        if (version=="A"):
            asi = Asic('PR2', dif, num)
        else:
            asi = Asic('PR2B', dif, num)
            asi.setInt("PA_CCOMP_0",0)
            asi.setInt("PA_CCOMP_1",0)
            asi.setInt("PA_CCOMP_2",0)
            asi.setInt("PA_CCOMP_3",0)
            asi.setInt("CHOICE_TRIGGER_OUT",0)
        asi.setInt('DIF_ID', dif)
        asi.setInt('HEADER', num)
        asi.setInt('EN_BIAS_DISCRI', 1)
        asi.setInt('CF2_200FF', 0)
        asi.setInt('CF3_100FF', 0)
        asi.setInt('DACDELAY', 31)
        asi.setInt('PP_BIAS_RAMP_DELAY', 0)
        asi.setInt('ON_OFF_2MA', 1)
        asi.setInt('EN_BIAS_PA', 1)
        asi.setInt('PP_BIAS_6BIT_DAC', 0)
        asi.setInt('EN_BIAS_DISCRI_CHARGE', 0)
        asi.setInt('EN_DOUT_OC', 0)
        asi.setInt('ON_OFF_OTAQ', 0)
        asi.setInt('DIS_RAZCHN_EXT', 0)
        asi.setInt('EN_BIAS_DAC_DELAY', 0)
        asi.setInt('PP_SLOW_LVDS_REC', 0)
        asi.setInt('EN10BDAC', 1)
        asi.setInt('PP_BIAS_SCA', 0)
        asi.setInt('USEBCOMPENSATION', 0)
        asi.setInt('VTHDISCRICHARGE', 863)
        asi.setInt('EN_BIAS_DISCRI_ADC_CHARGE', 0)
        asi.setInt('PP_BIAS_DISCRI_ADC_CHARGE', 0)
        asi.setInt('EN_SLOW_LVDS_REC', 1)
        asi.setInt('SEL_STARB_RAMP_ADC_EXT', 0)
        asi.setInt('DIS_TRIGGERS', 0)
        asi.setInt('ON_OFF_INPUT_DAC', 1)
        asi.setInt('EN_BIAS_SCA', 0)
        asi.setInt('EN_BIAS_6BIT_DAC', 1)
        asi.setInt('PP_TRANSMITTER', 0)
        asi.setInt('PP_TEMP_SENSOR', 0)
        asi.setInt('EN_TRANSMIT', 0)
        asi.setInt('PP_BIAS_CHARGE', 0)
        asi.setInt('PP_BIAS_DISCRI_ADC_TIME', 0)
        asi.setInt('ON_OFF_OTA_MUX', 0)
        asi.setInt('CMD_POLARITY', 0)
        asi.setInt('PP_BIAS_DAC_DELAY', 0)
        asi.setInt('CF1_2P5PF', 0)
        asi.setInt('PP_FAST_LVDS_REC', 0)
        asi.setInt('EN_BIAS_RAMP_DELAY', 0)
        asi.setInt('PP10BDAC', 0)
        asi.setInt('EN_BIAS_CHARGE', 0)
        asi.setInt('EN_FAST_LVDS_REC', 1)
        asi.setInt('PP_BIAS_TDC', 0)
        asi.setInt('CF0_1P25PF', 0)
        asi.setInt('EN_TRANSMITTER', 0)
        asi.setInt('EN_ADC', 0)
        asi.setInt('EN_NOR32_CHARGE', 0)
        asi.setInt('EN_80M', 0)
        asi.setInt('DIS_TRIG_MUX', 1)
        asi.setInt('PP_BIAS_DISCRI_CHARGE', 0)
        asi.setInt('ON_OFF_1MA', 1)
        asi.setInt('ON_OFF_OTA_PROBE', 0)
        asi.setInt('PP_DISCRI_DELAY', 0)
        asi.setInt('EN_DISCRI_DELAY', 0)
        asi.setInt('LATCH', 1)
        asi.setInt('EN_BIAS_DISCRI_ADC_TIME', 0)
        asi.setInt('EN_NOR32_TIME', 1)
        asi.setInt('EN_TEMP_SENSOR', 0)
        asi.setInt('PP_BIAS_PA', 0)
        asi.setInt('PP_ADC', 0)
        asi.setInt('PP_BIAS_DISCRI', 0)
        asi.setInt('VTHTIME', 610)
        asi.setInt('SEL_80M', 0)
        asi.setInt('EN_BIAS_TDC', 0)
        asi.setInt('DIS_RAZCHN_INT', 1)
        d6b = asi.getIntVector('DAC6B')
        for x in range(32):
            d6b.append(31)
        asi.setIntVector('DAC6B', d6b)
        didac = asi.getIntVector('INPUTDAC')
        for x in range(32):
            didac.append(125)
        asi.setIntVector('INPUTDAC', didac)
        didacc = asi.getIntVector('INPUTDACCOMMAND')
        for x in range(32):
            didacc.append(1)
        asi.setIntVector('INPUTDACCOMMAND', didacc)
        dmdc = asi.getIntVector('MASKDISCRICHARGE')
        for x in range(32):
            dmdc.append(1)
        asi.setIntVector('MASKDISCRICHARGE', dmdc)
        dmdt = asi.getIntVector('MASKDISCRITIME')
        for x in range(32):
            dmdt.append(0)
        asi.setIntVector('MASKDISCRITIME', dmdt)
        asi.thisown = 0

        return asi

####

    def end(self):
        """
        close the DB
        """
        try:
            del self.setup

            DBInit_terminate()
            print "termninate called"
        except Exception, e:
            print "initOracle:: : ", e.getMessage()
            return

    def dumpStateNames(self):
        """
        print the list of states in the DB
        """
        try:
            for x in Setup_getSetupNames():
                print x
        except Exception, e:
            print "initOracle::DumpState : ", e.getMessage()
            return

 
