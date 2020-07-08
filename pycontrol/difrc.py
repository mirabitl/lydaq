from __future__ import absolute_import
from __future__ import print_function
import lydaqrc
import time
import MongoJob as mg
import json
import os



class difRC(lydaqrc.lydaqControl):

    # daq
    def daq_initialise(self):
        m = {}
        r = {}
        print(self.appMap)

        if "GPIOSERVER" in self.appMap.keys():
            s = json.loads(self.appMap['GPIOSERVER'][0].sendTransition("CONFIGURE", m))
            r["GPIOSERVER"] = s
             json.loads(self.appMap['GPIOSERVER'][0].sendCommand("VMEON", {}))
             json.loads(self.appMap['GPIOSERVER'][0].sendCommand("VMEOFF", {}))
             json.loads(self.appMap['GPIOSERVER'][0].sendCommand("VMEON", {}))
             time.sleep(5)
        
        s = json.loads(self.appMap['CCCSERVER'][0].sendTransition("OPEN", m))
        s = json.loads(self.appMap['CCCSERVER'][0].sendTransition("INITIALISE", m))
        s = json.loads(self.appMap['CCCSERVER'][0].sendTransition("CONFIGURE", m))
        json.loads(self.appMap['CCCSERVER'][0].sendTransition("STOP", m))
        time.sleep(1.);
        json.loads(self.appMap['CCCSERVER'][0].sendCommand("CCCRESET", {}))
        json.loads(self.appMap['CCCSERVER'][0].sendCommand("DIFRESET", {}))
        r["CCCSERVER"] = s

        for x in self.appMap["BUILDER"]:
            s = json.loads(x.sendTransition("CONFIGURE", m))
            r["BUILDER_%d" % x.appInstance] = s

        for x in self.appMap["DIFMANAGER"]:
            s = json.loads(x.sendTransition("SCAN", m))
            r["DIFMANAGER_SCAN_%d" % x.appInstance] = s

        for x in self.appMap["DIFMANAGER"]:
            s = json.loads(x.sendTransition("INITIALISE", m))
            r["DIFMANAGER_INIT_%d" % x.appInstance] = s

        return json.dumps(r)

    def daq_configure(self):
        m = {}
        r = {}
        json.loads(self.appMap['CCCSERVER'][0].sendCommand("CCCRESET", {}))
        s=json.loads(self.appMap['CCCSERVER'][0].sendCommand("DIFRESET", {}))
        r["CCCSERVER"] = s

        for x in self.appMap["DIFMANAGER"]:
            s = json.loads(x.sendTransition("CONFIGURE", m))
            r["DIFMANAGER_%d" % x.appInstance] = s
        return json.dumps(r)

    def daq_stop(self):
        m = {}
        r = {}
        s=json.loads(self.appMap['CCCSERVER'][0].sendTransition("STOP", m))
        r["CCCSERVER"] = s

        for x in self.appMap["DIFMANAGER"]:
            s = json.loads(x.sendTransition("STOP", m))
            r["DIFMANAGER_%d" % x.appInstance] = s

        for x in self.appMap["BUILDER"]:
            s = json.loads(x.sendTransition("STOP", m))
            r["BUILDER_%d" % x.appInstance] = s

        return json.dumps(r)

    def daq_destroy(self):
        m = {}
        r = {}
        for x in self.appMap["DIFMANAGER"]:
            s = json.loads(x.sendTransition("DESTROY", m))
            r["DIFMANAGER_%d" % x.appInstance] = s

        return json.dumps(r)

    def daq_start(self, run, location="UNKNOWN", comment="Not set"):

        if (location == "UNKNOWN"):
            location = os.getenv("DAQSETUP", "UNKNOWN")
        nrun = run
        if (run == 0):
            smg = mg.instance()
            jnrun = smg.getRun(location, comment)
        r = {}
        m = {}
        # print "EVENT BUILDER",jnrun['run']
        m['run'] = jnrun['run']
        for x in self.appMap["BUILDER"]:
            print("Sending Start to vent builder")
            s = json.loads(x.sendTransition("START", m))
            r["BUILDER_%d" % x.appInstance] = s

        m = {}
        for x in self.appMap["DIFMANAGER"]:
            s = json.loads(x.sendTransition("START", m))
            r["DIFMANAGER_%d" % x.appInstance] = s

        s=json.loads(self.appMap['CCCSERVER'][0].sendTransition("START", m))
        r["CCCSERVER"] = s
        return json.dumps(r)

    def SourceStatus(self, verbose=False):
        rep = {}
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

                    #rep["%s_%d" % (s.host, s.infos['instance'])] = r
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

    # DIF Specific
    def setControlRegister(self,ctrlreg):
        param = {}
        param["value"] = int(ctrlreg, 16)
        return self.processCommand("CTRLREG", "DIFMANAGER", param)

