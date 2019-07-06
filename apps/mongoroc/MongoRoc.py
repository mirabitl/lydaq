
import os
from pymongo import MongoClient
import json
from bson.objectid import ObjectId


def IP2Int(ip):
    o = map(int, ip.split('.'))
    res = (16777216 * o[3]) + (65536 * o[2]) + (256 * o[1]) + o[0]
    return res


class MongoRoc:
    """
    Main class to access the Mongo DB
    """

    def __init__(self, host,port,dbname,username,pwd):
        """
        connect Mongodb database named dbname
        """
        self.connection=MongoClient(host,port)
        self.db=self.connection[dbname]
        self.db.authenticate(username,pwd)
        self.state = {}
        self.asiclist = []
        self.bson_id=[]
    def createNewState(self,name):
        self.state["name"]=name
        self.state["version"]=1
        self.state["asics"]=[]
    def addFEB(self,ipname,nasic,asictype="PR2"):
        febid=IP2Int(ipname)
        for i in range(nasic):
            asic={}
            asic["dif"]=febid
            asic["num"]=i+1
            asic["slc"]=self.initPR2(i+1,asictype)
            asic["_id"]=None
            print asic["dif"],asic["num"],asic["_id"]," is added"
            self.asiclist.append(asic)
    def uploadNewState(self):
        # First append modified ASICS
        for i in range(len(self.asiclist)):
            if (self.asiclist[i]["_id"]!=None):
                continue
            del self.asiclist[i]["_id"]
            result=self.db.asics.insert_one(self.asiclist[i])
            self.asiclist[i]["_id"]=result.inserted_id
        for  i in range(len(self.asiclist)):
            self.bson_id.append(self.asiclist[i]["_id"])
        self.state["asics"]=self.bson_id
        resstate=self.db.states.insert_one(self.state)
        print resstate
    def download(self,statename,version):
        res=self.db.states.find({'name':statename,'version':version})
        for x in res:
            print x["name"],x["version"],x["asics"]
            self.state["name"]=x["name"]
            self.state["version"]=x["version"]
            #var=raw_input()
            slc={}
            slc["state"]=statename
            slc["version"]=version
            slc["asics"]=[]
            self.asiclist=[]
            for y in x["asics"]:
                resa=self.db.asics.find_one({'_id':y})
                self.asiclist.append(resa)
                s={}
                s["slc"]=resa["slc"]
                s["num"]=resa["num"]
                s["dif"]=resa["dif"]
                #print res["dif"]
                slc["asics"].append(s)
            f=open("/dev/shm/%s_%s.json" % (statename,version),"w+")
            f.write(json.dumps(slc))
            f.close()
            return slc
    def initPR2(self, num,version="PR2"):
        """
        PETIROC 2  initialisation
        """
	#print "***** init HR2"
        _jasic={}
        _jasic["header"]=num
        _jasic["EN10bDac"] = 1
        _jasic["PP10bDac"] = 0
        _jasic["EN_adc"] =0
        _jasic["PP_adc"] = 0
        _jasic["sel_starb_ramp_adc_ext"] = 0
        _jasic["usebcompensation"] = 0
        _jasic["EN_bias_dac_delay"] = 0
        _jasic["PP_bias_dac_delay"] = 0
        _jasic["EN_bias_ramp_delay"] = 0
        _jasic["PP_bias_ramp_delay"] = 0
        _jasic["EN_discri_delay"] =0
        _jasic["PP_discri_delay"] = 0
        _jasic["EN_temp_sensor"] = 0
        _jasic["PP_temp_sensor"] = 0
        _jasic["EN_bias_pa"] = 1
        _jasic["PP_bias_pa"] = 0
        _jasic["EN_bias_discri"] = 1
        _jasic["PP_bias_discri"] = 0
        _jasic["cmd_polarity"] = 0
        _jasic["latch"] = 1
        _jasic["EN_bias_6bit_dac"] =1
        _jasic["PP_bias_6bit_dac"] = 0
        _jasic["EN_bias_tdc"] = 0
        _jasic["PP_bias_tdc"] = 0
        _jasic["ON_OFF_input_dac"] = 1
        _jasic["EN_bias_charge"] = 0
        _jasic["PP_bias_charge"] = 0
        _jasic["Cf3_100fF"] = 0
        _jasic["Cf2_200fF"] = 0
        _jasic["Cf1_2p5pF"] = 0
        _jasic["Cf0_1p25pF"] = 0
        _jasic["EN_bias_sca"] = 0
        _jasic["PP_bias_sca"] = 0
        _jasic["EN_bias_discri_charge"] = 0
        _jasic["PP_bias_discri_charge"] = 0
        _jasic["EN_bias_discri_adc_time"] = 0
        _jasic["PP_bias_discri_adc_time"] = 0
        _jasic["EN_bias_discri_adc_charge"] = 0
        _jasic["PP_bias_discri_adc_charge"] = 0
        _jasic["DIS_razchn_int"] = 1
        _jasic["DIS_razchn_ext"] = 0
        _jasic["SEL_80M"] = 0
        _jasic["EN_80M"] = 0
        _jasic["EN_slow_lvds_rec"] = 1
        _jasic["PP_slow_lvds_rec"] = 0
        _jasic["EN_fast_lvds_rec"] = 1
        _jasic["PP_fast_lvds_rec"] = 0
        _jasic["EN_transmitter"] = 0
        _jasic["PP_transmitter"] = 0
        _jasic["ON_OFF_1mA"] =1
        _jasic["ON_OFF_2mA"] = 1
        _jasic["ON_OFF_otaQ"] = 0
        _jasic["ON_OFF_ota_mux"] = 0
        _jasic["ON_OFF_ota_probe"] = 0
        _jasic["DIS_trig_mux"] = 1
        _jasic["EN_NOR32_time"] = 1
        _jasic["EN_NOR32_charge"] = 0
        _jasic["DIS_triggers"] = 0
        _jasic["EN_dout_oc"] = 0
        _jasic["EN_transmit"] = 1
        if (version == "PR2B"):
            _jasic["PA_ccomp_0"] =0
            _jasic["PA_ccomp_1"] =0
            _jasic["PA_ccomp_2"] = 0
            _jasic["PA_ccomp_3"] =0
            _jasic["Choice_Trigger_Out"] =0
        _jasic["DacDelay"] = 0
        idac=[]
        bdac=[]
        mdc=[]
        mdt=[]
        idc=[]
        for ch in range(32):
            idac.append(125);
            bdac.append(31);
            mdc.append(1);
            mdt.append(0)
            idc.append(1)
    
        _jasic["InputDac"] = idac;
        _jasic["6bDac"] = bdac;
        _jasic["MaskDiscriCharge"] = mdc;
        _jasic["MaskDiscriTime"] = mdt;
        _jasic["InputDacCommand"] = idc;

        _jasic["VthDiscriCharge"] = 863
        _jasic["VthTime"] = 610

        return _jasic


    def uploadChanges(self,statename):
        """
        Upload a new version
        The state name will be, old_state_name_xx where xx is the new index (starting from 00)
        """
        # Find last version
        res=self.db.states.find({'name':statename})
        last=0
        for x in res:
            last=x["version"]
        if (last==0):
            print " No state ",statename,"found"
            return
        # First append modified ASICS
        for i in range(len(self.asiclist)):
            if (self.asiclist[i]["_id"]!=None):
                continue
            del self.asiclist[i]["_id"]
            result=self.db.asics.insert_one(self.asiclist[i])
            self.asiclist[i]["_id"]=result.inserted_id
        self.bson_id=[]
        for  a in self.asiclist:
            print a
            print a["_id"]
            self.bson_id.append(a["_id"])
        self.state["asics"]=self.bson_id
        self.state["version"]=last+1
        resstate=self.db.states.insert_one(self.state)
        print resstate,self.state["version"],self.state["name"]
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

 
    def ChangeLatch(self, Latch, idif=0, iasic=0):
        """
        Change the Latch mode of the asic #asic on the TDCDIF #dif
        If 0 all hardware is changed
        """
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            try:
                a["slc"]["latch"]=Latch
                a["_id"]=None
            except Exception, e:
                print e.getMessage()

    def ChangeVthTime(self, VthTime, idif=0, iasic=0):
        """
        Change the VTHTIME  of the asic #asic on the TDCDIF #dif
        If 0 all hardware is changed
        """
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            try:
                a["slc"]["VthTime"]=VthTime
                a["_id"]=None
            except Exception, e:
                print e.getMessage()


    def ChangeDacDelay(self, delay, idif=0, iasic=0):
        """
        Change the DACDELAY  of the asic #asic on the TDCDIF #dif
        If 0 all hardware is changed
        """
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            try:
                a["slc"]["DacDelay"]=delay
                a["_id"]=None
            except Exception, e:
                print e.getMessage()

    def ChangeAllEnabled(self, idif=0, iasic=0):
        """
        Change all the ENable signals  of the asic #asic on the TDCDIF #dif
        If 0 all hardware is changed
        """
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            try:
                a["slc"]["EN_bias_discri"]=1
                a["slc"]["EN_bias_pa"]=1
                a["slc"]["EN_bias_discri_charge"]=1
                a["slc"]["EN_dout_oc"]=1
                a["slc"]["EN_bias_dac_delay"]=1
                a["slc"]["EN10bdac"]=1
                a["slc"]["EN_bias_discri_adc_charge"]=1
                a["slc"]["EN_bias_sca"]=1
                a["slc"]["EN_bias_6bit_dac"]=1
                a["slc"]["EN_transmit"]=1
                a["slc"]["EN_bias_ramp_delay"]=1
                a["slc"]["EN_bias_charge"]=1
                a["slc"]["EN_fast_lvds_rec"]=1
                a["slc"]["EN_transmitter"]=1
                a["slc"]["EN_adc"]=1
                a["slc"]["EN_NOR32_charge"]=1
                a["slc"]["EN_80M"]=1
                a["slc"]["EN_discri_delay"]=1
                a["slc"]["EN_bias_discri_adc_time"]=1
                a["slc"]["EN_NOR32_time"]=1
                a["slc"]["EN_temp_sensor"]=1

                a["_id"]=None

            except Exception, e:
                print e.getMessage()

    def Change6BDac(self, idif, iasic, ich, dac):
        """
        Change the 6BDAC value to dac  of the asic #asic on the TDCDIF #dif       
        """

        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            try:
                a["slc"]["6bDac"][ich]=dac
                a["_id"]=None
            except Exception, e:
                print e.getMessage()
    def Correct6BDac(self, idif, iasic, cor):
        """
        Change the 6BDAC value   of the asic #asic on the TDCDIF #dif
        cor is an array of 32 value , 
        6BDAC[i]=6BDAC[i]+cor[i]
        """
        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            try:
                for ich in range(32):
                    print " Dac changed", idif, iasic, ich, vg[ich], cor[ich]
                    a["slc"]["6bDac"][ich] = a["slc"]["6bDac"][ich]+cor[ich]

                a["_id"]=None
            except Exception, e:
                print e.getMessage()


    def ChangeMask(self, idif, iasic, ich, mask):
        """
        Change PETIROC2 MASKDISCRITIME parameter for one channel
        Careful: 1 = inactive, 0=active
        """

        for a in self.asiclist:
            if (idif != 0 and a["dif"] != idif):
                continue
            if (iasic != 0 and a["num"] != iasic):
                continue
            try:
                a["slc"]["VthTime"]=VthTime
                for ich in range(32):
                    a["slc"]["MaskDiscriTime"][ich] = (mask>>ich)&1

                a["_id"]=None
            except Exception, e:
                print e.getMessage()

