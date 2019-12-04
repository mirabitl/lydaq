
from ilcconfdb import *
import os
import sqlite3 as sqlite
def IP2Int(ip):
    o = map(int, ip.split('.'))
    res = (16777216 * o[3]) + (65536 * o[2]) + (256 * o[1]) + o[0]
    return res
class GricAccess:
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
            
    def slowShaper(self):
        """
        Unset Power pulsing on all ASICs
        """
        for a in self.asics: 
            print a.getInt('DISCRI0')
            a.setInt('SW100F0',1)
            a.setInt('SW100K0',1)
            a.setInt('SW50F0',0)
            a.setInt('SW50K0',0)

            a.setInt('SW100F1',1)
            a.setInt('SW100K1',1)
            a.setInt('SW50F1',0)
            a.setInt('SW50K1',0)

            a.setInt('SW100F2',1)
            a.setInt('SW100K2',1)
            a.setInt('SW50F2',0)
            a.setInt('SW50K2',0)

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

    def ChangeHeader(self,headern,idif=0,iasic=0):
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
                a.setInt("HEADER",headern);
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

    def SetMask(self,list,idif=0,iasic=0):
        m=0xFFFFFFFFFFFFFFFF
        for i in list:
            m=m &~(1<<i);
        sm="0x%lx" % m
        self.ChangeMask(sm,sm,sm,idif,iasic)
    def ChangeCTEST(self,CTEST,idif=0,iasic=0):
        for a in self.asics:
            if (idif!=0 and a.getInt("DIF_ID") != idif):
                continue;
            if (iasic!=0 and a.getInt("HEADER") != iasic):
                 continue;
            print a.getInt('DIF_ID'),a.getInt('HEADER'),a.getString("CTEST")
            try:
                a.setString("CTEST",CTEST);
                a.setInt("TRIG0B",1)
                a.setInt("TRIG1B",1)
                a.setInt("TRIG2B",1)
                a.setModified(1)

            except Exception, e:
                print e.getMessage()
            print a.getInt('DIF_ID'),a.getInt('HEADER'),a.getString("CTEST")



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
       
        

        thePR2=self.initAsic(dif_num,header,128)
        self.asiclist.append(thePR2)
        self.asicConf.add(thePR2) 


    def addDIF(self,ipaddr,nb_asic,lda_address="ff:ff:ff:ff:ff:ff",lda_channel=0,dcc_channel=0,xmlfile=None):
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
        theDif = self.initDif(ipaddr)
        dif_num=theDif.getInt("ID")
        print "DIF number %d" % (dif_num) 
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

                thePR2=self.initAsic(dif_num,header,128)
                self.asiclist.append(thePR2)
                self.asicConf.add(thePR2) 


    def initDif(self,addr):
        """
        Default DHCAL Dif initialisation
        """
        d=Dif("TDCDIF")
        d.setString('NAME',"TDC%d" % ((IP2Int(addr)>>16)&0xFFFF))
        d.setString('IP_ADDRESS',addr)
        d.setInt('TYPE',2)
        #d.setString('LDA_ADDRESS',lda_address)
        #d.setInt('LDA_CHANNEL',lda_chan)
        #d.setInt('DCC_CHANNEL',dcc_chan)
        d.setInt('ID',(IP2Int(addr)>>16)&0xFFFF)
        print "creating DIF %d %x" % ((IP2Int(addr)>>16)&0xFFFF,IP2Int(addr))
        d.thisown=0
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
        d.setString('CTEST','0X0')
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
        asi.setInt('EN_BIAS_DISCRI_CHARGE',0)
        asi.setInt('EN_DOUT_OC',0)
        asi.setInt('ON_OFF_OTAQ',0)
        asi.setInt('DIS_RAZCHN_EXT',0)
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
        asi.setInt('EN_BIAS_SCA',0)
        asi.setInt('EN_BIAS_6BIT_DAC',1)
        asi.setInt('PP_TRANSMITTER',0)
        asi.setInt('PP_TEMP_SENSOR',0)
        asi.setInt('EN_TRANSMIT',0)
        asi.setInt('PP_BIAS_CHARGE',0)
        asi.setInt('PP_BIAS_DISCRI_ADC_TIME',0)
        asi.setInt('ON_OFF_OTA_MUX',0)
        asi.setInt('CMD_POLARITY',0)
        asi.setInt('PP_BIAS_DAC_DELAY',0)
        asi.setInt('CF1_2P5PF',0)
        asi.setInt('PP_FAST_LVDS_REC',0)
        asi.setInt('EN_BIAS_RAMP_DELAY',0)
        asi.setInt('PP10BDAC',0)
        asi.setInt('EN_BIAS_CHARGE',0)
        asi.setInt('EN_FAST_LVDS_REC',1)
        asi.setInt('PP_BIAS_TDC',0)
        asi.setInt('CF0_1P25PF',0)
        asi.setInt('EN_TRANSMITTER',0)
        asi.setInt('EN_ADC',0)
        asi.setInt('EN_NOR32_CHARGE',0)
        asi.setInt('EN_80M',0)
        asi.setInt('DIS_TRIG_MUX',1)
        asi.setInt('PP_BIAS_DISCRI_CHARGE',0)
        asi.setInt('ON_OFF_1MA',1)
        asi.setInt('ON_OFF_OTA_PROBE',0)
        asi.setInt('PP_DISCRI_DELAY',0)
        asi.setInt('EN_DISCRI_DELAY',0)
        asi.setInt('LATCH',1)
        asi.setInt('EN_BIAS_DISCRI_ADC_TIME',0)
        asi.setInt('EN_NOR32_TIME',1)
        asi.setInt('EN_TEMP_SENSOR',0)
        asi.setInt('PP_BIAS_PA',0)
        asi.setInt('PP_ADC',0)
        asi.setInt('PP_BIAS_DISCRI',0)
        asi.setInt('VTHTIME',610)
        asi.setInt('SEL_80M',0)
        asi.setInt('EN_BIAS_TDC',0)
        asi.setInt('DIS_RAZCHN_INT',1)
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
        asi.thisown=0

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



