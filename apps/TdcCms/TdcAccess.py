

from ilcconfdb import *
import os
import sqlite3 as sqlite
class TdcAccess:
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

    def addAsic(self,dif_num,header):
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
       
        

        thePR2=self.initPR2(dif_num,header)
        self.asiclist.append(thePR2)
        self.asicConf.add(thePR2) 


    def addDIF(self,dif_num,ipaddr,nb_asic,lda_address="ff:ff:ff:ff:ff:ff",lda_channel=0,dcc_channel=0,xmlfile=None):
        """
        Add a new DIF and load asics conf from file if any
        dif_num = DIF ID
        nb_asic = Number of ASICs
        lda_address = LDA MAC address (ff:ff:ff:ff:ff:ff)
        lda_channel = LDA Channel (0)
        dcc_channel = DCC Channel (0)
        xmlfile = XML list of ASICs (None)
        """
        print "force DIF"
        theDif = self.initDif(dif_num,ipaddr)
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

                thePR2=self.initPR2(dif_num,header)
                self.asiclist.append(thePR2)
                self.asicConf.add(thePR2) 


    def initDif(self,num,addr):
        """
        Default DHCAL Dif initialisation
        """
        d=Dif("TDCDIF")
        d.setString('NAME',"TDC%d" % num)
        d.setString('IP_ADDRESS',addr)
        d.setInt('TYPE',2)
        #d.setString('LDA_ADDRESS',lda_address)
        #d.setInt('LDA_CHANNEL',lda_chan)
        #d.setInt('DCC_CHANNEL',dcc_chan)
        d.setInt('ID',num)
        return d


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
	      
	    
    def initPR2(self,dif,num):
        """
        HardRoc 2  initialisation
        """
	#print "***** init HR2"
        asi=Asic('PR2',dif,num)
        asi.setInt('DIF_ID',dif)
        asi.setInt('HEADER',num)
        asi.setInt('EN_BIAS_DISCRI',1)
        asi.setInt('CF2_200FF',0)
        asi.setInt('CF3_100FF',0)
        asi.setInt('DACDELAY',31)
        asi.setInt('PP_BIAS_RAMP_DELAY',0)
        asi.setInt('ON_OFF_2MA',1)
        asi.setInt('EN_BIAS_PA',1)
        asi.setInt('PP_BIAS_6BIT_DAC',0)
        asi.setInt('EN_BIAS_DISCRI_CHARGE',1)
        asi.setInt('EN_DOUT_OC',0)
        asi.setInt('ON_OFF_OTAQ',1)
        asi.setInt('DIS_RAZCHN_EXT',1)
        asi.setInt('EN_BIAS_DAC_DELAY',0)
        asi.setInt('PP_SLOW_LVDS_REC',0)
        asi.setInt('EN10BDAC',1)
        asi.setInt('PP_BIAS_SCA',0)
        asi.setInt('USEBCOMPENSATION',0)
        asi.setInt('VTHDISCRICHARGE',863)
        asi.setInt('EN_BIAS_DISCRI_ADC_CHARGE',0)
        asi.setInt('PP_BIAS_DISCRI_ADC_CHARGE',0)
        asi.setInt('EN_SLOW_LVDS_REC',1)
        asi.setInt('SEL_STARB_RAMP_ADC_EXT',0)
        asi.setInt('DIS_TRIGGERS',0)
        asi.setInt('ON_OFF_INPUT_DAC',1)
        asi.setInt('EN_BIAS_SCA',1)
        asi.setInt('EN_BIAS_6BIT_DAC',1)
        asi.setInt('PP_TRANSMITTER',0)
        asi.setInt('PP_TEMP_SENSOR',1)
        asi.setInt('EN_TRANSMIT',0)
        asi.setInt('PP_BIAS_CHARGE',0)
        asi.setInt('PP_BIAS_DISCRI_ADC_TIME',0)
        asi.setInt('ON_OFF_OTA_MUX',1)
        asi.setInt('CMD_POLARITY',0)
        asi.setInt('PP_BIAS_DAC_DELAY',0)
        asi.setInt('CF1_2P5PF',0)
        asi.setInt('PP_FAST_LVDS_REC',0)
        asi.setInt('EN_BIAS_RAMP_DELAY',0)
        asi.setInt('PP10BDAC',0)
        asi.setInt('EN_BIAS_CHARGE',1)
        asi.setInt('EN_FAST_LVDS_REC',0)
        asi.setInt('PP_BIAS_TDC',0)
        asi.setInt('CF0_1P25PF',0)
        asi.setInt('EN_TRANSMITTER',0)
        asi.setInt('EN_ADC',0)
        asi.setInt('EN_NOR32_CHARGE',1)
        asi.setInt('EN_80M',1)
        asi.setInt('DIS_TRIG_MUX',0)
        asi.setInt('PP_BIAS_DISCRI_CHARGE',0)
        asi.setInt('ON_OFF_1MA',1)
        asi.setInt('ON_OFF_OTA_PROBE',1)
        asi.setInt('PP_DISCRI_DELAY',0)
        asi.setInt('EN_DISCRI_DELAY',0)
        asi.setInt('LATCH',0)
        asi.setInt('EN_BIAS_DISCRI_ADC_TIME',0)
        asi.setInt('EN_NOR32_TIME',1)
        asi.setInt('EN_TEMP_SENSOR',0)
        asi.setInt('PP_BIAS_PA',0)
        asi.setInt('PP_ADC',0)
        asi.setInt('PP_BIAS_DISCRI',0)
        asi.setInt('VTHTIME',700)
        asi.setInt('SEL_80M',1)
        asi.setInt('EN_BIAS_TDC',0)
        asi.setInt('DIS_RAZCHN_INT',0)
        d6b=asi.getIntVector('DAC6B')
        for x in range(32):
            d6b.append(31)
        asi.setIntVector('DAC6B',d6b)
        didac=asi.getIntVector('INPUTDAC')
        for x in range(32):
            didac.append(125)
        asi.setIntVector('INPUTDAC',didac)
        didacc=asi.getIntVector('INPUTDACCOMMAND')
        for x in range(32):
            didacc.append(1)
        asi.setIntVector('INPUTDACCOMMAND',didacc)
        dmdc=asi.getIntVector('MASKDISCRICHARGE')
        for x in range(32):
            dmdc.append(1)
        asi.setIntVector('MASKDISCRICHARGE',dmdc)
        dmdt=asi.getIntVector('MASKDISCRITIME')
        for x in range(32):
            dmdt.append(0)
        asi.setIntVector('MASKDISCRITIME',dmdt)
        return asi
	
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



