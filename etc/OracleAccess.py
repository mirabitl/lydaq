

from ilcconfdb import *
import RocDescription as rd
import os
import sqlite3 as sqlite
class OracleAccess:
    """
    Modify or/and create an Oracle DB state
    """
    def __init__(self,setupname=None):
        """
        Download of the specified setup setupname or just initilise the DB if None
        """
        DBInit_init();
        if (setupname!=None):
            self.setupname=setupname;
            self.setup=Setup_getSetup(setupname);
            self.states=self.setup.getStates();
            self.asicConf=self.states[0].getAsicConfiguration();
            self.asics=self.asicConf.getVector();
            self.difConf=self.states[0].getDifConfiguration();
            self.difs=self.difConf.getVector()
            self.ldaConf=self.states[0].getLdaConfiguration();
            self.dccConf=self.states[0].getDccConfiguration();

        self.diflist=[]
        self.asiclist=[]
        self.ldas=[]
        self.dccs=[]

    def unsetPowerPulsing(self):
        """
        Unset Power pulsing on all ASICs
        """
        for a in self.asics: 
            print a.getInt('DISCRI0')
            a.setInt('CLKMUX',1)
            a.setInt('SCON',1)
            a.setInt('OTAQ_PWRADC',1)
            a.setInt('PWRONW',1)
            a.setInt('PWRONSS',1)
            a.setInt('PWRONBUFF',1)
            a.setInt('PWRONPA',1)
            a.setInt('DISCRI0',1)
            a.setInt('DISCRI1',1)
            a.setInt('DISCRI2',1)
            a.setInt('OTABGSW',1)
            a.setInt('DACSW',1)
            a.setInt('PWRONFSB0',1)
            a.setInt('PWRONFSB1',1)
            a.setInt('PWRONFSB2',1)
            print a.getInt('DISCRI0')
            a.setModified(1)

    def setPowerPulsing(self):
        """
        set Power pulsing on all ASICs
        """
        for a in self.asics: 
            print a.getInt('DISCRI0')
            a.setInt('CLKMUX',0)
            a.setInt('SCON',0)
            a.setInt('OTAQ_PWRADC',0)
            a.setInt('PWRONW',0)
            a.setInt('PWRONSS',0)
            a.setInt('PWRONBUFF',0)
            a.setInt('PWRONPA',0)
            a.setInt('DISCRI0',0)
            a.setInt('DISCRI1',1)
            a.setInt('DISCRI2',1)
            a.setInt('OTABGSW',0)
            a.setInt('DACSW',0)
            a.setInt('PWRONFSB0',0)
            a.setInt('PWRONFSB1',1)
            a.setInt('PWRONFSB2',1)
            print a.getInt('DISCRI0')
            a.setModified(1)
    def ChangeDIFId(self,idifold,idifnew):
        """
        change DIF_ID from idifold to idifnew
        """
        for a in self.asics:
            if (a.getInt("DIF_ID") != idifold ):
                continue;
            try:
                a.setInt("DIF_ID",idifnew);
                a.setModified(1)
            except Exception, e:
                print e.getMessage()


    def ChangeThreshold(self,B0,B1,B2,idif=0,iasic=0):
        """
        change thresholds B0,B1,B2 on DIF idif and asic iasic.
        If not specified all Asics of a given DIF is changed
        if idif is not specified all Asics of all Difs are changed
        """
        for a in self.asics:
            if (idif !=0 and a.getInt("DIF_ID") != idif ):
                continue;
            if (iasic !=0 and a.getInt("HEADER") != iasic):
                continue;
            try:
                a.setInt("B0",B0);
                a.setInt("B1",B1);
                a.setInt("B2",B2);
                a.setModified(1)
            except Exception, e:
                print e.getMessage()

    def ChangeMRThreshold(self,B0,B1,B2,idif=0,iasic=0):
        """
        change thresholds BB0,BB1,BB2 on DIF idif and asic iasic.
        If not specified all Asics of a given DIF is changed
        if idif is not specified all Asics of all Difs are changed
        """
        for a in self.asics:
            if (idif !=0 and a.getInt("DIF_ID") != idif ):
                continue;
            if (iasic !=0 and a.getInt("HEADER") != iasic):
                continue;
            try:
                a.setInt("BB0",B0);
                a.setInt("BB1",B1);
                a.setInt("BB2",B2);
                a.setModified(1)
            except Exception, e:
                print e.getMessage()

    def ChangeGain(self,idif,iasic,ipad,scale):
        """
        Modify gain of all asics by a factor gain1/gain0 on HR2
        If not specified all Asics of a given DIF is changed
        if idif is not specified all Asics of all Difs are changed
        """

        for a in self.asics:
            if (a.getInt("DIF_ID") != idif ):
                continue;
            if (a.getInt("HEADER") != iasic):
                continue;

            vg=a.getIntVector("PAGAIN")
            vnew =int(scale*vg[ipad])
            if (vnew==0):
                vnew=1
            vg[ipad]=vnew
            print " Rescale factor",idif,iasic,scale,vnew
            try:
                a.setIntVector("PAGAIN",vg)
            except Exception, e:
                print e.getMessage()
            a.setModified(1)


    def SetGain(self,idif,iasic,ipad,vnew):
        """
        Modify gain of all asics by a factor gain1/gain0 on HR2
        If not specified all Asics of a given DIF is changed
        if idif is not specified all Asics of all Difs are changed
        """

        for a in self.asics:
            if (a.getInt("DIF_ID") != idif ):
                continue;
            if (a.getInt("HEADER") != iasic):
                continue;

            vg=a.getIntVector("PAGAIN")
            if (vnew==0):
                vnew=1
            vg[ipad]=vnew

            try:
                a.setIntVector("PAGAIN",vg)
            except Exception, e:
                print e.getMessage()
            a.setModified(1)

    def SetAsicGain(self,idif,iasic,vnew):
        """
        Modify gain of all asics by a factor gain1/gain0 on HR2
        If not specified all Asics of a given DIF is changed
        if idif is not specified all Asics of all Difs are changed
        """

        for a in self.asics:
            if (a.getInt("DIF_ID") != idif and idif!=0 ):
                continue;
            if (a.getInt("HEADER") != iasic and iasic!=0 ):
                continue;

            vg=a.getIntVector("PAGAIN")
            if (vnew==0):
                vnew=1
            for ipad in range(0,64):
                vg[ipad]=vnew

            try:
                a.setIntVector("PAGAIN",vg)
            except Exception, e:
                print e.getMessage()
            a.setModified(1)


    def RescaleGain(self,gain0,gain1,idif=0,iasic=0):
        """
        Modify gain of all asics by a factor gain1/gain0 on HR2
        If not specified all Asics of a given DIF is changed
        if idif is not specified all Asics of all Difs are changed
        """
        scale=gain1*1./gain0
        print " Rescale factor",scale
        for a in self.asics:
            if (idif !=0 and a.getInt("DIF_ID") != idif ):
                continue;
            if (iasic !=0 and a.getInt("HEADER") != iasic):
                continue;

            vg=a.getIntVector("PAGAIN")
            for i in range(64):
                vnew=int(scale*vg[i])
                print i,vg[i],vnew
                vg[i]=vnew
            try:
                a.setIntVector("PAGAIN",vg)
            except Exception, e:
                print e.getMessage()
            a.setModified(1)

    def SetEnabled(self,idif=0,iasic=0,status=1):
        """
        Change ENABLED status of the ASIC tagged by idif and iasic to the value status
         if idif is 0 all difs are concerned
         if iasic is 0 all asics are concerned
         status is 1 by default
        """
        for a in self.asics:
            if (idif!=0 and a.getInt("DIF_ID") != idif):
                continue;
            if (iasic!=0 and a.getInt("HEADER") != iasic):
                continue; 
            try:
                a.setInt("ENABLED",status)
                a.setModified(1)

            except Exception, e:
                print e.getMessage()
            print a.getInt('DIF_ID'),a.getInt('HEADER'),a.getInt("ENABLED")

    def SetSlowControlMask(self,idif,iasic=0):
        """
        Mask Slow control for one DIF idif and 1 or all asics (iasic=0)
        The Masked DIF or Asic are not used for slow control
        """
        if (iasic==0):
            for a in self.difs:
                if (a.getInt("ID") != idif):
                    continue;
                try:
                    a.setInt("MASK",1)
                    a.setModified(1)
                except Exception, e:
                    print e.getMessage()
                    print a.getInt('MASK')
        else:
           for a in self.asics:
               if (idif!=0 and a.getInt("DIF_ID") != idif):
                   continue;
               if (iasic!=0 and a.getInt("HEADER") != iasic):
                   continue; 
               try:
                a.setInt("ENABLED",0)
                a.setModified(1)

               except Exception, e:
                   print e.getMessage()
               print a.getInt('DIF_ID'),a.getInt('HEADER'),a.getInt("ENABLED")

    def SetMask(self,list,idif=0,iasic=0):
        m=0xFFFFFFFFFFFFFFFF
        for i in list:
            m=m &~(1<<i);
        sm="0x%lx" % m
        self.ChangeMask(sm,sm,sm,idif,iasic)
        
    def ChangeMask(self,M0,M1,M2,idif=0,iasic=0):
        """
        Set the mask 0,1,2 to M0,M1,M2 on DIF idif and ASIC iasic
        if idif is 0 all difs are concerned
        if iasic is 0 all asics are concerned
        """
        print M0,M1,M2,idif,iasic
        for a in self.asics:
            if (idif!=0 and a.getInt("DIF_ID") != idif):
                continue;
            if (iasic!=0 and a.getInt("HEADER") != iasic):
                 continue;
            print a.getInt('DIF_ID'),a.getInt('HEADER'),a.getString("MASK0")
            im0i=int(a.getString("MASK0"),16)
            im0n=int(M0,16)
            im1i=int(a.getString("MASK1"),16)
            im1n=int(M1,16)
            im2i=int(a.getString("MASK2"),16)
            im2n=int(M2,16)

            try:
                a.setString("MASK0",'0x%x' % (im0i&im0n));
                a.setString("MASK1",'0x%x' % (im1i&im1n));
                a.setString("MASK2",'0x%x' % (im2i&im2n));
                a.setModified(1)

            except Exception, e:
                print e.getMessage()
            print a.getInt('DIF_ID'),a.getInt('HEADER'),a.getString("MASK0")

    def setLDACabling(self,dif_id,ether_address,lda_channel,dcc_channel):
      """
      Set the LDA cabling of the DIF dif_id
      for a in self.asics:
      """
      for a in self.difs:
          if (a.getInt("ID") != dif_id):
              continue;
          try:
              a.setString("LDA_ADDRESS",ether_address);
              a.setInt("LDA_CHANNEL",lda_channel);
              a.setInt("DCC_CHANNEL",dcc_channel);
              a.setModified(1)

          except Exception, e:
              print e.getMessage()
          print a.getInt('ID')
      
    def ChangeMaskFromFile(self,fname):
        """
        Read the file fname conatining lines like:
        104;4;'0xfffffdffffffff7f'
        104;26;'0xffff7fffffffffff'
        104;32;'0xfffffffffeffffff'
        104;42;'0xffefbffeffffffff'
        104;45;'0xffffffbffbffffff'

        with DIFID, ASIC id and mask and set it 
        """
        flist=open(fname)
        s=flist.readlines()
        flist.close()
        for x in s:
            tokens=x.split(";")
            idif =int(tokens[0]);iasic = int(tokens[1]);mask=tokens[2]
            m1=mask.replace("\n","")
            m=m1.replace("'","")

            self.ChangeMask(m,m,m,idif,iasic)
        return

    def ChangeGainFromFile(self,fname,ratio):
        """
        Read the file fname conatining lines like:
        104;4;'0xfffffdffffffff7f'
        104;26;'0xffff7fffffffffff'
        104;32;'0xfffffffffeffffff'
        104;42;'0xffefbffeffffffff'
        104;45;'0xffffffbffbffffff'

        with DIFID, ASIC id and mask and set it 
        """
        flist=open(fname)
        s=flist.readlines()
        flist.close()
        for x in s:
            tokens=x.split(";")
            idif =int(tokens[0]);iasic = int(tokens[1]);mask=tokens[2]
            m1=mask.replace("\n","")
            ipad=int(m1)

            self.ChangeGain(idif,iasic,ipad,ratio)
        return

    def uploadChanges(self):
        """
        Upload a new version
        The state name will be, old_state_name_xx where xx is the new index (starting from 00)
        """
        self.asicConf.setModified(1);
        self.difConf.setModified(1);
        self.dccConf.setModified(0)
        try:
            self.setup.recursiveUpload();
        except Exception, e:
            print e.getMessage()
 
        
 
    def getDIFList(self):
        """
        return the list of DIF in the current setup
        """
        dl=[]
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

    def toSQLITE(self):
        """
         Export the current setup to an sqlite file with name = setupname.sqlite
        """
        self.dbname=self.setupname+".sqlite"
        os.system("rm -f %s" % self.dbname)
        os.system("cat /data/online/DBUtils/data/tab_create.sqli | sqlite3 %s" % self.dbname)
        self.condb =sqlite.connect(self.dbname)
        self.sq_setup = rd.SETUPDef()
        self.sq_setup.NAME=self.setupname
        self.sq_setup_id=self.sq_setup.insert(self.condb)
        # Loop on DIF and insert them
        for x in self.difConf.getVector():
            dif = rd.DIFDef()
            dif.NUM=x.getInt("ID")
            dif.NAME =x.getString("NAME")
            dif.SETUP_ID=self.sq_setup_id
            dif.DCC_CHANNEL=x.getInt("DCC_CHANNEL");
            dif.LDA_ADDRESS=x.getString("LDA_ADDRESS");
            dif.LDA_CHANNEL=x.getInt("LDA_CHANNEL");
            dif.MASK=x.getInt("MASK");
            dif.NUM=x.getInt("ID");
            dif.MonitorSequencer=x.getInt("MONITOR_SEQUENCER");
            dif.NumericalReadoutMode=x.getInt("NUMERICAL_READOUT_MODE");
            dif.NumericalReadoutStartMode=x.getInt("NUMERICAL_READOUT_START_MODE");
            dif.AVDDShdn=x.getInt("AVDD_SHDN");
            dif.DVDDShdn=x.getInt("DVDD_SHDN");
            dif.DIFIMonGain=x.getInt("DIF_IMON_GAIN");
            dif.SlabIMonGain=x.getInt("SLAB_IMON_GAIN");
            dif.MonitoredChannel=x.getInt("MONITORED_CHANNEL");
            dif.PowerAnalog=x.getInt("POWER_ANALOG");
            dif.PowerDigital=x.getInt("POWER_DIGITAL");
            dif.PowerDAC=x.getInt("POWER_DAC");
            dif.PowerADC=x.getInt("POWER_ADC");
            dif.PowerSS=x.getInt("POWER_SS");
            dif.TimerHoldRegister=x.getInt("TIMER_HOLD_REGISTER");
            dif.EnableMonitoring=x.getInt("ENABLE_MONITORING");
            dif.MemoryDisplayLimitMax=x.getInt("MEMORY_DISPLAY_LIMIT_MAX");
            dif.MemoryEfficiencyLimitMax=x.getInt("MEMORY_EFFICIENCY_LIMIT_MAX");
            dif.MemoryDisplayLimitMin=x.getInt("MEMORY_DISPLAY_LIMIT_MIN");
            dif.MemoryEfficiencyLimitMin=x.getInt("MEMORY_EFFICIENCY_LIMIT_MIN");
            self.sq_dif_id=dif.insert(self.condb)
        # Loop on ASICs and insert them
        for x in self.asics:
             hr2=rd.HR2Def(x.getInt("HEADER"))
             hr2.DIF_NUM=x.getInt("DIF_ID")
             hr2.SETUP_ID=self.sq_setup_id

             hr2.QScSrOutSc =x.getInt("QSCSROUTSC");
             hr2.EnOCDout1b =x.getInt("ENOCDOUT1B");
             hr2.EnOCDout2b =x.getInt("ENOCDOUT2B");
             hr2.EnOCTransmitOn1b =x.getInt("ENOCTRANSMITON1B");
             hr2.EnOCTransmitOn2b =x.getInt("ENOCTRANSMITON2B");
             hr2.EnOCchipSatb =x.getInt("ENOCCHIPSATB");
             hr2.SelEndReadout =x.getInt("SELENDREADOUT");
             hr2.SelStartReadout =x.getInt("SELSTARTREADOUT");
             hr2.ClkMux =x.getInt("CLKMUX");
             hr2.ScOn =x.getInt("SCON");
             hr2.RazChnExtVal =x.getInt("RAZCHNEXTVAL");
             hr2.RazChnIntVal =x.getInt("RAZCHNINTVAL");
             hr2.TrigExtVal =x.getInt("TRIGEXTVAL");
             hr2.DiscrOrOr =x.getInt("DISCROROR");
             hr2.EnTrigOut =x.getInt("ENTRIGOUT");
             hr2.Trig0b =x.getInt("TRIG0B");
             hr2.Trig1b =x.getInt("TRIG1B");
             hr2.Trig2b =x.getInt("TRIG2B");
             hr2.OtaBgSw =x.getInt("OTABGSW");
             hr2.DacSw =x.getInt("DACSW");
             hr2.SmallDac =x.getInt("SMALLDAC");
             hr2.B2 =x.getInt("B2");
             hr2.B1 =x.getInt("B1");
             hr2.B0 =x.getInt("B0");
             hr2.Mask2 =x.getString("MASK2");
             hr2.Mask1 =x.getString("MASK1");
             hr2.Mask0 =x.getString("MASK0");
             hr2.RS_or_Discri =x.getInt("RS_OR_DISCRI");
             hr2.Discri1 =x.getInt("DISCRI1");
             hr2.Discri2 =x.getInt("DISCRI2");
             hr2.Discri0 =x.getInt("DISCRI0");
             hr2.OtaQ_PwrADC =x.getInt("OTAQ_PWRADC");
             hr2.En_OtaQ =x.getInt("EN_OTAQ");
             hr2.Sw50f0 =x.getInt("SW50F0");
             hr2.Sw100f0 =x.getInt("SW100F0");
             hr2.Sw100k0 =x.getInt("SW100K0");
             hr2.Sw50k0 =x.getInt("SW50K0");
             hr2.PwrOnFsb1 =x.getInt("PWRONFSB1");
             hr2.PwrOnFsb2 =x.getInt("PWRONFSB2");
             hr2.PwrOnFsb0 =x.getInt("PWRONFSB0");
             hr2.Sel1 =x.getInt("SEL1");
             hr2.Sel0 =x.getInt("SEL0");
             hr2.Sw50f1 =x.getInt("SW50F1");
             hr2.Sw100f1 =x.getInt("SW100F1");
             hr2.Sw100k1 =x.getInt("SW100K1");
             hr2.Sw50k1 =x.getInt("SW50K1");
             hr2.Cmdb0Fsb1 =x.getInt("CMDB0FSB1");
             hr2.Cmdb1Fsb1 =x.getInt("CMDB1FSB1");
             hr2.Cmdb2Fsb1 =x.getInt("CMDB2FSB1");
             hr2.Cmdb3Fsb1 =x.getInt("CMDB3FSB1");
             hr2.Sw50f2 =x.getInt("SW50F2");
             hr2.Sw100f2 =x.getInt("SW100F2");
             hr2.Sw100k2 =x.getInt("SW100K2");
             hr2.Sw50k2 =x.getInt("SW50K2");
             hr2.Cmdb0Fsb2 =x.getInt("CMDB0FSB2");
             hr2.Cmdb1Fsb2 =x.getInt("CMDB1FSB2");
             hr2.Cmdb2Fsb2 =x.getInt("CMDB2FSB2");
             hr2.Cmdb3Fsb2 =x.getInt("CMDB3FSB2");
             hr2.PwrOnW =x.getInt("PWRONW");
             hr2.PwrOnSS =x.getInt("PWRONSS");
             hr2.PwrOnBuff =x.getInt("PWRONBUFF");
             hr2.SwSsc =x.getInt("SWSSC");
             hr2.CmdB0SS =x.getInt("CMDB0SS");
             hr2.CmdB1SS =x.getInt("CMDB1SS");
             hr2.CmdB2SS =x.getInt("CMDB2SS");
             hr2.CmdB3SS =x.getInt("CMDB3SS");
             hr2.PwrOnPA =x.getInt("PWRONPA");
             vg=x.getIntVector("PAGAIN")
             for i in range(64):
                 hr2.__dict__['PaGain%d' % i]=vg[i]
             hr2.insert(self.condb)
         
    def addLDA(self,lda_address):
        """
        Add an LDA
        """
        print "force LDA"
        theLDA=Lda()
        theLDA.setString("MAC_ADRESS",lda_address)
        self.ldaConf.add(theLDA)
        self.ldas.append(theLDA)
        print "LDA added ",lda_address
        #print "in DCCCONF: ",l.getString("LDA_ADDRESS"),l.getInt("LDA_CHANNEL"),lda_channel

    def addDCC(self,lda_address,lda_channel):
        """
        add a DCCC
        """
        print "force DCC"
        theDcc=Dcc()
        theDcc.setString("LDA_ADDRESS",lda_address)
        theDcc.setInt("LDA_CHANNEL",lda_channel)
        self.dccConf.add(theDcc)
        self.dccs.append(theDcc)
        print "\t DCC added",theDcc.getString("LDA_ADDRESS"),theDcc.getInt("LDA_CHANNEL")

    def addAsic(self,dif_num,header,gain=128,AsicType=2):
        """
        Add a new DIF and load asics conf from file if any
        dif_num = DIF ID
        nb_asic = Number of ASICs
        gain = default ASIC (HR2) gain (128)
        lda_address = LDA MAC address (ff:ff:ff:ff:ff:ff)
        lda_channel = LDA Channel (0)
        dcc_channel = DCC Channel (0)
        AsicType = Type of ASIC (2)
        xmlfile = XML list of ASICs (None)
        """
        print "force ASIC"
       
        
        if (AsicType == 2):
            theHr2=self.initAsic(dif_num,header,gain)
            self.asiclist.append(theHr2)
            self.asicConf.add(theHr2) 


    def addDIF(self,dif_num,nb_asic,gain=128,lda_address="ff:ff:ff:ff:ff:ff",lda_channel=0,dcc_channel=0,AsicType=2,xmlfile=None):
        """
        Add a new DIF and load asics conf from file if any
        dif_num = DIF ID
        nb_asic = Number of ASICs
        gain = default ASIC (HR2) gain (128)
        lda_address = LDA MAC address (ff:ff:ff:ff:ff:ff)
        lda_channel = LDA Channel (0)
        dcc_channel = DCC Channel (0)
        AsicType = Type of ASIC (2)
        xmlfile = XML list of ASICs (None)
        """
        print "force DIF"
        theDif = self.initDif(dif_num)
        if (lda_channel!=0):
            theDif.setString("LDA_ADDRESS",lda_address)
            theDif.setInt("LDA_CHANNEL",lda_channel)
            theDif.setInt("DCC_CHANNEL",dcc_channel)
 
        self.difConf.add(theDif)
        self.diflist.append(theDif)
        if (xmlfile!=None):
            self.asicConf.addFromXML(xmlfile)
        else:
            for ih in range(nb_asic):
                header=ih+1
                if (AsicType == 2):
                    theHr2=self.initAsic(dif_num,header,gain)
                    self.asiclist.append(theHr2)
                    self.asicConf.add(theHr2) 


    def initDif(self,num):
        """
        Default DHCAL Dif initialisation
        """
        d=Dif("DHCALDIF")
        d.setString('NAME',"FT101%.3d" % num)
        #d.setString('LDA_ADDRESS',lda_address)
        #d.setInt('LDA_CHANNEL',lda_chan)
        #d.setInt('DCC_CHANNEL',dcc_chan)
        d.setInt('ID',num)
        return d

    def setConstructionDB(self,consname):
        """
        Connect to the Construction DB
        consname= name of the construction database sqlite
        """
        self.contest =sqlite.connect(consname)

    def initialiseState(self,name):
        """
        Create a new state named 'name'
        """
      
        self.setupname=name
        self.asicConf=AsicConfiguration()
        self.difConf=DifConfiguration()
        self.ldaConf=LdaConfiguration()
        self.dccConf=DccConfiguration()
        self.state= State(name)
        self.setup=Setup(name)
        self.asics=self.asicConf.getVector();
        self.difs=self.difConf.getVector()


    def uploadFromFile(self,setupname,fname):
        """
        Upload a new setup from an XML file
        setupname = new setup name
        fname = XML file
        """
        try:
            print "Setup"
            self.setup=Setup(setupname)
        except Exception, e:
            print " Setup creation Problem : ",e.getMessage()
        try:
            print "State"
            self.state=State_createStateFromXML(setupname+"_"+fname,fname)
        except Exception, e:
            print " State creation Problem : ",e.getMessage()
        try:
            print "Addstate"
            self.setup.addState(self.state);
        except Exception, e:
            print " add State  Problem : ",e.getMessage()
        try:
            print "Upload"
            self.setup.upload()
        except Exception, e:
            print " upload Problem : ",e.getMessage()


    def uploadNewState(self,name):
        """
        upload newly created state in oracle with state named 'name'
        """
        if (len(self.ldas)>0):
            try:
                lda_vers=self.ldaConf.upload(name)
            except Exception, e:
                print "LDA upload Problem : ",e.getMessage()
                return
            print "et c est bon pour les LDA"


            try:
                dcc_vers=self.dccConf.upload(name)
            except Exception, e:
                print "DCC upload Problem : ",e.getMessage()
                return
            print "et c est bon pour les DCC"

        try:
            dif_vers=self.difConf.upload(name)
        except Exception, e:
            print "DIF upload Problem : ",e.getMessage()
            return
        print "et c est bon pour les DIF"
        try:
            asic_vers=self.asicConf.upload(name)
        except Exception, e:
            print "Asic upload Problem : ",e.getMessage()
            return
        print "et c est bon pour les ASIC"
        
        self.state.setAsicVersion(asic_vers)
        self.state.setDifVersion(dif_vers)
        v0=Version(0,0)
        try:
            if (len(self.ldas)>0):
                self.state.setLdaVersion(lda_vers)
                self.state.setDccVersion(dcc_vers)
            else:
                self.state.setLdaVersion(v0)
                self.state.setDccVersion(v0)

            self.state.upload()
        except Exception, e:
            print "State upload Problem : ",e.getMessage()
            return
        print "et c est bon pour le STATE"

        self.setup.addState(self.state)
        try:
            self.setup.upload()
        except Exception, e:
            print "Setup upload Problem : ",e.getMessage()
            return
        print "et c est bon pour le SETUP"
        #DBInit_terminate(); 
    
    def setLDACabling(self,dif_id,ether_address,lda_channel,dcc_channel):
      """
      Set the LDA cabling of the DIF dif_id
      for a in self.asics:
      """
      for a in self.difs:
          if (a.getInt("ID") != dif_id):
              continue;
          try:
              a.setString("LDA_ADDRESS",ether_address);
              a.setInt("LDA_CHANNEL",lda_channel);
              a.setInt("DCC_CHANNEL",dcc_channel);
              a.setModified(1)

          except Exception, e:
              print e.getMessage()
          print a.getInt('ID')


    def addSlot(self,slot,mask=7,AsicType=2):
        """
        RPC only
        Find the chamber in constrcution DB corresponding to slot nb=slot and add it to the setup
        slot = Chamber Slot
        mask = 1 for DIF1+ 2 for DIF2 +4 for DIF3 (7)
        AsicType = Type of Asic (2)
        """
        if (self.contest==None):
            return
        st1="SELECT CASSETTE FROM SLOT  WHERE ID='%d'" % slot
        c=self.contest.cursor()
 #       print str_cmd
        c.execute(st1)
        for row in c:
            print "Slot %d is chamber %d adding it " % (slot,-1*row[0])
            self.addChamber(-1*row[0],mask,AsicType)
            break
    def addChamber(self,chamber,mask=7,AsicType=2):
        """
        RPC only
        Find chamber 'chamber' in sqlite DB and add associated DIFs and ASIC to the Oracle DB. Gain parameters are taken from Rodolphe tests
        mask = 1 for DIF1+ 2 for DIF2 +4 for DIF3 (7)
        AsicType = Type of Asic (2)
        """
        if (self.contest==None):
            return
        if (chamber<0):
            usb=True
            chamber=-chamber
        else:
            usb=False
        if (mask!=7):
            print mask," masqued DIF"
        st1="SELECT number,dif1,dif2,dif3 FROM cassette  WHERE number='%d'" % chamber
        dif=[]
        c=self.contest.cursor()
 #       print str_cmd
        c.execute(st1)
        for row in c:
            num=row[0]
            if ((mask & 1)!=0):
                dif.append(row[1])
            if ((mask & 2)!=0):
                dif.append(row[2])
            if ((mask & 4)!=0):
                dif.append(row[3])
            break
        c.close()
        for d in dif:
            if (d == 0):
              continue;
            theDif = self.initDif(d)
            dif_num=d
            st2="SELECT number,asu1,asu2,ldaaddress,ldaslot,dccslot FROM dif  WHERE number='%d'" % d
            c=self.contest.cursor()
            c.execute(st2)
            asu1=''
            asu2=''
            for row in c:
                num=row[0]
                asu1=row[1]
                asu2=row[2]
                lda_eth=row[3]
                lda_channel=row[4]
                dcc_channel=row[5]
                if (usb):
                    lda_channel=0
            c.close()
            if (lda_channel!=0):
                theDif.setString("LDA_ADDRESS",lda_eth)
                theDif.setInt("LDA_CHANNEL",lda_channel)
                theDif.setInt("DCC_CHANNEL",dcc_channel)
            
            if (lda_channel != 0 ):
            # Check if the LDA is already committed
                lda_found=False
                for l in self.ldaConf.getVector():
                  #print "in LDACONF: ",l.getString("MAC_ADRESS"),lda_eth
                    if (l.getString("MAC_ADRESS") == lda_eth):
                        lda_found=True
                        break
                if (not lda_found):
                    theLDA=Lda()
                    theLDA.setString("MAC_ADRESS",lda_eth)
                    self.ldaConf.add(theLDA)
                    self.ldas.append(theLDA)
                    print "LDA added ",lda_eth
                dcc_found=False
                for l in self.dccConf.getVector():
                #print "in DCCCONF: ",l.getString("LDA_ADDRESS"),l.getInt("LDA_CHANNEL"),lda_channel
                    if (l.getString("LDA_ADDRESS") == lda_eth):
                        if (l.getInt("LDA_CHANNEL") == lda_channel):
                            dcc_found=True
                            break
                if (not dcc_found):
                    theDcc=Dcc()
                    theDcc.setString("LDA_ADDRESS",lda_eth)
                    theDcc.setInt("LDA_CHANNEL",lda_channel)
                    self.dccConf.add(theDcc)
                    self.dccs.append(theDcc)
                    print "\t DCC added",theDcc.getString("LDA_ADDRESS"),theDcc.getInt("LDA_CHANNEL")
            self.difConf.add(theDif)
            self.diflist.append(theDif)
            print "\t \t DIF added",dif_num
            for a in range(24):
                header=a+1
                st3="SELECT asic%d FROM asus WHERE barcode='%s'" % (a+1,asu1)
                c=self.contest.cursor()
                c.execute(st3)
                asicbc=""
                for row in c:
                    asicbc=row[0]
                c.close()
                if (AsicType==2):
                   theHr2=self.initAsic(dif_num,header)
                if (AsicType == 11):   
                   theHr2=self.initMicroroc(dif_num,header)
                self.asiclist.append(theHr2)
                if (AsicType == 2):
		   st4 =" SELECT "
                   for ic in range(64):
                       st4 = st4+"gain%d," % ic
                   st4=st4+"name  FROM asics WHERE barcode='%s'" % asicbc
                   c=self.contest.cursor()
                   c.execute(st4)
                   gain_vec=theHr2.getIntVector('PAGAIN')
                   for row in c:
                       for ic in range(64):
                           gain_vec[ic] = row[ic]
                   c.close()
                   theHr2.setIntVector('PAGAIN',gain_vec);

                   self.asicConf.add(theHr2)
                   # asics=self.asicConf.getVector()
                    #print len(self.asics)
                
                
                header=a+25
                st3="SELECT asic%d FROM asus WHERE barcode='%s'" % (a+1,asu2)
                c=self.contest.cursor()
                c.execute(st3)
                asicbc=""
                for row in c:
                    asicbc=row[0]
                c.close()

                if (AsicType==2):
                   theHr2p=self.initAsic(dif_num,header)
                if (AsicType == 11):   
                   theHr2p=self.initMicroroc(dif_num,header)
		if (AsicType ==2):		
               	   st4 =" SELECT "
                   for ic in range(64):
                       st4 = st4+"gain%d," % ic
                   st4=st4+"name  FROM asics WHERE barcode='%s'" % asicbc
                   c=self.contest.cursor()
                   c.execute(st4)
                   gain_vec=theHr2p.getIntVector('PAGAIN')
                   for row in c:
                       for ic in range(64):
                           gain_vec[ic] = row[ic]
                   c.close()
                   theHr2p.setIntVector('PAGAIN',gain_vec);
                   self.asiclist.append(theHr2p)
                   self.asicConf.add(theHr2p)
                   #asics=self.asicConf.getVector()
                   #print len(self.asics)

                   #print asics[len(asics)-1].toXML()

                   #print theHr2p.toXML()
        return dif
    def DumpDIFs(self):
        """
        return XML string of the list of DIFs
        """
        s=""
        for x in self.difConf.getVector():
            s=s+x.toXML()+"\n"
        return s
    def DumpAsics(self):
        """
        Print list of ASicx
        """
        for x in self.asicConf.getVector():
            print x.getInt('DIF_ID'),x.getInt('HEADER')
	      
	    
    def setTricotCR(self,idif=0,iasic=0):
         """
         change thresholds RC of Amplifier for strip mode on DIF idif
         and asic iasic.
         If not specified all Asics of a given DIF is changed
         if idif is not specified all Asics of all Difs are changed
         """
         for a in self.asics:
             if (idif !=0 and a.getInt("DIF_ID") != idif ):
                 continue;
             if (iasic !=0 and a.getInt("HEADER") != iasic):
                 continue;
             try:
                 a.setInt('SW50F0',1)
                 a.setInt('SW100F0',0)
                 a.setInt('SW100K0',0)
                 a.setInt('SW50K0',0)
                 a.setInt('SW50F1',1)
                 a.setInt('SW100F1',0)
                 a.setInt('SW100K1',0)
                 a.setInt('SW50K1',0)
                 a.setInt('SW50F2',1)
                 a.setInt('SW100F2',0)
                 a.setInt('SW100K2',0)
                 a.setInt('SW50K2',0)

                 #a.setInt('BB0',B0)
                 #a.setInt('BB1',B1)
                 #a.setInt('BB2',B2)
                
########## Cette ligne la dit a la methode  uploadChanges() que tu dois
#          appeler de faire un "commit" des chgts

                 a.setModified(1)
             except Exception, e:
                 print e.getMessage()	    

    def initMicroroc(self,dif,num):
        """
        Microroc  initialisation
        """
	print "***** init microroc"
        d=Asic('MR',dif,num)
        d.setInt('DIF_ID',dif)
        d.setInt('HEADER',num)
        d.setInt('ENOCDOUT1B',1)
        d.setInt('ENOCDOUT2B',0)
        d.setInt('ENOCTRANSMITON1B',1)
        d.setInt('ENOCTRANSMITON2B',0)
        d.setInt('ENOCCHIPSATB',1)
        d.setInt('SELENDREADOUT',1)
        d.setInt('SELSTARTREADOUT',1)
        d.setInt('CKMUX',1)
        d.setInt('SCON',1)
        d.setInt('RAZCHNEXTVAL',0)
        d.setInt('RAZCHNINTVAL',1)
        d.setInt('TRIGEXTVAL',0)
        d.setInt('DISCOROR',1)
        d.setInt('ENTRIGOUT',0)
        d.setInt('TRIG0B',1)
        d.setInt('TRIG1B',1)
        d.setInt('TRIG2B',1)
        d.setString('MASK2','0XFFFFFFFFFFFFFFFF')
        d.setString('MASK1','0XFFFFFFFFFFFFFFFF')
        d.setString('MASK0','0XFFFFFFFFFFFFFFFF')
        d.setString('CTEST','0X0000000000000000')
        d.setInt('RS_OR_DISCRI',1)
        d.setInt('DISCRI1_PP',1)
        d.setInt('DISCRI2_PP',1)
        d.setInt('DISCRI0_PP',1)
        d.setInt('EN_OTAQ',1)
        d.setInt('PWRONPA',1)
        d.setInt('BB0',180)
        d.setInt('BB1',500)
        d.setInt('BB2',900)
        d.setInt('ENABLED',1)
        d.setInt('ENGBST',1)
        d.setInt('ENPPBANDGAP',1)
        d.setInt('ENPPDAC',1)
        d.setInt('PWRONBG',1)
        d.setInt('PWRONDAC',1)
        d.setInt('ENPPDAC4BIT',1)
        d.setInt('PWRONSHHG',1)
        d.setInt('PWRONSHLG',1)
        d.setInt('PWRONWIDLAR',1)
        d.setInt('QSC',1)
        d.setInt('SELRAZ0',1)
        d.setInt('SELRAZ1',1)
        d.setInt('SWHG',1)
        d.setInt('SWLG',1)
        d.setInt('VALIDSH',0)
        dv=d.getIntVector('B0B3')
        for x in range(64):
            dv.append(7 )
        d.setIntVector('B0B3',dv)
        return d	

    def initAsic(self,dif,num,gain=128):
        """
        HardRoc 2  initialisation
        """
	#print "***** init HR2"
        d=Asic('HR2',dif,num)
        d.setInt('DIF_ID',dif)
        d.setInt('HEADER',num)
        d.setInt('QSCSROUTSC',1)
        d.setInt('ENOCDOUT1B',1)
        d.setInt('ENOCDOUT2B',0)
        d.setInt('ENOCTRANSMITON1B',1)
        d.setInt('ENOCTRANSMITON2B',0)
        d.setInt('ENOCCHIPSATB',1)
        d.setInt('SELENDREADOUT',1)
        d.setInt('SELSTARTREADOUT',1)
        d.setInt('CLKMUX',1)
        d.setInt('SCON',0)
        d.setInt('RAZCHNEXTVAL',0)
        d.setInt('RAZCHNINTVAL',1)
        d.setInt('TRIGEXTVAL',0)
        d.setInt('DISCROROR',1)
        d.setInt('ENTRIGOUT',1)
        d.setInt('TRIG0B',1)
        d.setInt('TRIG1B',0)
        d.setInt('TRIG2B',0)
        d.setInt('OTABGSW',0)
        d.setInt('DACSW',0)
        d.setInt('SMALLDAC',0)
        d.setInt('B2',250)
        d.setInt('B1',250)
        d.setInt('B0',250)
        d.setString('MASK2','0XFFFFFFFFFFFFFFFF')
        d.setString('MASK1','0XFFFFFFFFFFFFFFFF')
        d.setString('MASK0','0XFFFFFFFFFFFFFFFF')
        d.setInt('RS_OR_DISCRI',1)
        d.setInt('DISCRI1',0)
        d.setInt('DISCRI2',0)
        d.setInt('DISCRI0',0)
        d.setInt('OTAQ_PWRADC',0)
        d.setInt('EN_OTAQ',1)
        d.setInt('SW50F0',1)
        d.setInt('SW100F0',1)
        d.setInt('SW100K0',1)
        d.setInt('SW50K0',1)
        d.setInt('PWRONFSB1',0)
        d.setInt('PWRONFSB2',0)
        d.setInt('PWRONFSB0',0)
        d.setInt('SEL1',0)
        d.setInt('SEL0',1)
        d.setInt('SW50F1',1)
        d.setInt('SW100F1',1)
        d.setInt('SW100K1',1)
        d.setInt('SW50K1',1)
        d.setInt('CMDB0FSB1',1)
        d.setInt('CMDB1FSB1',1)
        d.setInt('CMDB2FSB1',0)
        d.setInt('CMDB3FSB1',1)
        d.setInt('SW50F2',1)
        d.setInt('SW100F2',1)
        d.setInt('SW100K2',1)
        d.setInt('SW50K2',1)
        d.setInt('CMDB0FSB2',1)
        d.setInt('CMDB1FSB2',1)
        d.setInt('CMDB2FSB2',0)
        d.setInt('CMDB3FSB2',1)
        d.setInt('PWRONW',0)
        d.setInt('PWRONSS',0)
        d.setInt('PWRONBUFF',0)
        d.setInt('SWSSC',7)
        d.setInt('CMDB0SS',0)
        d.setInt('CMDB1SS',0)
        d.setInt('CMDB2SS',0)
        d.setInt('CMDB3SS',0)
        d.setInt('PWRONPA',0)
        # Unset power pulsing
        d.setInt('CLKMUX',1)
        d.setInt('SCON',1)
        d.setInt('OTAQ_PWRADC',1)
        d.setInt('PWRONW',1)
        d.setInt('PWRONSS',0)
        d.setInt('PWRONBUFF',1)
        d.setInt('PWRONPA',1)
        d.setInt('DISCRI0',1)
        d.setInt('DISCRI1',1)
        d.setInt('DISCRI2',1)
        d.setInt('OTABGSW',1)
        d.setInt('DACSW',1)
        d.setInt('PWRONFSB0',1)
        d.setInt('PWRONFSB1',1)
        d.setInt('PWRONFSB2',1)
        dv=d.getIntVector('PAGAIN')
        for x in range(64):
            dv.append(gain)
        d.setIntVector('PAGAIN',dv)
        return d
	
####
  

    def end(self):
        """
        close the DB
        """
        try:
            del self.setup;
            
            DBInit_terminate();
            print "termninate called"
        except Exception, e:
            print "initOracle:: : ",e.getMessage()
            return



    def dumpStateNames(self):
        """
        print the list of states in the DB
        """
        try:
            for x in Setup_getSetupNames():
                print x
        except Exception, e:
            print "initOracle::DumpState : ",e.getMessage()
            return

    def parseChamber(self,chamberInfo,mask=7,AsicType=2):
        """
        Find chamber 'chamber' in sqlite DB and add associated DIFs and ASIC to the Oracle DB. Gain parameters are taken from Rodolphe tests
        """
        if (mask!=7):
            print mask," masqued DIF"

        # rrr
        if (len(chamberInfo.difs)==0):
            return


        dif=[]
        for x in chamberInfo.difs:
            num=x.dif_id
            if ((mask & 1)!=0 ):
                dif.append(num)
            if ((mask & 2)!=0):
                dif.append(num)
            if ((mask & 4)!=0):
                dif.append(num)
            break
        for d in dif:
            if (d == 0):
              continue;
            theDif = self.initDif(d)
            dif_num=d
            theDif.setString("LDA_ADDRESS",dif.lda_eth)
            theDif.setInt("LDA_CHANNEL",dif.lda_channel)
            theDif.setInt("DCC_CHANNEL",dif.dcc_channel)
            
            if (lda_channel != 0 ):
            # Check if the LDA is already committed
                lda_found=False
                for l in self.ldaConf.getVector():
                  #print "in LDACONF: ",l.getString("MAC_ADRESS"),lda_eth
                    if (l.getString("MAC_ADRESS") == lda_eth):
                        lda_found=True
                        break
                if (not lda_found):
                    theLDA=Lda()
                    theLDA.setString("MAC_ADRESS",lda_eth)
                    self.ldaConf.add(theLDA)
                    self.ldas.append(theLDA)
                    print "LDA added ",lda_eth
                dcc_found=False
                for l in self.dccConf.getVector():
                #print "in DCCCONF: ",l.getString("LDA_ADDRESS"),l.getInt("LDA_CHANNEL"),lda_channel
                    if (l.getString("LDA_ADDRESS") == lda_eth):
                        if (l.getInt("LDA_CHANNEL") == lda_channel):
                            dcc_found=True
                            break
                if (not dcc_found):
                    theDcc=Dcc()
                    theDcc.setString("LDA_ADDRESS",lda_eth)
                    theDcc.setInt("LDA_CHANNEL",lda_channel)
                    self.dccConf.add(theDcc)
                    self.dccs.append(theDcc)
                    print "\t DCC added",theDcc.getString("LDA_ADDRESS"),theDcc.getInt("LDA_CHANNEL")
            self.difConf.add(theDif)
            self.difs.append(theDif)
            print "\t \t DIF added",dif_num
            for a in range(24):
                header=a+1
                st3="SELECT asic%d FROM asus WHERE barcode='%s'" % (a+1,asu1)
                c=self.contest.cursor()
                c.execute(st3)
                asicbc=""
                for row in c:
                    asicbc=row[0]
                c.close()
                if (AsicType==2):
                   theHr2=self.initAsic(dif_num,header)
                if (AsicType == 11):   
                   theHr2=self.initMicroroc(dif_num,header)
                self.asics.append(theHr2)
                if (AsicType == 2):
		   st4 =" SELECT "
                   for ic in range(64):
                       st4 = st4+"gain%d," % ic
                   st4=st4+"name  FROM asics WHERE barcode='%s'" % asicbc
                   c=self.contest.cursor()
                   c.execute(st4)
                   gain_vec=theHr2.getIntVector('PAGAIN')
                   for row in c:
                       for ic in range(64):
                           gain_vec[ic] = row[ic]
                   c.close()
                   theHr2.setIntVector('PAGAIN',gain_vec);

                   self.asicConf.add(theHr2)
                   # asics=self.asicConf.getVector()
                    #print len(self.asics)
                
                
                header=a+25
                st3="SELECT asic%d FROM asus WHERE barcode='%s'" % (a+1,asu2)
                c=self.contest.cursor()
                c.execute(st3)
                asicbc=""
                for row in c:
                    asicbc=row[0]
                c.close()

                if (AsicType==2):
                   theHr2p=self.initAsic(dif_num,header)
                if (AsicType == 11):   
                   theHr2p=self.initMicroroc(dif_num,header)
		if (AsicType ==2):		
               	   st4 =" SELECT "
                   for ic in range(64):
                       st4 = st4+"gain%d," % ic
                   st4=st4+"name  FROM asics WHERE barcode='%s'" % asicbc
                   c=self.contest.cursor()
                   c.execute(st4)
                   gain_vec=theHr2p.getIntVector('PAGAIN')
                   for row in c:
                       for ic in range(64):
                           gain_vec[ic] = row[ic]
                   c.close()
                   theHr2p.setIntVector('PAGAIN',gain_vec);
                   self.asics.append(theHr2p)
                   self.asicConf.add(theHr2p)
                   #asics=self.asicConf.getVector()
                   #print len(self.asics)

                   #print asics[len(asics)-1].toXML()

                   #print theHr2p.toXML()
        return dif



